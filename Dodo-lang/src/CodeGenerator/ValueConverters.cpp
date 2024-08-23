#include "GenerateCode.hpp"
#include "ParserVariables.hpp"

RegisterNames::RegisterNames(std::string size1, std::string size2, std::string size4, std::string size8) :
        size1(std::move(size1)), size2(std::move(size2)), size4(std::move(size4)), size8(std::move(size8)) {}

bool RegisterNames::nonDefault() const {
    if (size1 != "%al" or size2 != "%ax" or size4 != "%eax" or size8 != "%rax") {
        return true;
    }
    return false;
}

std::string RegisterNames::registerBySize(uint16_t size) const {
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
            CodeError("Invalid singleSize in custom register function!");
    }
    return "0";
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
            CodeError("Invalid singleSize in postfix function!");
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
            CodeError("Invalid singleSize in register A function!");
    }
    return "";
}

// converts value in given register to given singleSize and if asked returns it to the source register
// pass registers as in "%reg"
std::string ConvertValueInRegister(std::ofstream& out, uint8_t originalSize, uint8_t targetSize,
                                   uint8_t inputType, uint8_t outputType, RegisterNames registers, bool returnToOriginal) {

    // first change type of the variable, nothing to do here yet, floats will require stuff
    if (inputType == ParserValue::Value::floatingPoint or outputType == ParserValue::Value::floatingPoint) {
        CodeError("Floating point conversions not yet supported!");
    }

    switch (outputType) {
        case ParserValue::Value::signedInteger:
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
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            CodeError("Invalid singleSize for singleSize conversion!");
                    }
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
            break;
        case ParserValue::Value::unsignedInteger:
            // expand values without caring about the 1 in front
            // does it by either setting a 0 to ah or by doing an and and leaving only the needed value
            switch (originalSize) {
                case 1:
                    switch(targetSize) {
                        case 1:
                            return registers.size1;
                        case 2:
                            if (registers.size1 != "%al") {
                                out << "movb    " << registers.size1 << ", %al\n";
                            }
                            out << "movb    %0, %ah\n";
                            if (returnToOriginal) {
                                out << "movw    %ax, " << registers.size2 << "\n";
                                return registers.size2;
                            }
                            return "%ax";
                        case 4:
                            if (registers.size1 != "%al") {
                                out << "movb    " << registers.size1 << ", %al\n";
                            }
                            out << "movl    $0x000000FF, %ebx\n";
                            out << "andl    %ebx, %eax\n";
                            if (returnToOriginal) {
                                out << "movl    %eax, " << registers.size4 << "\n";
                                return registers.size4;
                            }
                            return "%eax";
                        case 8:
                            if (registers.size1 != "%al") {
                                out << "movb    " << registers.size1 << ", %al\n";
                            }
                            out << "movq    $0x00000000000000FF, %rbx\n";
                            out << "andq    %rbx, %rax\n";
                            if (returnToOriginal) {
                                out << "movq    %rax, " << registers.size8 << "\n";
                                return registers.size8;
                            }
                            return "%rax";
                        default:
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            out << "movl    $0x0000FFFF, %ebx\n";
                            out << "andl    %ebx, %eax\n";
                            if (returnToOriginal) {
                                out << "movl    %eax, " << registers.size4 << "\n";
                                return registers.size4;
                            }
                            return "%eax";
                        case 8:
                            if (registers.size2 != "%ax") {
                                out << "movw    " << registers.size2 << ", %ax\n";
                            }
                            out << "movq    $0x000000000000FFFF, %rbx\n";
                            out << "andq    %rbx, %rax\n";
                            if (returnToOriginal) {
                                out << "movq    %rax, " << registers.size8 << "\n";
                                return registers.size8;
                            }
                            return "%rax";
                        default:
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            out << "movq    $0x00000000FFFFFFFF, %rbx\n";
                            out << "andq    %rbx, %rax\n";
                            if (returnToOriginal) {
                                out << "movq    %rax, " << registers.size8 << "\n";
                                return registers.size8;
                            }
                            return "%rax";
                        default:
                            CodeError("Invalid singleSize for singleSize conversion!");
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
                            CodeError("Invalid singleSize for singleSize conversion!");
                    }
                default:
                    CodeError("Invalid singleSize for singleSize conversion!");
            }
            break;
        default:
            CodeError("Unsupported type for size conversion!");
    }


    // will never get here
    return "";
}

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

    if (value.operationType == ParserValue::Value::floatingPoint) {
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

    // just an signedInteger
    return std::stoll(*value.value);
}

void CodeError(const std::string message) {
    std::cout << "ERROR! Code generation failed : " << message << "\n";
    throw CodeException();
}

// converts value from given location on stack to given singleSize and returns it as a any or specified register value if asked
// pass registers as in "%reg"
std::string ConvertSizeFromStack(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, uint64_t offset,
                                 uint8_t inputType, uint8_t outputType, bool mustBeReg, StackVector* stack,
                                 bool mustUseGivenReg, RegisterNames registers) {

    // first change type of the variable, nothing to do here yet, floats will require stuff
    if (inputType == ParserValue::Value::floatingPoint or outputType == ParserValue::Value::floatingPoint) {
        CodeError("Floating point conversions not yet supported!");
    }

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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "cbtw\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movb    -" << offset << "(%rbp), %al\n";
                        out << "movb    $0, %ah\n";
                    }
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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "movsb   -" << offset << "(%rbp), %eax\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movl    $0, %eax\n";
                        out << "movb    -" << offset << "(%rbp), %al\n";
                    }

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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "movsb   -" << offset << "(%rbp), %eax\n";
                        out << "cltq\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movq    $0, %rax\n";
                        out << "movb    -" << offset << "(%rbp), %al\n";
                    }
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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "movsw   -" << offset << "(%rbp), %eax\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movl    $0, %eax\n";
                        out << "movw    -" << offset << "(%rbp), %ax\n";
                    }
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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "movsw   -" << offset << "(%rbp), %eax\n";
                        out << "cltq\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movq    $0, %rax\n";
                        out << "movw    -" << offset << "(%rbp), %ax\n";
                    }
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
                    if (outputType == ParserValue::Value::signedInteger) {
                        out << "movl    -" << offset << "(%rbp), %eax\n";
                        out << "cltq\n";
                    }
                    else if (outputType == ParserValue::Value::unsignedInteger) {
                        out << "movq    $0, %rax\n";
                        out << "movl    -" << offset << "(%rbp), %eax\n";
                    }
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

ValueType::ValueType(uint8_t reg, uint8_t val, uint8_t sta, uint8_t hea) : reg(reg), val(val), sta(sta), hea(hea) {}
