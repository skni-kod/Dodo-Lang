#include "GenerateCode.hpp"
#include "ParserVariables.hpp"
#include "StackVector.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
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
        // shifts the one to the leftmost position of defines size of var
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
        // shifts the one to the leftmost position of defines size of var
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

uint64_t GetTypeSize(const std::string& typeName) {
    if (parserTypes.isKey(typeName)) {
        return parserTypes[typeName].size;
    }
    else {
        CodeError("Variable of non-existent type!");
    }
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
                                const ParserValue& expression, ValueType valueType = {1, 1, 1, 1}) {

    if (expression.nodeType == ParserValue::Node::empty) {
        // shouldn't happen afaik
        CodeError("Empty node in wrong place!");
    }

    // if given node is a constant just return it's value
    if (expression.nodeType == ParserValue::constant) {
        // fixed sizes for now
        if (outputSize > 4) {
            uint64_t number = ConvertValue(*expression.value, 64);
            if (valueType.val) {
                return "$" + std::to_string(number);
            }
            else if (valueType.reg) {
                out << "movq    $" << std::to_string(number) << ", %rax\n";
                return "%rax";
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }

        }
        else if (outputSize > 2) {
            uint64_t number = ConvertValue(*expression.value, 32);
            if (number > 4294967295) {
                CodeError("Invalid number for this size of variable!");
            }
            if (valueType.val) {
                return "$" + std::to_string(number);
            }
            else if (valueType.reg) {
                out << "movl    $" << std::to_string(number) << ", %eax\n";
                return "%eax";
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }
        }
        else if (outputSize > 1) {
            uint64_t number = ConvertValue(*expression.value, 16);
            if (number > 65535) {
                CodeError("Invalid number for this size of variable!");
            }
            if (valueType.val) {
                return "$" + std::to_string(number);
            }
            else if (valueType.reg) {
                out << "movw    $" << std::to_string(number) << ", %ax\n";
                return "%ax";
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }
        }
        else {
            uint64_t number = ConvertValue(*expression.value, 8);
            if (number > 255) {
                CodeError("Invalid number for this size of variable!");
            }
            if (valueType.val) {
                return "$" + std::to_string(number);
            }
            else if (valueType.reg) {
                out << "movb    $" << std::to_string(number) << ", %al\n";
                return "%al";
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }
        }
    }

    // if given node is a variable
    if (expression.nodeType == ParserValue::Node::variable) {
        auto& var = variables.find(*expression.value);
        if (var.size == outputSize) {
            // situation is simple, just return the stack location
            if (valueType.sta) {
                return "-" + std::to_string(var.offset) + "(%rbp)";
            }
            else if (valueType.reg) {
                switch (var.size) {
                    case 8:
                        out << "movq    -" << var.offset << "(%rbp), %rax\n";
                        return "%rax";
                    case 4:
                        out << "movl    -" << var.offset << "(%rbp), %eax\n";
                        return "%eax";
                    case 2:
                        out << "movw    -" << var.offset << "(%rbp), %ax\n";
                        return "%ax";
                    case 1:
                        out << "movb    -" << var.offset << "(%rbp), %al\n";
                        return "%al";
                    default:
                        CodeError("Invalid variable size!");
                }
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }

        }
        else {
            // TODO: add conversion skip with unsigned value move
            // TODO: figure out the mystery of movsb and movsw instructions
            if (valueType.reg) {
                switch (outputSize) {
                    case 8:
                        switch (var.size) {
                            case 4:
                                out << "movl    -" << var.offset << "(%rbp), %eax\n";
                                out << "cltq\n";
                                return "%rax";
                            case 2:
                                out << "movsw   -" << var.offset << "(%rbp), %eax\n";
                                out << "cwtl\n";
                                return "%rax";
                            case 1:
                                out << "movsb   -" << var.offset << "(%rbp), %eax\n";
                                out << "cwtl\n";
                                return "%rax";
                            default:
                                CodeError("Invalid variable size!");
                        }
                    case 4:
                        switch (var.size) {
                            case 8:
                                out << "movq    -" << var.offset << "(%rbp), %rax\n";
                                return "%eax";
                            case 2:
                                out << "movsw   -" << var.offset << "(%rbp), %eax\n";
                                return "%eax";
                            case 1:
                                out << "movsb   -" << var.offset << "(%rbp), %eax\n";
                                return "%eax";
                            default:
                                CodeError("Invalid variable size!");
                        }
                    case 2:
                        switch (var.size) {
                            case 8:
                                out << "movq    -" << var.offset << "(%rbp), %rax\n";
                                return "%ax";
                            case 4:
                                out << "movl    -" << var.offset << "(%rbp), %eax\n";
                                return "%ax";
                            case 1:
                                out << "movb    -" << var.offset << "(%rbp), %al\n";
                                out << "cbtw\n";
                                return "%ax";
                            default:
                                CodeError("Invalid variable size!");
                        }
                    case 1:
                        switch (var.size) {
                            case 8:
                                out << "movq    -" << var.offset << "(%rbp), %rax\n";
                                return "%al";
                            case 4:
                                out << "movl    -" << var.offset << "(%rbp), %eax\n";
                                return "%al";
                            case 2:
                                out << "movw    -" << var.offset << "(%rbp), %ax\n";
                                return "%al";
                            default:
                                CodeError("Invalid variable size!");
                        }
                    default:
                        CodeError("Invalid output size!");

                }
            }
            else if (valueType.sta) {
                CodeError("Adaptive size variable return from stack not yet supported!");
            }
            else {
                CodeError("Unsupported value return type for node type!");
            }
        }
    }

    // if given node is an actual multi thingy expression
    if (expression.nodeType == ParserValue::Node::operation) {
        switch (expression.secondType) {
            case ParserValue::Operation::addition:
            {
                uint8_t largestSize = outputSize;
                if (expression.left->nodeType == ParserValue::Node::variable) {
                    auto& var = variables.find(*expression.left->value);
                    if (var.size > largestSize) {
                        largestSize = var.size;
                    }
                }
                if (expression.right->nodeType == ParserValue::Node::variable) {
                    auto& var = variables.find(*expression.right->value);
                    if (var.size > largestSize) {
                        largestSize = var.size;
                    }
                }

                std::string left = CalculateExpression(variables, out, largestSize, *expression.left, {0, 1, 1, 1});
                std::string right = CalculateExpression(variables, out, largestSize, *expression.right, {0, 1, 1, 1});

                switch(largestSize) {
                    case 8:
                    {
                        out << "movq    " << left << ", %rax\n";
                        out << "addq    " << right << ", %rax\n";
                    }
                    break;
                    case 4:
                    {
                        out << "movl    " << left << ", %eax\n";
                        out << "addl    " << right << ", %eax\n";
                    }
                    break;
                    case 2:
                    {
                        out << "movw    " << left << ", %ax\n";
                        out << "addw    " << right << ", rax\n";
                    }
                    break;
                    case 1:
                    {
                        out << "movb    " << left << ", %al\n";
                        if (expression.left->nodeType == ParserValue::Node::operation) {
                            variables.free(left);
                        }
                        out << "addb    " << right << ", %al\n";
                        if (expression.right->nodeType == ParserValue::Node::operation) {
                            variables.free(right);
                        }

                        if (valueType.reg) {
                            return "%al";
                        }
                        else if (valueType.sta) {
                            StackVariable var;
                            var.size = 1;
                            var.amount = 1;
                            std::string temp =  variables.pushAndStr(var);
                            out << "movb    %al, " << temp << "\n";
                            return temp;
                        }

                    }
                    break;
                }
                CodeError("Unsupported value return type for node type!");
            }

            default:
                CodeError("Unhandled operation type!");
        }
    }

    CodeError("Unhandled expression path!");
    return "$0";
}

void GenerateFunction(const std::string& identifier, std::ofstream& out) {
    if (identifier == "main") {
        out << "_start:\n";
        if (TargetArchitecture == "X86-64") {
            out << "pushq   %rbp\n";
            out << "movq    %rsp, %rbp\n";
        }
        else if (TargetArchitecture == "X86-32") {
            out << "pushd   %ebp\n";
            out << "movd    %esp, %ebp\n";
        }
    }
    else {
        out << identifier << ":\n";
        if (TargetArchitecture == "X86-64") {
            out << "pushq   %rbp\n";
            out << "movq    %rsp, %rbp\n";
        }
        else if (TargetArchitecture == "X86-32") {
            out << "pushd   %ebp\n";
            out << "movd    %esp, %ebp\n";
        }
    }
    StackVector variables;

    const ParserFunction* function = &parserFunctions[identifier];
    // return logic will be changed, probably
    bool didReturn = false;
    for (const auto& current : function->instructions) {
        switch (current.type) {
            case FunctionInstruction::Type::declaration:
            {
                const DeclarationInstruction& dec = *current.Variant.declarationInstruction;
                StackVariable var;
                var.size = GetTypeSize(dec.typeName);
                var.amount = 1;
                var.name = dec.name;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    // TODO: Change this into a dedicated function with all the checks etc
                    if (var.size > 4) {
                        std::string source = CalculateExpression(variables, out, var.size, dec.expression, {1, 1, 0, 1});
                        out << "movq    " << source << ", " << variables.pushAndStr(var) << "\n";
                    }
                    else if (var.size > 2) {
                        std::string source = CalculateExpression(variables, out, var.size, dec.expression, {1, 1, 0, 1});
                        out << "movl    " << source << ", " << variables.pushAndStr(var) << "\n";
                    }
                    else if (var.size > 1) {
                        std::string source = CalculateExpression(variables, out, var.size, dec.expression, {1, 1, 0, 1});
                        out << "movw    " << source << ", " << variables.pushAndStr(var) << "\n";
                    }
                    else {
                        std::string source = CalculateExpression(variables, out, var.size, dec.expression, {1, 1, 0, 1});
                        out << "movb    " << source << ", " << variables.pushAndStr(var) << "\n";
                    }
                }
                else {
                    // declaring with a zero to avoid trash or exploits(?), also for debugging
                    if (var.size > 4) {
                        if (var.offset % 8 != 0) {
                            var.offset = (var.offset / 8 + 1) * 8;
                        }
                        out << "movq    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 2) {
                        if (var.offset % 4 != 0) {
                            var.offset = (var.offset / 4 + 1) * 4;
                        }
                        out << "movl    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 1) {
                        if (var.offset % 2 != 0) {
                            var.offset = (var.offset / 2 + 1) * 2;
                        }
                        out << "movw    $0, -" << var.offset << "(%rbp)\n";
                    }
                    else {
                        out << "movb    $0, -" << var.offset << "(%rbp)\n";
                    }
                }
                variables.vec.back().push_back(var);
                break;
            }
            case FunctionInstruction::Type::returnValue:
                const ReturnInstruction& ret = *current.Variant.returnInstruction;
                // TODO: Change 0 to value
                if (identifier == "main") {
                    if (TargetArchitecture == "X86-64") {

                        std::string source = CalculateExpression(variables, out, 8, ret.expression);
                        out << "movq    " << source << ", %rdi\n";

                        // interrupt number
                        out << "movq    $60, %rax\n";
                        // call out to the kernel to end this misery
                        out << "syscall\n";
                    }
                }
                else {
                    if (TargetArchitecture == "X86-64") {
                        // value here;
                        out << "movq    $0, %rax\n";
                        out << "popq    %rbp\n";
                        out << "ret\n";
                    }
                }
                didReturn = true;
                break;
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

    // TODO: make this a loop for all functions
    GenerateFunction("main", out);
}


