#include "X86_64Assembly.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "../TheGenerator.hpp"
#include "Assembly/LinearAnalysis.hpp"
#include <fstream>

namespace x86_64 {
    void AddGlobalVariables(std::ofstream& out) {
        for (auto& n : globalVariables.map) {
            out << "\nglob." << n.first << ":\n";
            if (n.second.type.subtype == ParserVariable::Subtype::globalValue) {
                switch (n.second.type.size) {
                    case 1:
                        PrintWithSpaces(".byte", out);
                        break;
                    case 2:
                        PrintWithSpaces(".value", out);
                        break;
                    case 4:
                        PrintWithSpaces(".long", out);
                        break;
                    case 8:
                        PrintWithSpaces(".quad", out);
                        break;
                    default:
                        CodeGeneratorError("Bug: Invalid size in global variables out!");
                }
            }
            else {
                switch (Options::addressSize) {
                    case 1:
                        PrintWithSpaces(".byte", out);
                        break;
                    case 2:
                        PrintWithSpaces(".value", out);
                        break;
                    case 4:
                        PrintWithSpaces(".long", out);
                        break;
                    case 8:
                        PrintWithSpaces(".quad", out);
                        break;
                    default:
                        CodeGeneratorError("Bug: Invalid size in global variables out!");
                }
            }
            
            if (n.second.expression.nodeType != ParserValue::Node::constant) {
                CodeGeneratorError("Global variable initial values must be constant values!");
            }
            out << *n.second.expression.value << "\n";
        }
    }
    
    uint64_t GetJumpType(uint32_t code, uint32_t comparison) {
        if (code == Bytecode::jumpConditionalTrue) {
            switch (comparison) {
                case ParserCondition::greater:
                    return x86_64::jg;
                case ParserCondition::greaterEqual:
                    return x86_64::jge;
                case ParserCondition::lesser:
                    return x86_64::jb;
                case ParserCondition::lesserEqual:
                    return x86_64::jbe;
                case ParserCondition::equals:
                    return x86_64::je;
                case ParserCondition::notEquals:
                    return x86_64::jne;
                default:
                    CodeGeneratorError("Bug: Invalid jump label request!");
            }
        }
        else {
            switch (comparison) {
                case ParserCondition::greater:
                    return x86_64::jbe;
                case ParserCondition::greaterEqual:
                    return x86_64::jb;
                case ParserCondition::lesser:
                    return x86_64::jge;
                case ParserCondition::lesserEqual:
                    return x86_64::jb;
                case ParserCondition::equals:
                    return x86_64::jne;
                case ParserCondition::notEquals:
                    return x86_64::je;
                default:
                    CodeGeneratorError("Bug: Invalid jump label request!");
            }
        }
        CodeGeneratorError("Bug: Invalid jump label request!");
        return 0;
    }

    std::vector <std::string> argumentNames;

    void CallX86_64Function(ParserFunction* function, const uint64_t& index, std::string& result) {
        if (argumentNames.size() != function->arguments.size()) {
            CodeGeneratorError("Bug: Argument amount mismatch in generator!");
        }
        for (uint64_t n = 0; n < argumentNames.size(); n++) {
            // move every argument into place
            // but if the variable still exists after and the target is the same as source copy it elsewhere
            if (not argumentNames[n].starts_with("$") and variableLifetimes[argumentNames[n]].lastUse > index and
            generatorMemory.findThing(argumentNames[n]) == DataLocation(function->arguments[n].locationType, int64_t(function->arguments[n].locationValue))) {
                // great, now it needs to be moved, let's do it, I really need to make dedicated functions for those
                auto location = generatorMemory.findThing(argumentNames[n]);
                if (location.type == Operand::reg) {
                    bool found = false;
                    for (uint64_t k = generatorMemory.registers.size() - 1; k > 0; k--) {
                        if (generatorMemory.registers[k].content.value == "!" or generatorMemory.registers[k].content.value.starts_with("$")) {
                            // found one!
                            MoveValue(argumentNames[n], "%" + std::to_string(n), argumentNames[n], argumentNames[n][1] - '0', index);
                            found = true;
                            break;
                        }
                    }
                    if (not found) {
                        // move it to stack
                        MoveValue("%0", "@" + std::to_string(AddStackVariable(argumentNames[n])->offset), argumentNames[n], argumentNames[n][1] - '0', index);
                    }
                }
                else {
                    CodeGeneratorError("Unimplemented: stack argument pass copying");
                }
            }
            MoveValue(argumentNames[n], DataLocation(function->arguments[n].locationType,
                int64_t(function->arguments[n].locationValue)).forMove(), argumentNames[n],
                parserTypes[function->arguments[n].typeName].size, index);
        }
        // if function has a return type ensure register a is not used
        if (not function->returnType.empty() and generatorMemory.registers[0].content.value != "!") {
            // this will be annoying, I need to do a get out of here function
            auto& reg = generatorMemory.registers[0];
            if (not reg.content.value.starts_with("$")) {
                // we really have a variable here, let's just move it to a different free register or to stack
                bool found = false;
                std::string content = reg.content.value;
                reg.content.value = "!";
                if (generatorMemory.findThing(content).type == Operand::none) {
                    for (uint64_t n = generatorMemory.registers.size() - 1; n > 0; n--) {
                        if (generatorMemory.registers[n].content.value == "!" or generatorMemory.registers[n].content.value.starts_with("$")) {
                            // found one!
                            MoveValue("%0", "%" + std::to_string(n), content, content[1] - '0', index);
                            found = true;
                            break;
                        }
                    }
                    if (not found) {
                        // move it to stack
                        MoveValue("%0", "@" + std::to_string(AddStackVariable(content)->offset), content, content[1] - '0', index);
                    }
                }
                reg.content.value = result;
            }
        }
        Instruction ins;
        ins.type = x86_64::call;
        ins.op1 = DataLocation(Operand::fun, function);
        finalInstructions.push_back(ins);
        if (not function->returnType.empty()) {
            SetContent({Operand::reg, uint64_t(0)}, result);
        }

    }

    void ConvertBytecode(Bytecode& bytecode, uint64_t index) {
        switch (bytecode.code) {
            case Bytecode::add:
                if (Optimizations::swapExpressionOperands) {
                    if (bytecode.target.starts_with("$")) {
                        std::swap(bytecode.source, bytecode.target);
                    }
                }
                GenerateInstruction({
                    x86_64::add, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                }, index);
                break;
            case Bytecode::subtract:
                GenerateInstruction({
                    x86_64::sub, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                }, index);
                break;
            case Bytecode::multiply:
                if (Optimizations::swapExpressionOperands) {
                    if (bytecode.target.starts_with("$")) {
                        std::swap(bytecode.source, bytecode.target);
                    }
                }
                if (bytecode.type.type == ParserType::unsignedInteger) {
                    GenerateInstruction({
                    x86_64::mul, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }})}
                    }, index);
                }
                else if (bytecode.type.type == ParserType::signedInteger) {
                    // works much better if swapping in enabled
                    if (bytecode.type.size <= 4 and bytecode.source.starts_with("$")) {
                        // three operand form
                        GenerateInstruction({
                    x86_64::imul, bytecode.type.size, bytecode.target, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                    }, index);
                    }
                    else {
                        // 2 operand form
                        GenerateInstruction({
                    x86_64::imul, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                    }, index);
                    }
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid type for multiplication!");
                }
                break;
            case Bytecode::divide:
                if (bytecode.type.size == 1) {
                    // TODO: add something that does a movzbw on al to ax to set zeroes to ah
                    std::cout << "INFO L1: Due to the nature of 8 bit integer division result value may be invalid!\n";
                }
                if (bytecode.type.type == ParserType::unsignedInteger) {
                    GenerateInstruction({
                    x86_64::div, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }})}
                    }, index);
                }
                else if (bytecode.type.type == ParserType::signedInteger) {
                    GenerateInstruction({
                    x86_64::idiv, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.GetPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }})}
                    }, index);
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid type for division!");
                }
                break;
            case Bytecode::callFunction:
                CallX86_64Function(&parserFunctions[bytecode.source], index, bytecode.target);
                argumentNames.clear();
                break;
            case Bytecode::moveArgument:
                argumentNames.push_back(bytecode.source);
                break;
            case Bytecode::returnValue:
                // move returned value into register a
                MoveValue(bytecode.source, "%0", bytecode.source, bytecode.type.size, index);
                NewMoveValue(VariableInfo(bytecode.source), VariableInfo("%0"), bytecode.source);
                // insert a return statement
                GenerateInstruction({x86_64::ret}, index);
                break;
            case Bytecode::pushLevel:
                FillDesignatedPlaces(index);
                generatorMemory.pushLevel();
                break;
            case Bytecode::popLevel:// rax = k + fun(k - 1) == k + t
                FillDesignatedPlaces(index);
                generatorMemory.popLevel();
                break;
            case Bytecode::jumpConditionalFalse:
            case Bytecode::jumpConditionalTrue:
                FillDesignatedPlaces(index);
                {
                    Instruction ins;
                    ins.type = GetJumpType(bytecode.code, bytecode.number);
                    ins.op1.type = Operand::jla;
                    ins.op1.number = std::stoull(bytecode.source.substr(Options::jumpLabelPrefix.length()));
                    finalInstructions.push_back(ins);
                }
                break;

            case Bytecode::jump:
                FillDesignatedPlaces(index);
                {
                    Instruction ins;
                    ins.type = x86_64::jmp;
                    ins.op1.type = Operand::jla;
                    ins.op1.number = std::stoull(bytecode.source.substr(Options::jumpLabelPrefix.length()));
                    finalInstructions.push_back(ins);
                }
                break;
            case Bytecode::compare:
                if (Optimizations::swapExpressionOperands) {
                    if (bytecode.target.starts_with("$")) {
                        std::swap(bytecode.source, bytecode.target);
                    }
                    // TODO: add a condition switch function
                    switch (bytecode.number) {
                        case ParserCondition::lesser:
                            bytecode.number = ParserCondition::greaterEqual;
                            break;
                        case ParserCondition::greater:
                            bytecode.number = ParserCondition::lesserEqual;
                            break;
                        case ParserCondition::lesserEqual:
                            bytecode.number = ParserCondition::greater;
                            break;
                        case ParserCondition::greaterEqual:
                            bytecode.number = ParserCondition::lesser;
                            break;
                    }
                }
                // compare here
                FillDesignatedPlaces(index);
                GenerateInstruction({
                    x86_64::cmp, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {})}
                }, index);
                break;
            case Bytecode::declare:
            {
                auto& var = variableLifetimes[bytecode.target];
                if (Optimizations::skipUnusedVariables and var.usageAmount == 0) {
                        return;
                }

                MoveValue(bytecode.source, (var.assignStatus == Operand::reg ?
                                            "%" + std::to_string(var.regNumber) :
                                            "@" + std::to_string(AddStackVariable(bytecode.target)->offset)), bytecode.target, bytecode.type.size, index);
                break;
            }
            case Bytecode::assign:
                AssignExpressionToVariable(bytecode.source, bytecode.target);
                break;
            case Bytecode::addLabel:
                FillDesignatedPlaces(index);
                if (bytecode.source.starts_with(Options::jumpLabelPrefix)) {
                    Instruction ins;
                    ins.type = x86_64::jumpLabel;
                    ins.op1.type = Operand::jla;
                    ins.op1.number = std::stoull(bytecode.source.substr(Options::jumpLabelPrefix.length()));
                    finalInstructions.push_back(ins);
                }
                else {
                    CodeGeneratorError("Unsupported: non jump x86_64 label!");
                }
                break;
            case Bytecode::moveValue:
            {
                auto target = generatorMemory.findThing(bytecode.target);
                if (target.type == Operand::reg) {
                    std::string temp = generatorMemory.registers[target.number].content.value;
                    SetContent(target, "!");
                    MoveValue(bytecode.source, "%" + std::to_string(target.number),
                              temp, bytecode.type.size, index);
                }
                else if (target.type == Operand::sta) {
                    auto* sta = FindStackVariableByOffset(target.offset);
                    std::string temp = sta->content.value;
                    SetContent(target, "!");
                    MoveValue(bytecode.source, "@" + std::to_string(target.offset),
                              temp, bytecode.type.size, index);
                }
                else if (target.type == Operand::none) {
                    SetContent(generatorMemory.findThing(bytecode.source), bytecode.target);
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid operand target for move!");
                }
            }
                break;
            case Bytecode::addFromArgument:
                if (Optimizations::skipUnusedVariables and variableLifetimes[bytecode.target].usageAmount == 0) {
                    break;
                }
                if (bytecode.source.front() == '%') {
                    generatorMemory.registers[std::stoull(bytecode.source.substr(1))].content.value = bytecode.target;
                }
                else {
                    CodeGeneratorError("Unimplemented: Stack argument at start of function!");
                }
                break;
            default:
                CodeGeneratorError("Invalid bytecode code in converter!");
                break;
        }
    }
}