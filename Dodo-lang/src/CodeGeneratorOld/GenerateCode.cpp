#include "GenerateCode.hpp"
#include "ParserVariables.hpp"
#include "StackVector.hpp"
#include "GeneratorSettings.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <utility>
#include <stack>

namespace fs = std::filesystem;

uint16_t GetValueByteSize(const ParserValue& value, StackVector& variables) {
    if (value.nodeType == ParserValue::Node::constant) {
        // simple, just check how large the number is, yeah no support for non numeric here
        uint64_t val;
        if (value.isNegative) {
            val = -std::stoll(*value.value) * 2ll;
        }
        else {
            val = std::stoll(*value.value);
        }
        if (val < 256) {
            return 1;
        }
        if (val < 65536) {
            return 2;
        }
        if (val < 4294967296) {
            return 4;
        }
        return 8;
    }
    else if (value.nodeType == ParserValue::Node::variable) {
        return parserTypes[variables.find(*value.value).typeName].size;
    }
    else if (value.nodeType == ParserValue::Node::operation) {
        if (value.operationType == ParserValue::Operation::functionCall) {
            return parserTypes[parserFunctions[*value.value].returnType].size;
        }

        uint16_t left = GetValueByteSize(*value.left, variables);
        uint16_t right = GetValueByteSize(*value.right, variables);
        if (left > right) {
            return left;
        }
        return right;
    }
    CodeError("Unexpected value in size calculation!");
    return 1;
}

// takes a mathematical expression and returns the value to put wherever the user wants
// output type refers to the expected type of value to come out, namely a signed, unsigned or float
std::string CalculateExpression(StackVector& variables, std::ofstream& out, uint16_t outputSize,
                                const ParserValue& expression, uint8_t outputType,
                                ValueType returnValueLocations, RegisterNames registers) {

    if (expression.nodeType == ParserValue::Node::empty) {
        // shouldn't happen afaik
        CodeError("Empty node in wrong place!");
    }

    // if given node is a constant just return it's value
    if (expression.nodeType == ParserValue::constant) {
        uint64_t number = ConvertValue(*expression.value, 64);
        if (returnValueLocations.val) {
            return "$" + std::to_string(number);
        }
        else if (returnValueLocations.reg) {
            out << "mov" << AddInstructionPostfix(outputSize) << "    $" << std::to_string(number) << ", "
                << registers.registerBySize(outputSize) << "\n";
            return registers.registerBySize(outputSize);
        }
        else {
            CodeError("Unsupported value return type for node type!");
            return "";
        }
    }

    // if given node is a variable
    if (expression.nodeType == ParserValue::Node::variable) {
        auto& var = variables.find(*expression.value);
        if (not returnValueLocations.reg) {
            // strictly stack
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType, false,
                                        &variables);
        }
        if (not returnValueLocations.sta) {
            // strictly register
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType, true,
                                        nullptr, true, registers);
        }
        else {
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, var.type, outputType);
        }
    }

    // if given node is an actual multi thingy expression
    if (expression.nodeType == ParserValue::Node::operation) {
        // for now functions are going to be separate
        if (expression.operationType == ParserValue::Operation::functionCall) {
            if (returnValueLocations.reg) {
                return GenerateFunctionCall(out, variables, *expression.value,
                                            outputSize, outputType, &expression, registers);
            }
            else if (returnValueLocations.sta) {
                StackVariable var;
                var.singleSize = outputSize;
                var.amount = 1;
                const StackVariable& pushed = variables.push(var);
                return GenerateFunctionCall(out, variables, *expression.value,
                                            outputSize, outputType, &expression, pushed.getAddressAsRegisterNames());
            }
            else {
                CodeError("Unsupported function return type!");
            }
        }


        uint8_t largestSize = outputSize;
        bool leftLarger = false;
        bool rightLarger = false;
        if (expression.left->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.left->value);
            if (var.singleSize > largestSize) {
                largestSize = var.singleSize;
                leftLarger = true;
            }
        }
        if (expression.right->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.right->value);
            if (var.singleSize > largestSize) {
                largestSize = var.singleSize;
                rightLarger = true;
            }
        }

        std::string left = CalculateExpression(variables, out, largestSize, *expression.left, outputType, {0, 1, 1, 1});
        std::string right;
        if (expression.operationType == ParserValue::Operation::division) {
            right = CalculateExpression(variables, out, largestSize, *expression.right, outputType, {1, 0, 0, 0},
                                        {"%bl", "%bx", "%ebx", "%rbx"});
        }
        else {
            right = CalculateExpression(variables, out, largestSize, *expression.right, outputType, {0, 1, 1, 1});
        }


        out << "mov" << AddInstructionPostfix(largestSize) << "    " << left << ", " << AddRegisterA(largestSize)
            << "\n";
        // freeing temp variables
        if (left[0] != '%' and left[0] != '$' and variables.findByOffset(left).name.empty()) {
            variables.free(left);
        }

        // operations really only change this part
        switch (expression.operationType) {
            case ParserValue::Operation::addition:
                out << "add" << AddInstructionPostfix(largestSize) << "    " << right << ", "
                    << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::subtraction:
                out << "sub" << AddInstructionPostfix(largestSize) << "    " << right << ", "
                    << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::multiplication:
                if (outputType == ParserValue::Value::unsignedInteger) {
                    out << "mul" << AddInstructionPostfix(largestSize) << "    " << right << "\n";
                }
                else if (outputType == ParserValue::Value::signedInteger) {
                    out << "imul" << AddInstructionPostfix(largestSize) << "   " << right << ", "
                        << registers.registerBySize(largestSize) << "\n";
                }

                break;
            case ParserValue::Operation::division:
                if (outputType == ParserValue::Value::unsignedInteger) {
                    out << "movq    $0, %rdx\n";
                    out << "div" << AddInstructionPostfix(largestSize) << "    " << right << ", "
                        << registers.registerBySize(largestSize) << "\n";
                }
                else if (outputType == ParserValue::Value::signedInteger) {
                    out << "movq    $0, %rdx\n";
                    out << "idiv" << AddInstructionPostfix(largestSize) << "   " << right << ", "
                        << registers.registerBySize(largestSize) << "\n";
                }
                break;
            default:
                CodeError("Unsupported operation type!");
        }


        // freeing temp variables
        if (right[0] != '%' and right[0] != '$' and variables.findByOffset(right).name.empty()) {
            variables.free(right);
        }

        std::string reg = ConvertValueInRegister(out, largestSize, outputSize, outputType, outputType, registers);

        if (returnValueLocations.reg) {
            return reg;
        }
        else if (returnValueLocations.sta) {
            StackVariable var;
            var.singleSize = largestSize;
            var.amount = 1;

            std::string varAddress = variables.pushAndStr(var);
            out << "mov" << AddInstructionPostfix(outputSize) << "    " << reg << ", " << varAddress << "\n";
            return varAddress;
        }
        else {
            CodeError("Unsupported value return type for node type!");
        }
    }

    CodeError("Unhandled expression path!");
    return "$0";
}

void GenerateFunction(const std::string& identifier, std::ofstream& out) {
    StackVector variables;
    const ParserFunction* function = &parserFunctions[identifier];
    variables.addArguments(*function);

    // for internal leveling
    uint64_t returnLabelCounter = 0;
    std::stack<uint64_t> returnLabels;
    std::stack<uint64_t> whileLevels;
    std::stack<uint64_t> forLevels;
    std::stack<const ForInstruction*> forPointers;

    if (setting::OutputDodoInstructions) {
        out << "\n" << setting::CommentCharacter << " Declaration of function: " << function->returnType << " "
            << identifier << "(...)\n";
    }

    out << identifier << ":\n";

    if (TargetArchitecture == "X86-64") {
        out << "pushq   %rbp\n";
        out << "movq    %rsp, %rbp\n";
    }

    bool didReturn = false;
    for (uint64_t current = 0; current < function->instructions.size(); current++) {
        switch (function->instructions[current].type) {
            case FunctionInstruction::Type::beginScope:
                returnLabels.push(returnLabelCounter++);
                variables.addLevel();
                break;

            case FunctionInstruction::Type::endScope:
                if (current == function->instructions.size() - 1 or
                    function->instructions[current + 1].type != FunctionInstruction::Type::elseStatement) {
                    if (not whileLevels.empty() and whileLevels.top() == returnLabels.size()) {
                        out << "jmp     .L" << returnLabels.top() - 1 << "." << function->name << "\n";
                        whileLevels.pop();
                    }
                    if (not forLevels.empty() and forLevels.top() == returnLabels.size()) {
                        // doing the value changes after loop run
                        if (setting::OutputDodoInstructions) {
                            out << setting::CommentCharacter << " Post for loop instructions\n";
                        }
                        for (auto& change: forPointers.top()->instructions) {
                            if (change.type != FunctionInstruction::Type::valueChange) {
                                CodeError("Instructions other than value change not yet added to for loop!");
                            }
                            auto& ins = *change.variant.valueChangeInstruction;
                            if (ins.expression.nodeType == ParserValue::Node::operation) {
                                out << setting::CommentCharacter << " Value change of variable: " << ins.name
                                    << " = expression\n";
                            }
                            else if (ins.expression.nodeType == ParserValue::Node::constant) {
                                out << setting::CommentCharacter << " Value change of variable: " << ins.name
                                    << " = constant\n";
                            }
                            else if (ins.expression.nodeType == ParserValue::Node::variable) {
                                out << setting::CommentCharacter << " Value change of variable: " << " " << ins.name
                                    << " = variable\n";
                            }
                            else {
                                out << setting::CommentCharacter << " Value change of variable: " << " " << ins.name
                                    << "\n";
                            }
                            StackVariable& var = variables.find(ins.name);
                            const ParserType& type = parserTypes[var.typeName];
                            if (not var.isMutable) {
                                CodeError("Variable: " + var.name + " is immutable!");
                            }

                            if (ins.expression.nodeType != ParserValue::Node::empty) {
                                std::string source = CalculateExpression(variables, out, var.singleSize, ins.expression,
                                                                         type.type,
                                                                         {1, 1, 0, 1});
                                out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                                    << var.getAddress() << "\n";
                            }
                            else {
                                CodeError("No expression in value assignment!");
                            }
                        }

                        forPointers.pop();
                        variables.popLevel();
                    }
                    out << "jmp     .L" << returnLabels.top() - 1 << "." << function->name << "\n";
                    out << ".L" << returnLabels.top() << "." << function->name << ":\n";
                    forLevels.pop();
                    returnLabels.pop();
                    variables.popLevel();
                }
                break;

            case FunctionInstruction::Type::elseStatement:
                out << "jmp     .L" << returnLabelCounter << "." << function->name << "\n";
                out << ".L" << returnLabels.top() << "." << function->name << ":\n";
                returnLabels.pop();
                break;

            case FunctionInstruction::Type::ifStatement: {
                const IfInstruction& ifi = *function->instructions[current].variant.ifInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " If statement\n";
                }

                // take both types and find the larger one
                uint16_t larger;
                {
                    auto left = GetValueByteSize(ifi.condition.left, variables);
                    auto right = GetValueByteSize(ifi.condition.right, variables);
                    larger = (left > right ? left : right);
                }

                // get the registers ready
                // TODO: add type check here
                std::string right = CalculateExpression(variables, out, larger, ifi.condition.left, 0, {0, 0, 1, 0});
                std::string left = CalculateExpression(variables, out, larger, ifi.condition.right, 0, {1, 1, 0, 0});
                out << "cmp" << AddInstructionPostfix(larger) << "    " << left << ", " << right << "\n";
                if (ifi.condition.type == ParserCondition::Type::greater) {
                    out << "jbe     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (ifi.condition.type == ParserCondition::Type::greaterEqual) {
                    out << "jb      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (ifi.condition.type == ParserCondition::Type::lesser) {
                    out << "jge     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (ifi.condition.type == ParserCondition::Type::lesserEqual) {
                    out << "jg      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (ifi.condition.type == ParserCondition::Type::equals) {
                    out << "jne     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (ifi.condition.type == ParserCondition::Type::notEquals) {
                    out << "je      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }

                // freeing temp variables
                if (right[0] != '%' and right[0] != '$' and variables.findByOffset(right).name.empty()) {
                    variables.free(right);
                }
                break;
            }

            case FunctionInstruction::Type::whileStatement: {
                const WhileInstruction& whi = *function->instructions[current].variant.whileInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " While statement\n";
                }

                // take both types and find the larger one
                uint16_t larger;
                {
                    auto left = GetValueByteSize(whi.condition.left, variables);
                    auto right = GetValueByteSize(whi.condition.right, variables);
                    larger = (left > right ? left : right);
                }

                // get the registers ready
                // TODO: add type check here
                out << ".L" << returnLabelCounter++ << "." << function->name << ":\n";
                std::string right = CalculateExpression(variables, out, larger, whi.condition.left, 0, {0, 0, 1, 0});
                std::string left = CalculateExpression(variables, out, larger, whi.condition.right, 0, {1, 1, 0, 0});
                whileLevels.push(returnLabels.size() + 1);
                out << "cmp" << AddInstructionPostfix(larger) << "    " << left << ", " << right << "\n";
                if (whi.condition.type == ParserCondition::Type::greater) {
                    out << "jbe     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (whi.condition.type == ParserCondition::Type::greaterEqual) {
                    out << "jb      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (whi.condition.type == ParserCondition::Type::lesser) {
                    out << "jge     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (whi.condition.type == ParserCondition::Type::lesserEqual) {
                    out << "jg      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (whi.condition.type == ParserCondition::Type::equals) {
                    out << "jne     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (whi.condition.type == ParserCondition::Type::notEquals) {
                    out << "je      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                // freeing temp variables
                if (right[0] != '%' and right[0] != '$' and variables.findByOffset(right).name.empty()) {
                    variables.free(right);
                }
                break;
            }

            case FunctionInstruction::Type::forStatement: {
                const ForInstruction& loop = *function->instructions[current].variant.forInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " For statement\n";
                }

                // take both types and find the larger one

                variables.addLevel();
                // declaring the variables first
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " Declaration of for loop variables\n";
                }
                for (auto& loopVar: loop.variables) {
                    StackVariable var;
                    const ParserType& type = parserTypes[loopVar.typeName];
                    var.singleSize = type.size;
                    var.amount = 1;
                    var.name = loopVar.identifier;
                    var.type = type.type;
                    var.typeName = loopVar.typeName;
                    var.isMutable = true;

                    if (setting::OutputDodoInstructions) {
                        if (loopVar.value.nodeType == ParserValue::Node::operation) {
                            out << setting::CommentCharacter << " Declaration of: mut " << loopVar.typeName << " "
                                << loopVar.identifier
                                << " = expression\n";
                        }
                        else if (loopVar.value.nodeType == ParserValue::Node::constant) {
                            out << setting::CommentCharacter << " Declaration of: mut " << loopVar.typeName << " "
                                << loopVar.identifier
                                << " = constant\n";
                        }
                        else if (loopVar.value.nodeType == ParserValue::Node::variable) {
                            out << setting::CommentCharacter << " Declaration of: mut " << loopVar.typeName << " "
                                << loopVar.identifier
                                << " = variable\n";
                        }
                        else {
                            out << setting::CommentCharacter << " Declaration of: mut " << loopVar.typeName << " "
                                << loopVar.identifier
                                << "\n";
                        }
                    }

                    // this one is guaranteed to have value or be 0
                    std::string source = CalculateExpression(variables, out, var.singleSize, loopVar.value, type.type,
                                                             {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                        << variables.pushAndStr(var) << "\n";
                }

                uint16_t larger;
                {
                    auto left = GetValueByteSize(loop.condition.left, variables);
                    auto right = GetValueByteSize(loop.condition.right, variables);
                    larger = (left > right ? left : right);
                }

                // get the registers ready
                // TODO: add type check here
                out << ".L" << returnLabelCounter++ << "." << function->name << ":\n";
                std::string right = CalculateExpression(variables, out, larger, loop.condition.left, 0, {0, 0, 1, 0});
                std::string left = CalculateExpression(variables, out, larger, loop.condition.right, 0, {1, 1, 0, 0});
                forLevels.push(returnLabels.size() + 1);
                forPointers.push(&loop);
                out << "cmp" << AddInstructionPostfix(larger) << "    " << left << ", " << right << "\n";
                if (loop.condition.type == ParserCondition::Type::greater) {
                    out << "jbe     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (loop.condition.type == ParserCondition::Type::greaterEqual) {
                    out << "jb      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (loop.condition.type == ParserCondition::Type::lesser) {
                    out << "jge     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (loop.condition.type == ParserCondition::Type::lesserEqual) {
                    out << "jg      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (loop.condition.type == ParserCondition::Type::equals) {
                    out << "jne     " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                else if (loop.condition.type == ParserCondition::Type::notEquals) {
                    out << "je      " << ".L" << returnLabelCounter << "." << function->name << "\n";
                }
                // freeing temp variables
                if (right[0] != '%' and right[0] != '$' and variables.findByOffset(right).name.empty()) {
                    variables.free(right);
                }
                break;
            }

            case FunctionInstruction::Type::declaration: {
                const DeclarationInstruction& dec = *function->instructions[current].variant.declarationInstruction;
                if (setting::OutputDodoInstructions) {
                    if (dec.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let")
                            << " " << dec.typeName << " " << dec.name
                            << " = expression\n";
                    }
                    else if (dec.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let")
                            << " " << dec.typeName << " " << dec.name
                            << " = constant\n";
                    }
                    else if (dec.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let")
                            << " " << dec.typeName << " " << dec.name
                            << " = variable\n";
                    }
                    else {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let")
                            << " " << dec.typeName << " " << dec.name
                            << "\n";
                    }
                }
                StackVariable var;
                const ParserType& type = parserTypes[dec.typeName];
                var.singleSize = type.size;
                var.amount = 1;
                var.name = dec.name;
                var.type = type.type;
                var.typeName = dec.typeName;
                var.isMutable = dec.isMutable;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    std::string source = CalculateExpression(variables, out, var.singleSize, dec.expression, type.type,
                                                             {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                        << variables.pushAndStr(var) << "\n";
                }
                else {
                    // declaring with a zero to avoid trash or exploits(?), also for debugging
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    $0, " << variables.pushAndStr(var)
                        << "\n";
                }
                break;
            }
            case FunctionInstruction::Type::returnValue: {
                const ReturnInstruction& ret = *function->instructions[current].variant.returnInstruction;
                if (setting::OutputDodoInstructions) {
                    if (ret.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Expression return\n";
                    }
                    else if (ret.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Constant return\n";
                    }
                    else if (ret.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Variable return\n";
                    }
                    else {
                        out << setting::CommentCharacter << " Valueless return\n";
                    }
                }


                ParserType& type = parserTypes[function->returnType];

                std::string source = CalculateExpression(variables, out, type.size, ret.expression, type.type);
                if (source != "%rax") {
                    out << "mov" << AddInstructionPostfix(type.size) << "    " << source << ", "
                        << AddRegisterA(type.size) << "\n";
                }
                out << "popq    %rbp\n";
                out << "ret\n";

                didReturn = true;
                break;
            }
            case FunctionInstruction::Type::valueChange: {
                const ValueChangeInstruction& val = *function->instructions[current].variant.valueChangeInstruction;
                if (setting::OutputDodoInstructions) {
                    if (val.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name
                            << " = expression\n";
                    }
                    else if (val.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name
                            << " = constant\n";
                    }
                    else if (val.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name
                            << " = variable\n";
                    }
                    else {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name << "\n";
                    }
                }
                StackVariable& var = variables.find(val.name);
                const ParserType& type = parserTypes[var.typeName];
                if (not var.isMutable) {
                    CodeError("Variable: " + var.name + " is immutable!");
                }

                if (val.expression.nodeType != ParserValue::Node::empty) {
                    std::string source = CalculateExpression(variables, out, var.singleSize, val.expression, type.type,
                                                             {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                        << var.getAddress() << "\n";
                }
                else {
                    CodeError("No expression in value assignment!");
                }
                break;
            }
            case FunctionInstruction::Type::functionCall: {
                const FunctionCallInstruction& fun = *function->instructions[current].variant.functionCallInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " Function call to: " << fun.functionName << "(...)\n";
                }

                GenerateFunctionCall(out, variables, fun.functionName, 8, 0, &fun.arguments);
                break;
            }
        }
    }

    if (not didReturn) {
        if (function->returnType != "void" and not function->returnType.empty()) {
            // TODO: make this smarter
            CodeError("Value not returned at the end of a function!");
        }

    }
}

void GenerateCode() {
    doneParsing = true;
    std::ofstream out;
    if (!fs::is_directory("build")) {
        fs::create_directory("build");
    }
    out.open("build/out.s");
    if (!out.is_open()) {
        CodeError("Failed to open output .asm file");
    }

    out << "# Generated by dodo lang compiler by SKNI \"KOD\"\n";
    out << "# Target architecture: " << TargetArchitecture << "\n";
    out << "# Target system: " << TargetSystem << "\n";

    out << ".data\n";
    // ...
    out << ".text\n";
    // ...
    out << ".global _start\n";
    // ...

    // TODO: add a check to see if given function is used at all
    for (auto& n: parserFunctions.map) {
        GenerateFunction(n.second.name, out);
    }

    out << "\n";
    out << "_start:\n";
    out << "call    main\n";
    out << "movq    %rax, %rdi\n";
    out << "movq    $60, %rax\n";
    out << "syscall\n";
}


