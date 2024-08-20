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
uint64_t ConvertNumber(const std::string& value, uint16_t bitSize) {
    // detecting what this value even is, yeah lexer could do that but I want to be sure it works exactly as intended
    std::string temp = value;
    if (temp.empty()) {
        CodeError("Empty string passed to numeric conversion!");
    }

    bool isNegative = false;

    if (temp.front() == '-') {
        if (temp.size() == 1) {
            CodeError("A '-' operand passed to numeric conversion!");
        }
        temp = temp.substr(1, temp.size() - 1);
        isNegative = true;
    }

    // checking if it's a 0x..., 0b... or 0o...
    if (temp.size() > 2 and temp.front() == 0 and (temp[1] == 'x' or temp[1] == 'b' or temp[1] == 'o')) {
        // TODO: add non decimals
        return 0;
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

// takes a mathematical expression and returns the value to put wherever the user wants
std::string CalculateExpression(const StackVector& variables, std::ofstream& out, uint16_t outputSize, const ParserValue& expression) {

    // if given node is a constant just return it's value
    if (expression.nodeType == ParserValue::constant) {
        // fixed sizes for now
        if (outputSize > 4) {
            uint64_t number = ConvertNumber(*expression.value, 64);
            return "$" + std::to_string(number);
        }
        else if (outputSize > 2) {
            uint64_t number = ConvertNumber(*expression.value, 32);
            if (number > 4294967295) {
                CodeError("Invalid number for this size of variable!");
            }
            return "$" + std::to_string(number);
        }
        else if (outputSize > 1) {
            uint64_t number = ConvertNumber(*expression.value, 16);
            if (number > 65535) {
                CodeError("Invalid number for this size of variable!");
            }
            return "$" + std::to_string(number);
        }
        else {
            uint64_t number = ConvertNumber(*expression.value, 8);
            if (number > 255) {
                CodeError("Invalid number for this size of variable!");
            }
            return "$" + std::to_string(number);
        }
    }

    // if given node is a variable
    if (expression.nodeType == ParserValue::Node::variable) {

    }

    CodeError("Expression generation error!");
    return "";
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
                var.offset = variables.lastOffset() + (var.size * var.amount);
                var.name = dec.name;

                if (dec.expression.nodeType != ParserValue::Node::empty) {
                    // TODO: Change this into a dedicated function with all the checks etc
                    if (var.size > 4) {
                        if (var.offset % 8 != 0) {
                            var.offset = (var.offset / 8 + 1) * 8;
                        }
                        uint64_t number = ConvertNumber(*dec.expression.value, 64);
                        out << "movq    " << CalculateExpression(variables, out, var.size, dec.expression) << ", -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 2) {
                        if (var.offset % 4 != 0) {
                            var.offset = (var.offset / 4 + 1) * 4;
                        }
                        uint64_t number = ConvertNumber(*dec.expression.value, 32);
                        if (number > 4294967295) {
                            CodeError("Invalid number for this size of variable!");
                        }
                        out << "movl    " << CalculateExpression(variables, out, var.size, dec.expression) << ", -" << var.offset << "(%rbp)\n";
                    }
                    else if (var.size > 1) {
                        if (var.offset % 2 != 0) {
                            var.offset = (var.offset / 2 + 1) * 2;
                        }
                        uint64_t number = ConvertNumber(*dec.expression.value, 16);
                        if (number > 65535) {
                            CodeError("Invalid number for this size of variable!");
                        }
                        out << "movw    " << CalculateExpression(variables, out, var.size, dec.expression) << ", -" << var.offset << "(%rbp)\n";
                    }
                    else {
                        uint64_t number = ConvertNumber(*dec.expression.value, 8);
                        if (number > 255) {
                            CodeError("Invalid number for this size of variable!");
                        }
                        out << "movb    " << CalculateExpression(variables, out, var.size, dec.expression) << ", -" << var.offset << "(%rbp)\n";
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
                        // interrupt number
                        out << "movq    $60, %rax\n";
                        if (ret.expression.nodeType == ParserValue::Node::empty) {
                            // shouldn't happen afaik
                            CodeError("Empty return statement!");
                        }
                        if (ret.expression.nodeType == ParserValue::Node::constant) {
                            out << "movq    $" << ConvertNumber(*ret.expression.value, 64) << ", %rdi\n";
                        }
                        else if (ret.expression.nodeType == ParserValue::Node::variable) {
                            auto& var = variables.find(*ret.expression.value);
                            if (var.size > 4) {
                                out << "movq    -" << var.offset << "(%rbp), %rdi\n";
                            }
                            else if (var.size > 2) {
                                // might be a stupid way
                                out << "movq    $0, %rdi\n";
                                out << "movl    -" << var.offset << "(%rbp), %edi\n";
                            }
                            else if (var.size > 1) {
                                // might be an even dumber way
                                out << "movq    $0, %rbx\n";
                                out << "movw    -" << var.offset << "(%rbp), %bx\n";
                                out << "movq    %rbx, %rdi\n";
                            }
                            else {
                                // might be an even dumber way
                                out << "movq    $0, %rbx\n";
                                out << "movb    -" << var.offset << "(%rbp), %bl\n";
                                out << "movq    %rbx, %rdi\n";
                            }
                        }
                        else {
                            // because complex expressions are not yet supported
                            out << "movq    $0, %rdi\n";
                        }
                        // call out to the kernel to end this misery
                        out << "syscall\n";
                    }
                    else if (TargetArchitecture == "X86-32") {
                        // 32 bit not yet supported
                        out << "movl    $1, %eax\n";
                        out << "movl    $0, %ebx\n";
                        out << "int     $80\n";
                    }
                }
                else {
                    if (TargetArchitecture == "X86-64") {
                        out << "movq    $0, %rax\n";
                        out << "popq    %rbp\n";
                        out << "ret\n";
                    }
                    else if (TargetArchitecture == "X86-32") {
                        out << "movl    $0, %eax\n";
                        out << "popl    %ebp\n";
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
                out << "movq    $60, %rax\n";
                out << "movq    $0, %rdi\n";
                out << "syscall\n";
            }
            else if (TargetArchitecture == "X86-32") {
                out << "movl    $1, %eax\n";
                out << "movl    $0, %ebx\n";
                out << "int     $80\n";
            }
        }
        else {
            if (TargetArchitecture == "X86-64") {
                out << "movq    $0, %rax\n";
                out << "popq    %rbp\n";
                out << "ret\n";
            }
            else if (TargetArchitecture == "X86-32") {
                out << "movl    $0, %eax\n";
                out << "popl    %ebp\n";
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
    GenerateFunction("main", out);
}


