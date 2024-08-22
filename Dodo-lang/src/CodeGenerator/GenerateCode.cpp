#include "GenerateCode.hpp"
#include "ParserVariables.hpp"
#include "StackVector.hpp"
#include "GeneratorSettings.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <utility>

namespace fs = std::filesystem;

const char *CodeException::what() {
    return "Code generation failed!";
}

// converts a string to a value that is the decimal number of binary representation of the number
uint64_t ConvertValue(const std::string& value, uint16_t bitSize) {
    // detecting what this value even is, yeah lexer could do that but I want to be sure it works exactly as intended
    std::string temp = value;

    bool isNegative = false;

    if (temp.front() == '-') {
        if (temp.size() == 1) {
            CodeError("A '-' operand passed to numeric conversion!");
        }
        temp = temp.substr(1, temp.size() - 1);
        isNegative = true;
    }


    bool isFloatingPoint = false;
    for (auto& n : temp) {
        if (n == '.') {
            if (isFloatingPoint == false) {
                isFloatingPoint = true;
            }
            else {
                CodeError("Multi dot floating point number!");
            }
        }
    }

    if (isFloatingPoint) {
        // TODO: add floats
        return 0;
    }

    uint64_t number = std::stoll(temp);
    if (isNegative) {
        // shifts the one to the leftmost position of defines singleSize of var
        uint64_t theOne = 1;
        theOne <<= (bitSize - 1);
        uint64_t negativeBase = -1;
        negativeBase >>= (64 - bitSize + 1);
        // binary or's the values to always produce the binary representation
        number = (negativeBase - number + 1) | theOne;
    }

    return number;
}

uint64_t ConvertValue(const ParserValue& value, uint16_t bitSize) {
    // detecting what this value even is, yeah lexer could do that but I want to be sure it works exactly as intended

    if (value.secondType == ParserValue::Value::floating) {
        // TODO: add floats
        return 0;
    }

    if (value.isNegative) {
        uint64_t number = std::stoll(value.value->substr(1, value.value->size() - 1));
        // shifts the one to the leftmost position of defines singleSize of var
        uint64_t theOne = 1;
        theOne <<= (bitSize - 1);
        uint64_t negativeBase = -1;
        negativeBase >>= (64 - bitSize + 1);
        // binary or's the values to always produce the binary representation
        number = (negativeBase - number + 1) | theOne;
        return number;
    }

    // just an integer
    return std::stoll(*value.value);
}

void CodeError(const std::string message) {
    std::cout << "ERROR! Code generation failed : " << message << "\n";
    throw CodeException();
}

// converts value from given location on stack to given singleSize and returns it as a any or specified register value if asked
// pass registers as in "%reg"
std::string ConvertSizeFromStack(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, uint64_t offset,
                                  bool mustBeReg = false, StackVector* stack = nullptr, bool mustUseGivenReg = false, RegisterNames registers = {}) {
    switch (originalSize) {
        case 1:
            switch(targetSize) {
                case 1:
                    if (mustBeReg) {
                        out << "movb    -" << offset << "(%rbp), " << registers.size1 <<"\n";
                        return registers.size1;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 2:
                    out << "movb    -" << offset << "(%rbp), %al\n";
                    out << "cbtw\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movw    %ax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size2 != "%ax") {
                            out << "movw    %ax, " << registers.size2 << "\n";
                        }
                        return registers.size2;
                    }
                    return "%ax";
                case 4:
                    out << "movsb   -" << offset << "(%rbp), %eax\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movl    %eax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size4 != "%eax") {
                            out << "movl    %eax, " << registers.size4 << "\n";
                        }
                        return registers.size4;
                    }
                    return "%eax";
                case 8:
                    out << "movsb   -" << offset << "(%rbp), %eax\n";
                    out << "cltq\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movq    %rax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size8 != "%rax") {
                            out << "movq    %rax, " << registers.size8 << "\n";
                        }
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
        case 2:
            switch(targetSize) {
                case 1:
                    if (mustBeReg) {
                        out << "movb    -" << offset << "(%rbp), " << registers.size1 <<"\n";
                        return registers.size1;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 2:
                    if (mustBeReg) {
                        out << "movw    -" << offset << "(%rbp), " << registers.size2 <<"\n";
                        return registers.size2;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 4:
                    out << "movsw   -" << offset << "(%rbp), %eax\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movl    %eax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size4 != "%eax") {
                            out << "movl    %eax, " << registers.size4 << "\n";
                        }
                        return registers.size4;
                    }
                    return "%eax";
                case 8:
                    out << "movsw   -" << offset << "(%rbp), %eax\n";
                    out << "cltq\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movq    %rax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size8 != "%rax") {
                            out << "movq    %rax, " << registers.size8 << "\n";
                        }
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
        case 4:
            switch(targetSize) {
                case 1:
                    if (mustBeReg) {
                        out << "movb    -" << offset << "(%rbp), " << registers.size1 <<"\n";
                        return registers.size1;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 2:
                    if (mustBeReg) {
                        out << "movw    -" << offset << "(%rbp), " << registers.size2 <<"\n";
                        return registers.size2;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 4:
                    if (mustBeReg) {
                        out << "movl    -" << offset << "(%rbp), " << registers.size4 <<"\n";
                        return registers.size4;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 8:
                    out << "movl    -" << offset << "(%rbp), %eax\n";
                    out << "cltq\n";
                    if (stack) {
                        StackVariable var;
                        var.singleSize = targetSize;
                        var.amount = 1;
                        std::string address = stack->pushAndStr(var);
                        out << "movq    %rax, " << address << "\n";
                        return address;
                    }
                    if (mustUseGivenReg) {
                        if (registers.size8 != "%rax") {
                            out << "movq    %rax, " << registers.size8 << "\n";
                        }
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
        case 8:
            switch(targetSize) {
                case 1:
                    if (mustBeReg) {
                        out << "movb    -" << offset << "(%rbp), " << registers.size1 <<"\n";
                        return registers.size1;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 2:
                    if (mustBeReg) {
                        out << "movw    -" << offset << "(%rbp), " << registers.size2 <<"\n";
                        return registers.size2;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 4:
                    if (mustBeReg) {
                        out << "movl    -" << offset << "(%rbp), " << registers.size4 <<"\n";
                        return registers.size4;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                case 8:
                    if (mustBeReg) {
                        out << "movq    -" << offset << "(%rbp), " << registers.size8 <<"\n";
                        return registers.size8;
                    }
                    return '-' + std::to_string(offset) + "(%rbp)";
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
        default:
            CodeError("Invalid singleSize for singleSize conversion!");
    }
    // will never get here
    return "";
}

uint64_t GetTypeSize(const std::string& typeName) {
    if (parserTypes.isKey(typeName)) {
        return parserTypes[typeName].size;
    }
    else {
        CodeError("Variable of non-existent type!");
    }
    return 0;
}

struct ValueType {
    uint8_t reg:1 = 1;
    uint8_t val:1 = 1;
    uint8_t sta:1 = 1;
    uint8_t hea:1 = 1;
    ValueType(uint8_t reg, uint8_t val, uint8_t sta, uint8_t hea) : reg(reg), val(val), sta(sta), hea(hea) {}
};

// takes a mathematical expression and returns the value to put wherever the user wants
std::string CalculateExpression(StackVector& variables, std::ofstream& out, uint16_t outputSize,
                                const ParserValue& expression, ValueType valueType = {1, 1, 1, 1}, RegisterNames registers = {}) {

    if (expression.nodeType == ParserValue::Node::empty) {
        // shouldn't happen afaik
        CodeError("Empty node in wrong place!");
    }

    // if given node is a constant just return it's value
    if (expression.nodeType == ParserValue::constant) {
        uint64_t number = ConvertValue(*expression.value, 64);
        if (valueType.val) {
            return "$" + std::to_string(number);
        }
        else if (valueType.reg) {
            out << "mov"  << AddInstructionPostfix(outputSize) << "    $" << std::to_string(number) << ", " << registers.registerBySize(outputSize) << "\n";
            return registers.size8;
        }
        else {
            CodeError("Unsupported value return type for node type!");
            return "";
        }
    }

    // if given node is a variable
    if (expression.nodeType == ParserValue::Node::variable) {
        auto& var = variables.find(*expression.value);
        if (not valueType.reg) {
            // strictly stack
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, false, &variables);
        }
        if (not valueType.sta) {
            // strictly register
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset, true, nullptr, true, registers);
        }
        else {
            return ConvertSizeFromStack(out, var.singleSize, outputSize, var.offset);
        }
    }

    // if given node is an actual multi thingy expression
    if (expression.nodeType == ParserValue::Node::operation) {
        // for now functions are going to be separate
        if (expression.secondType == ParserValue::Operation::functionCall) {
            if (valueType.reg) {
                return GenerateFunctionCall(out, variables.lastOffset(), variables.registerOffset, *expression.value,
                                     outputSize, registers);
            }
            else if (valueType.sta) {
                StackVariable var;
                var.singleSize = outputSize;
                var.amount = 1;
                const StackVariable& pushed = variables.push(var);
                return GenerateFunctionCall(out, variables.lastOffset(), variables.registerOffset, *expression.value,
                                     outputSize, pushed.getAddressAsRegisterNames());
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

        std::string left = CalculateExpression(variables, out, largestSize, *expression.left, {0, 1, 1, 1});
        std::string right;
        if (expression.secondType == ParserValue::Operation::division) {
            right = CalculateExpression(variables, out, largestSize, *expression.right, {1, 0, 0, 0}, {"%bl", "%bx", "%ebx", "%rbx"});
        }
        else {
            right = CalculateExpression(variables, out, largestSize, *expression.right, {0, 1, 1, 1});
        }


        out << "mov" << AddInstructionPostfix(largestSize) << "    " << left << ", " << AddRegisterA(largestSize) << "\n";
        if (expression.left->nodeType == ParserValue::Node::operation) {
            variables.free(left);
        }
        if (leftLarger and not (expression.left->nodeType == ParserValue::Node::variable and
        variables.find(*expression.left->value).singleSize >= largestSize)) {
            variables.free(left);
        }

        // operations really only change this part
        switch (expression.secondType) {
            case ParserValue::Operation::addition:
                out << "add" << AddInstructionPostfix(largestSize) << "    " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::subtraction:
                out << "sub" << AddInstructionPostfix(largestSize) << "    " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::multiplication:
                out << "imul" << AddInstructionPostfix(largestSize) << "   " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            case ParserValue::Operation::division:
                out << "movq    $0, %rdx\n";
                out << "idiv" << AddInstructionPostfix(largestSize) << "   " << right << ", " << registers.registerBySize(largestSize) << "\n";
                break;
            default:
                CodeError("Unsupported operation type!");
        }


        if (expression.right->nodeType == ParserValue::Node::operation) {
            variables.free(right);
        }
        if (rightLarger and not (expression.right->nodeType == ParserValue::Node::variable and
                                variables.find(*expression.right->value).singleSize >= largestSize)) {
            variables.free(right);
        }

        std::string reg = ConvertSizeInRegister(out, largestSize, outputSize);

        if (valueType.reg) {
            return reg;
        }
        else if (valueType.sta) {
            StackVariable var;
            var.singleSize = largestSize;
            var.amount = 1;

            std::string varAddress =  variables.pushAndStr(var);
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

    if (setting::OutputDodoInstructions) {
        out << "\n" << setting::CommentCharacter << " Declaration of function: " << function->returnType << " " << identifier << "\n";
    }

    if (identifier == "main") {
        out << "_start:\n";
    }
    else {
        out << identifier << ":\n";
    }
    if (TargetArchitecture == "X86-64") {
        out << "pushq   %rbp\n";
        out << "movq    %rsp, %rbp\n";
    }

    bool didReturn = false;
    for (const auto& current : function->instructions) {
        switch (current.type) {
            case FunctionInstruction::Type::declaration: {
                const DeclarationInstruction &dec = *current.Variant.declarationInstruction;
                if (setting::OutputDodoInstructions) {
                    if (dec.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = expression\n";
                    } else if (dec.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = constant\n";
                    } else if (dec.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << " = variable\n";
                    } else {
                        out << setting::CommentCharacter << " Declaration of: " << (dec.isMutable ? "mut" : "let") << " " << dec.typeName << " " << dec.name
                            << "\n";
                    }
                }
                StackVariable var;
                var.singleSize = GetTypeSize(dec.typeName);
                var.amount = 1;
                var.name = dec.name;
                var.isMutable = dec.isMutable;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    std::string source = CalculateExpression(variables, out, var.singleSize, dec.expression, {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", " << variables.pushAndStr(var) << "\n";
                }
                else {
                    // declaring with a zero to avoid trash or exploits(?), also for debugging
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    $0, " << variables.pushAndStr(var) << "\n";
                }
                break;
            }
            case FunctionInstruction::Type::returnValue: {
                const ReturnInstruction &ret = *current.Variant.returnInstruction;
                if (setting::OutputDodoInstructions) {
                    if (ret.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Expression return\n";
                    } else if (ret.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Constant return\n";
                    } else if (ret.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Variable return\n";
                    } else {
                        out << setting::CommentCharacter << " Valueless return\n";
                    }
                }
                if (identifier == "main") {
                    if (TargetArchitecture == "X86-64") {

                        std::string source = CalculateExpression(variables, out, 8, ret.expression);
                        out << "movq    " << source << ", %rdi\n";

                        // interrupt number
                        out << "movq    $60, %rax\n";
                        // call out to the kernel to end this misery
                        out << "syscall\n";
                    }
                } else {
                    if (TargetArchitecture == "X86-64") {

                        std::string source = CalculateExpression(variables, out, 8, ret.expression);
                        if (source != "%rax") {
                            out << "movq    " << source << ", %rax\n";
                        }
                        out << "popq    %rbp\n";
                        out << "ret\n";
                    }
                }
                didReturn = true;
                break;
            }
            case FunctionInstruction::Type::valueChange:
            {
                const ValueChangeInstruction &val = *current.Variant.valueChangeInstruction;
                if (setting::OutputDodoInstructions) {
                    if (val.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name << " = expression\n";
                    } else if (val.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Value change of variable: " << val.name << " = constant\n";
                    } else if (val.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name << " = variable\n";
                    } else {
                        out << setting::CommentCharacter << " Value change of variable: " << " " << val.name << "\n";
                    }
                }
                StackVariable &var = variables.find(val.name);
                if (not var.isMutable) {
                    CodeError("Variable: " + var.name + " is immutable!");
                }

                if (val.expression.nodeType != ParserValue::Node::empty) {
                    // TODO: add memory to memory move, movsq didn't work in visualizer so I didn't risk it
                    std::string source = CalculateExpression(variables, out, var.singleSize, val.expression,
                                                             {1, 1, 0, 1});
                    out << "mov" << AddInstructionPostfix(var.singleSize) << "    " << source << ", "
                        << var.getAddress() << "\n";
                } else {
                    CodeError("No expression in value assignment!");
                }
                break;
            }
            case FunctionInstruction::Type::functionCall:
            {
                const FunctionCallInstruction &fun = *current.Variant.functionCallInstruction;
                if (setting::OutputDodoInstructions) {
                    out << setting::CommentCharacter << " Function call to: " << fun.functionName << "(...)\n";
                }

                GenerateFunctionCall(out, variables.lastOffset(), variables.registerOffset, fun.functionName, 8);
                break;
            }
        }
    }

    if (not didReturn) {
        if (identifier == "main") {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $0, %rdi\n";
                out << "movq    $60, %rax\n";
                out << "syscall\n";
            }
        }
        else {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $0, %rax\n";
                out << "popq    %rbp\n";
                out << "ret\n";
            }
        }
    }
}

void GenerateCode() {
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
    for (auto& n : parserFunctions.map) {
        GenerateFunction(n.second.name, out);
    }
}


