#include "X86_64Assembly.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "../TheGenerator.hpp"
#include "Assembly/LinearAnalysis.hpp"
#include <fstream>

namespace x86_64 {
    void AddGlobalVariables(std::ofstream& out) {
        for (auto& n : globalVariables.map) {
            out << "\n" << n.first << ":\n";
            if (n.second.type.subtype == ParserVariable::Subtype::value) {
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

    void CallX86_64Syscall(uint64_t callNumber, bool doesReturn, std::string result = "!") {
        std::array <uint16_t, 7> safeRegisters = {x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15};
        std::array <uint16_t, 6> registersToUse = {x86_64::rdi, x86_64::rsi, x86_64::rdx, x86_64::rcx, x86_64::r8, x86_64::r9};
        for (uint64_t n = 0; n < argumentNames.size(); n++) {
            if (n < registersToUse.size()) {
                uint64_t size = 0;
                if (argumentNames[n].starts_with("$") or argumentNames[n].starts_with("\"")) {
                    size = Options::addressSize;
                }
                else {
                    size = GetVariableType(argumentNames[n]).size;
                }
                MoveValue(VariableInfo(argumentNames[n]), VariableInfo::FromLocation({Operand::reg, uint64_t(registersToUse[n])}), argumentNames[n], size);
            }
            else {
                CodeGeneratorError("Unimplemented: passing of arguments in stack!");
            }
        }
        for (uint64_t n = 0; n < argumentNames.size(); n++) {
            if (not argumentNames[n].starts_with("$") and not argumentNames[n].starts_with("\"")) {
                auto& life = variableLifetimes[argumentNames[n]];
                if (life.lastUse > currentBytecodeIndex) {
                    CopyVariableElsewhereNoReference(VariableInfo(argumentNames[n]));
                }
            }
            generatorMemory.registers[n].content.value = "!";
        }
        // now values are moved, let's back up other values to stack
        for (uint64_t n = 1; n < generatorMemory.registers.size(); n++) {
            auto& reg = generatorMemory.registers[n];
            if (reg.content.value != "!" and not reg.content.value.starts_with("$") and not reg.content.value.starts_with("\"") and variableLifetimes[reg.content.value].lastUse > currentBytecodeIndex) {
                // value needs to be moved away to stack
                bool safe = false;
                for (auto m : safeRegisters) {
                    if (m == n) {
                        safe = true;
                        break;
                    }
                }
                if (safe) {
                    continue;
                }
                for (uint64_t m = 0; m < argumentNames.size() and m < 6; m++) {
                    if (registersToUse[m] == n) {
                        safe = true;
                        break;
                    }
                }
                if (not safe) {
                    MoveValue(VariableInfo::FromLocation({Operand::reg, n}), VariableInfo::FromLocation({Operand::sta, AddStackVariable(reg.content.value)->offset}), "!", GetVariableType(reg.content.value).size);
                }
            }
        }
        if (generatorMemory.registers.front().content.value != "!" and not generatorMemory.registers.front().content.value.starts_with("$")) {
            MoveValue(VariableInfo::FromLocation({Operand::reg, uint64_t(0)}), VariableInfo::FromLocation({Operand::sta, AddStackVariable(generatorMemory.registers.front().content.value)->offset}), "!", GetVariableType(generatorMemory.registers.front().content.value).size);
            generatorMemory.registers.front().content.value = "!";
        }
        MoveValue(VariableInfo("$" + std::to_string(callNumber)), VariableInfo("%0"), "$0", Options::addressSize);
        if (doesReturn) {
            generatorMemory.registers.front().content.value = result;
        }
        else {
            generatorMemory.registers.front().content.value = "!";
        }
        Instruction ins;
        ins.type = x86_64::syscall;
        finalInstructions.push_back(ins);
    }

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
                MoveVariableElsewhereNoReference(VariableInfo(argumentNames[n]));
            }
            MoveValue(VariableInfo(argumentNames[n]), VariableInfo::FromLocation(DataLocation(function->arguments[n].locationType,
                int64_t(function->arguments[n].locationValue))), argumentNames[n],
                parserTypes[function->arguments[n].typeName].size);
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
                    MoveVariableElsewhereNoReference(VariableInfo("%0"));
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
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                }, index);
                break;
            case Bytecode::subtract:
                GenerateInstruction({
                    x86_64::sub, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::reg, {0, 1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
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
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
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
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   Operand::imm, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
                    }, index);
                    }
                    else {
                        // 2 operand form
                        GenerateInstruction({
                    x86_64::imul, bytecode.type.size, bytecode.target, bytecode.source,
                    {
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}}),
                     OpCombination(Operand::reg, {1, 2, 3, 7 ,8, 9, 10, 11, 12, 13, 14, 15},
                                   Operand::sta, {},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"}})}
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
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
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
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }}),
                     OpCombination(Operand::reg, {0},
                                   Operand::sta, {},
                                   {{x86_64::rdx, 0}},
                                   {{DataLocation(Operand::replace, uint64_t(0)), bytecode.type.getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"},
                                           {DataLocation(Operand::reg, uint64_t(x86_64::rdx)), "!" }})}
                    }, index);
                }
                else {
                    CodeGeneratorError("Unimplemented: Invalid type for division!");
                }
                break;
            case Bytecode::getAddress:
                {
                    auto& life = variableLifetimes[VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype + 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"];
                    if (life.assignStatus == Operand::reg) {
                        X86_64GetVariableAddress(VariableInfo(bytecode.source), VariableInfo::FromLocation(DataLocation(Operand::reg, life.regNumber)), VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype + 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0");
                    }
                    else {
                        auto where = FindViableRegister();
                        X86_64GetVariableAddress(VariableInfo(bytecode.source), VariableInfo::FromLocation(where), VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype + 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0");
                    }
                }
                break;
            case Bytecode::getValue:
            {
                auto& life = variableLifetimes[VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype - 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0"];
                if (life.assignStatus == Operand::reg) {
                    X86_64DereferencePointer(VariableInfo(bytecode.source), VariableInfo::FromLocation(DataLocation(Operand::reg, life.regNumber)));
                    SetContent(DataLocation(Operand::reg, life.regNumber),
                               VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype - 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0");
                }
                else {
                    auto where = FindViableRegister();
                    X86_64DereferencePointer(VariableInfo(bytecode.source), VariableInfo::FromLocation(where));
                    SetContent(where,
                               VariableType(bytecode.type.size, bytecode.type.type, bytecode.type.subtype - 1).getPrefix() + "=#" + std::to_string(bytecode.number) + "-0");
                }
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
                // MoveValue(bytecode.source, "%0", bytecode.source, bytecode.type.size, index);
                MoveValue(VariableInfo(bytecode.source), VariableInfo("%0"), bytecode.source, bytecode.type.size);
                // insert a return statement
                GenerateInstruction({x86_64::ret}, index);
                break;
            case Bytecode::pushLevel:
                FillDesignatedPlaces(index);
                generatorMemory.pushLevel();
                break;
            case Bytecode::popLevel:
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

                MoveValue(VariableInfo(bytecode.source), VariableInfo((var.assignStatus == Operand::reg ?
                                            "%" + std::to_string(var.regNumber) :
                                            "@" + std::to_string(AddStackVariable(bytecode.target)->offset))), bytecode.target, bytecode.type.size);
                break;
            }
            case Bytecode::assign:
                if (bytecode.number == 1) {
                    // in that case we need to set the variable at it's address
                    DataLocation where;
                    if (generatorMemory.findThing(GetPreviousVariableAssignment(bytecode.target)).type != Operand::reg) {
                        where = FindViableRegister();
                        MoveValue(VariableInfo(GetPreviousVariableAssignment(bytecode.target)), VariableInfo::FromLocation(where), bytecode.target, Options::addressSize);
                    }
                    else {
                        where = generatorMemory.findThing(GetPreviousVariableAssignment(bytecode.target));
                    }
                    // move the value into the location
                    SetValueAtAddress(VariableInfo(bytecode.source), where.number, bytecode.type.size);
                    AssignExpressionToVariable(bytecode.target, GetPreviousVariableAssignment(bytecode.target));
                }
                else {
                    if (bytecode.source.starts_with("$")) {
                        MoveValue(VariableInfo(bytecode.source), VariableInfo(GetPreviousVariableAssignment(bytecode.target)), bytecode.target, bytecode.type.size);
                    }
                    else {
                        AssignExpressionToVariable(bytecode.source, bytecode.target);
                    }
                }
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
                    MoveValue(VariableInfo(bytecode.source), VariableInfo("%" + std::to_string(target.number)),
                              temp, bytecode.type.size);
                }
                else if (target.type == Operand::sta) {
                    auto* sta = FindStackVariableByOffset(target.offset);
                    std::string temp = sta->content.value;
                    SetContent(target, "!");
                    MoveValue(VariableInfo(bytecode.source), VariableInfo("@" + std::to_string(target.offset)),
                              temp, bytecode.type.size);
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
            case Bytecode::syscall:
                CallX86_64Syscall(bytecode.number, false);
                break;
            default:
                CodeGeneratorError("Invalid bytecode code in converter!");
                break;
        }
    }
}