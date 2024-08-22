#include "GenerateCode.hpp"

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
std::string ConvertSizeInRegister(std::ofstream& out, uint8_t originalSize, uint8_t targetSize, RegisterNames registers, bool returnToOriginal) {
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
    // will never get here
    return "";
}