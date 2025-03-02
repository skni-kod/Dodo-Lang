#include "MemoryStructure.hpp"

#include <utility>
#include "GenerateCode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "TheGenerator.hpp"
#include "GenerateCodeInternal.hpp"
#include "Misc/Options.hpp"

namespace internal {
    ContentEntry::ContentEntry(std::string value) : value(std::move(value)) {}

    const std::string& Register::nameBySize(uint32_t size) {
        for (auto& n: sizeNamePairs) {
            if (size == n.first) {
                return n.second;
            }
        }
        CodeGeneratorError("Invalid register size in assembly output!");
        return sizeNamePairs.front().second;
    }
}

void MemoryStructure::prepareX86_86() {
    registers.resize(16);
    for (auto& n: registers) {
        n.usedForIntegers = true;
        n.content.value = "!";
    }
    registers[0].sizeNamePairs = {{1, "al"},
                                  {2, "ax"},
                                  {4, "eax"},
                                  {8, "rax"}};
    registers[1].sizeNamePairs = {{1, "bl"},
                                  {2, "bx"},
                                  {4, "ebx"},
                                  {8, "rbx"}};
    registers[2].sizeNamePairs = {{1, "cl"},
                                  {2, "cx"},
                                  {4, "ecx"},
                                  {8, "rcx"}};
    registers[3].sizeNamePairs = {{1, "dl"},
                                  {2, "dx"},
                                  {4, "edx"},
                                  {8, "rdx"}};
    registers[4].sizeNamePairs = {{1, "sil"},
                                  {2, "si"},
                                  {4, "esi"},
                                  {8, "rsi"}};
    registers[5].sizeNamePairs = {{1, "dil"},
                                  {2, "di"},
                                  {4, "edi"},
                                  {8, "rdi"}};
    registers[6].sizeNamePairs = {{1, "spl"},
                                  {2, "sp"},
                                  {4, "esp"},
                                  {8, "rsp"}};
    registers[7].sizeNamePairs = {{1, "bpl"},
                                  {2, "bp"},
                                  {4, "ebp"},
                                  {8, "rbp"}};
    registers[8].sizeNamePairs = {{1, "r8b"},
                                  {2, "r8w"},
                                  {4, "r8d"},
                                  {8, "r8"}};
    registers[9].sizeNamePairs = {{1, "r9b"},
                                  {2, "r9w"},
                                  {4, "r9d"},
                                  {8, "r9"}};
    registers[10].sizeNamePairs = {{1, "r10b"},
                                   {2, "r10w"},
                                   {4, "r10d"},
                                   {8, "r10"}};
    registers[11].sizeNamePairs = {{1, "r11b"},
                                   {2, "r11w"},
                                   {4, "r11d"},
                                   {8, "r11"}};
    registers[12].sizeNamePairs = {{1, "r12b"},
                                   {2, "r12w"},
                                   {4, "r12d"},
                                   {8, "r12"}};
    registers[13].sizeNamePairs = {{1, "r13b"},
                                   {2, "r13w"},
                                   {4, "r13d"},
                                   {8, "r13"}};
    registers[14].sizeNamePairs = {{1, "r14b"},
                                   {2, "r14w"},
                                   {4, "r14d"},
                                   {8, "r14"}};
    registers[15].sizeNamePairs = {{1, "r15b"},
                                   {2, "r15w"},
                                   {4, "r15d"},
                                   {8, "r15"}};
    registers[8].usedForStorage = true;
    registers[9].usedForStorage = true;
    registers[10].usedForStorage = true;
    registers[11].usedForStorage = true;
    registers[12].usedForStorage = true;
    registers[13].usedForStorage = true;
    registers[14].usedForStorage = true;
    registers[15].usedForStorage = true;
}

void MemoryStructure::cleanX86_86() {
    for (auto& n: registers) {
        n.content.value = "!";
    }
    stack.clear();
}

void MemoryStructure::pushLevel() {
    variableLevels.emplace_back();
}

void MemoryStructure::popLevel() {
    // find every variable of this type and get rid of it
    for (auto& current: variableLevels.back()) {
        for (int64_t n = 0; n < registers.size(); n++) {
            if (registers[n].content.value == current.identifier) {
                registers.erase(registers.begin() + n);
                n--;
            }
        }
        for (int64_t n = 0; n < stack.size(); n++) {
            if (stack[n].content.value == current.identifier) {
                stack.erase(stack.begin() + n);
                n--;
            }
        }
    }
    variableLevels.pop_back();
}


char AddInstructionPostfix(uint32_t size) {
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        switch (size) {
            case 1:
                return 'b';
            case 2:
                return 'w';
            case 4:
                return 'l';
            case 8:
                return 'q';
            default:
                CodeGeneratorError("Invalid size for postfix!");
        }
    }
    return 0;
}

std::string GetSizedRegister(uint32_t number ,uint32_t size) {
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        switch (size) {
            case 1:
                return generatorMemory.registers[number].sizeNamePairs[0].second;
            case 2:
                return generatorMemory.registers[number].sizeNamePairs[1].second;
            case 4:
                return generatorMemory.registers[number].sizeNamePairs[2].second;
            case 8:
                return generatorMemory.registers[number].sizeNamePairs[3].second;
            default:
                CodeGeneratorError("Invalid size for register name!");
        }
    }
    return {};
}

DataLocation MemoryStructure::findThing(const std::string& name) {
    if (name.front() == '$') {
        return {Operand::imm, static_cast <uint64_t>(std::stoull(name.substr(1, name.size() - 1)))};
    }
    else if (name.front() == '@') {
        int64_t offset = std::stoll(name.substr(1, name.size() - 1));
        for (auto& n : stack) {
            if (n.offset == offset) {
                return {Operand::sta, n.offset};
            }
        }
        CodeGeneratorError("No such value in stack!");
    }
    else if (name.front() == '%') {
        return {Operand::reg, static_cast <uint64_t>(std::stoull(name.substr(1, name.size() - 1)))};
    }
    for (uint64_t n = 0; n < registers.size(); n++) {
        if (registers[n].content.value == name) {
            return {Operand::reg, n};
        }
    }
    for (auto& n : stack) {
        if (n.content.value == name) {
            return {Operand::sta, n.offset};
        }
    }
    return {Operand::none, static_cast <uint64_t>(0)};
}

DataLocation::DataLocation(uint8_t type, int64_t offset, bool isAddress) : type(type), offset(offset), extractAddress(isAddress) {}

DataLocation::DataLocation(uint8_t type, uint64_t value, bool isAddress) : type(type), value(value), extractAddress(isAddress) {}

DataLocation::DataLocation(uint8_t type, ParserFunction* functionPtr, bool isAddress) : type(type), functionPtr(functionPtr), extractAddress(isAddress) {}

DataLocation::DataLocation(uint8_t type, ParserVariable* globalPtr, bool isAddress) : type(type), globalPtr(globalPtr), extractAddress(isAddress) {}

void DataLocation::print(std::ofstream& out, uint8_t size) const {
    if (Options::assemblyFlavor == Options::AssemblyFlavor::GAS and Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        switch (this->type) {
            case Operand::imm:
                out << '$' << value;
                return;
            case Operand::sta:
                out << offset << "(%rbp)";
                return;
            case Operand::reg:
                if (extractAddress) {
                    out << "(";
                    for (auto& n : generatorMemory.registers[number].sizeNamePairs) {
                        if (n.first == Options::addressSize) {
                            out << '%' << n.second;
                        }
                    }
                    out << ")";
                    return;
                }
                for (auto& n : generatorMemory.registers[number].sizeNamePairs) {
                    if (n.first == size) {
                        out << '%' << n.second;
                    }
                }
                break;
            case Operand::aadr:
                out << "$" << globalPtr->nameForOutput();
                return;
            case Operand::sla:
                out << "$LS" << number;
            return;
            default:
                CodeGeneratorError("Unimplemented: unsupported operand type in print!");
        }
    }
}

DataLocation::DataLocation(const std::string& operand) {
    type = GetOperandType(operand);
    switch (type) {
        case Operand::reg:
        case Operand::imm:
            value = std::stoull(operand.substr(1, operand.size() - 1));
            return;
        case Operand::sta:
            offset = std::stoll(operand.substr(1, operand.size() - 1));
            return;
        default:
            CodeGeneratorError("Invalid operand in data location constructor!");;
    }
    
}

bool DataLocation::operator==(const DataLocation& data) const {
    if (type == data.type and value == data.value and extractAddress == data.extractAddress) {
        return true;
    }
    return false;
}

std::string DataLocation::forMove() const {
    switch (type) {
        case Operand::reg:
            return "%" + std::to_string(number);
        case Operand::sta:
            return "@" + std::to_string(offset);
        case Operand::imm:
            return "$" + std::to_string(value);
        default:
            CodeGeneratorError("Invalid DataLocation type for string!");
    }
    return "";
}

std::string X86_64GNU_ASPrefix(uint8_t size) {
    switch (size) {
        case 1:
            return "b";
        case 2:
            return "w";
        case 4:
            return "l";
        case 8:
            return "q";
        default:
            CodeGeneratorError("Invalid prefix size!");;
    }
    return "";
}

void PrintWithSpaces(const std::string& input, std::ofstream& out) {
    for (uint16_t n = 0; n < Options::functionIndentation; n++) {
        out << " ";
    }
    out << input;
    if (input.size() < Options::instructionSpace) {
        for (uint16_t n = input.size(); n < Options::instructionSpace; n++) {
            out << " ";
        }
    }
    else {
        out << " ";
    }
}

void DEPRECATEDInstruction::outputX86_64(std::ofstream& out) const {
    if (Options::assemblyFlavor == Options::AssemblyFlavor::GAS) {
        switch (this->type) {
            case x86_64::ret:
                PrintWithSpaces("jmp", out);
                out << "." << *lastFunctionName << ".return\n";
                break;
            case x86_64::mov:
                if (Optimizations::skipUselessMoves and op1 == op2) {
                    return;
                }
                PrintWithSpaces("mov" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::movzx:
                PrintWithSpaces("movz" + X86_64GNU_ASPrefix(sizeBefore) + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeBefore);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::movsx:
                PrintWithSpaces("movs" + X86_64GNU_ASPrefix(sizeBefore) + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeBefore);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::add:
                PrintWithSpaces("add" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::sub:
                PrintWithSpaces("sub" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::mul:
                PrintWithSpaces("mul" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::imul:
                // TODO: repeat the meltdown and understand why intel reference is lying about 2 operand with imm
                PrintWithSpaces("imul" + X86_64GNU_ASPrefix(sizeAfter), out);
                if (op3.type != Operand::none) {
                    if (Optimizations::mergeThreeOperandInstruction and op1 == op2) {
                        op3.print(out, sizeAfter);
                        out << ", ";
                        op1.print(out, sizeAfter);
                    }
                    else {
                        if (sizeAfter == 1) {
                            CodeGeneratorError("Unimplemented: Three operand imul does not support 1 byte values!");
                        }
                        op3.print(out, sizeAfter);
                        out << ", ";
                        op1.print(out, sizeAfter);
                        out << ", ";
                        op2.print(out, sizeAfter);
                    }
                }
                else {
                    op2.print(out, sizeAfter);
                    out << ", ";
                    op1.print(out, sizeAfter);
                }
                out << "\n";
                break;
            case x86_64::div:
                PrintWithSpaces("div" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::idiv:
                PrintWithSpaces("idiv" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::cmp:
                PrintWithSpaces("cmp", out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::jumpLabel:
                PrintWithSpaces(".L" + std::to_string(op1.number) + ":\n", out);
                out << "\n";
                break;
            case x86_64::jmp:
                PrintWithSpaces("jmp", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::jg:
                PrintWithSpaces("jg", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::jge:
                PrintWithSpaces("jge", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::jb:
                PrintWithSpaces("jb", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::jbe:
                PrintWithSpaces("jbe", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::je:
                PrintWithSpaces("je", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::jne:
                PrintWithSpaces("jne", out);
                out << ".L" << op1.number << "\n";
                break;
            case x86_64::returnPoint:
                PrintWithSpaces("." + *lastFunctionName + ".return:", out);
            out << "\n";
                break;
            case x86_64::call:
                PrintWithSpaces("call", out);
                out << op1.functionPtr->name << "\n";
                break;
            case x86_64::lea:
                PrintWithSpaces("lea" + X86_64GNU_ASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::syscall:
                PrintWithSpaces("syscall", out);
                out << "\n";
            break;
            default:
                CodeGeneratorError("Unimplemented: Unsupported instruction code!");
        }
    }
}
