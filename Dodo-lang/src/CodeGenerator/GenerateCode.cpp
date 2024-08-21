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

char AddInstructionPostfix(uint16_t size) {
    switch(size) {
        case 1:
            return 'b';
        case 2:
            return 'w';
        case 4:
            return 'l';
        case 8:
            return 'q';
        default:
            CodeError("Invalid size in postfix function!");
    }
    return 0;
}

std::string AddRegisterA(uint16_t size) {
    switch(size) {
        case 1:
            return "%al";
        case 2:
            return "%ax";
        case 4:
            return "%eax";
        case 8:
            return "%rax";
        default:
            CodeError("Invalid size in register A function!");
    }
    return "";
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

struct RegisterNames {
    std::string size1 = "%al";
    std::string size2 = "%ax";
    std::string size4 = "%eax";
    std::string size8 = "%rax";
    RegisterNames(std::string size1, std::string size2, std::string size4, std::string size8) :
        size1(std::move(size1)), size2(std::move(size2)), size4(std::move(size4)), size8(std::move(size8)) {}
    RegisterNames() = default;
    bool nonDefault() const {
        if (size1 != "%al" or size2 != "%ax" or size4 != "%eax" or size8 != "%rax") {
            return true;
        }
        return false;
    }
    std::string registerBySize(uint16_t size) {
        switch(size) {
            case 1:
                return size1;
            case 2:
                return size2;
            case 4:
                return size4;
            case 8:
                return size8;
            default:
                CodeError("Invalid size in custom register function!");
        }
        return "0";
    }
};

// converts value in given register to given size and if asked returns it to the source register
// pass registers as in "%reg"
std::string ConvertSizeInRegister(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, RegisterNames registers = {}, bool returnToOriginal = false) {
    switch (originalSize) {
        case 1:
            switch(targetSize) {
                case 1:
                    return registers.size1;
                case 2:
                    if (registers.size1 != "%al") {
                        out << "movb    " << registers.size1 << ", %al\n";
                    }
                    out << "cbtw\n";
                    if (returnToOriginal) {
                        out << "movw    %ax, " << registers.size2 << "\n";
                        return registers.size2;
                    }
                    return "%ax";
                case 4:
                    if (registers.size1 != "%al") {
                        out << "movb    " << registers.size1 << ", %al\n";
                    }
                    out << "cbtw\n";
                    out << "cwtl\n";
                    if (returnToOriginal) {
                        out << "movl    %eax, " << registers.size4 << "\n";
                        return registers.size4;
                    }
                    return "%eax";
                case 8:
                    if (registers.size1 != "%al") {
                        out << "movb    " << registers.size1 << ", %al\n";
                    }
                    out << "cbtw\n";
                    out << "cwtl\n";
                    out << "cltq\n";
                    if (returnToOriginal) {
                        out << "movq    %rax, " << registers.size8 << "\n";
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid size for size conversion!");
            }
        case 2:
            switch(targetSize) {
                case 1:
                    return registers.size1;
                case 2:
                    return registers.size2;
                case 4:
                    if (registers.size2 != "%ax") {
                        out << "movw    " << registers.size2 << ", %ax\n";
                    }
                    out << "cwtl\n";
                    if (returnToOriginal) {
                        out << "movl    %eax, " << registers.size4 << "\n";
                        return registers.size4;
                    }
                    return "%eax";
                case 8:
                    if (registers.size2 != "%ax") {
                        out << "movw    " << registers.size2 << ", %ax\n";
                    }
                    out << "cwtl\n";
                    out << "cltq\n";
                    if (returnToOriginal) {
                        out << "movq    %rax, " << registers.size8 << "\n";
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid size for size conversion!");
            }
        case 4:
            switch(targetSize) {
                case 1:
                    return registers.size1;
                case 2:
                    return registers.size2;
                case 4:
                    return registers.size4;
                case 8:
                    if (registers.size4 != "%eax") {
                        out << "movw    " << registers.size4 << ", %eax\n";
                    }
                    out << "cltq\n";
                    if (returnToOriginal) {
                        out << "movq    %rax, " << registers.size8 << "\n";
                        return registers.size8;
                    }
                    return "%rax";
                default:
                    CodeError("Invalid size for size conversion!");
            }
        case 8:
            switch(targetSize) {
                case 1:
                    return registers.size1;
                case 2:
                    return registers.size2;
                case 4:
                    return registers.size4;
                case 8:
                    return registers.size8;
                default:
                    CodeError("Invalid size for size conversion!");
            }
        default:
            CodeError("Invalid size for size conversion!");
    }
    // will never get here
    return "";
}

// converts value from given location on stack to given size and returns it as a any or specified register value if asked
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
                        var.size = targetSize;
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
                        var.size = targetSize;
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
                        var.size = targetSize;
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
                    CodeError("Invalid size for size conversion!");
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
                        var.size = targetSize;
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
                        var.size = targetSize;
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
                    CodeError("Invalid size for size conversion!");
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
                        var.size = targetSize;
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
                    CodeError("Invalid size for size conversion!");
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
                    CodeError("Invalid size for size conversion!");
            }
        default:
            CodeError("Invalid size for size conversion!");
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
            return ConvertSizeFromStack(out, var.size, outputSize, var.offset, false, &variables);
        }
        if (not valueType.sta) {
            // strictly register
            return ConvertSizeFromStack(out, var.size, outputSize, var.offset, true);
        }
        else {
            return ConvertSizeFromStack(out, var.size, outputSize, var.offset);
        }
    }

    // if given node is an actual multi thingy expression
    if (expression.nodeType == ParserValue::Node::operation) {
        uint8_t largestSize = outputSize;
        bool leftLarger = false;
        bool rightLarger = false;
        if (expression.left->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.left->value);
            if (var.size > largestSize) {
                largestSize = var.size;
                leftLarger = true;
            }
        }
        if (expression.right->nodeType == ParserValue::Node::variable) {
            auto& var = variables.find(*expression.right->value);
            if (var.size > largestSize) {
                largestSize = var.size;
                rightLarger = true;
            }
        }

        std::string left = CalculateExpression(variables, out, largestSize, *expression.left, {0, 1, 1, 1});
        std::string right;
        if (expression.secondType == ParserValue::Operation::division) {
            right = CalculateExpression(variables, out, largestSize, *expression.right, {1, 0, 1, 1}, {"%bl", "%bx", "%ebx", "%rbx"});
        }
        else {
            right = CalculateExpression(variables, out, largestSize, *expression.right, {0, 1, 1, 1});
        }


        out << "mov" << AddInstructionPostfix(largestSize) << "    " << left << ", " << AddRegisterA(largestSize) << "\n";
        if (expression.left->nodeType == ParserValue::Node::operation or leftLarger) {
            variables.free(left);
        }

        // operations really only change this part
        switch (expression.secondType) {
            case ParserValue::Operation::addition:
                out << "add" << AddInstructionPostfix(largestSize) << "    " << right << ", " << AddRegisterA(largestSize) << "\n";
                break;
            case ParserValue::Operation::subtraction:
                out << "sub" << AddInstructionPostfix(largestSize) << "    " << right << ", " << AddRegisterA(largestSize) << "\n";
                break;
            case ParserValue::Operation::multiplication:
                out << "imul" << AddInstructionPostfix(largestSize) << "   " << right << ", " << AddRegisterA(largestSize) << "\n";
                break;
            case ParserValue::Operation::division:
                out << "idiv" << AddInstructionPostfix(largestSize) << "   " << right << ", " << AddRegisterA(largestSize) << "\n";
                break;
            default:
                CodeError("Unsupported operation type!");
        }


        if (expression.right->nodeType == ParserValue::Node::operation or rightLarger) {
            variables.free(right);
        }

        std::string reg = ConvertSizeInRegister(out, largestSize, outputSize);

        if (valueType.reg) {
            return reg;
        }
        else if (valueType.sta) {
            StackVariable var;
            var.size = largestSize;
            var.amount = 1;

            std::string varAddress =  variables.pushAndStr(var);
            out << "mov" << AddInstructionPostfix(largestSize) << "    " << reg << ", " << varAddress << "\n";
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
                if (setting::OutputDodoInstructions) {
                    if (dec.expression.nodeType == ParserValue::Node::operation) {
                        out << setting::CommentCharacter << " Declaration of: " << dec.typeName << " " << dec.name << " = expression\n";
                    }
                    else if (dec.expression.nodeType == ParserValue::Node::constant) {
                        out << setting::CommentCharacter << " Declaration of: " << dec.typeName << " " << dec.name << " = constant\n";
                    }
                    else if (dec.expression.nodeType == ParserValue::Node::variable) {
                        out << setting::CommentCharacter << " Declaration of: " << dec.typeName << " " << dec.name << " = variable\n";
                    }
                    else {
                        out << setting::CommentCharacter << " Declaration of: " << dec.typeName << " " << dec.name << "\n";
                    }
                }
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
                        out << "movq    $0, " << variables.pushAndStr(var) << "\n";
                    }
                    else if (var.size > 2) {
                        if (var.offset % 4 != 0) {
                            var.offset = (var.offset / 4 + 1) * 4;
                        }
                        out << "movl    $0, " << variables.pushAndStr(var) << "\n";
                    }
                    else if (var.size > 1) {
                        if (var.offset % 2 != 0) {
                            var.offset = (var.offset / 2 + 1) * 2;
                        }
                        out << "movw    $0, " << variables.pushAndStr(var) << "\n";
                    }
                    else {
                        out << "movb    $0, " << variables.pushAndStr(var) << "\n";
                    }
                }
                break;
            }
            case FunctionInstruction::Type::returnValue:
                const ReturnInstruction& ret = *current.Variant.returnInstruction;
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


