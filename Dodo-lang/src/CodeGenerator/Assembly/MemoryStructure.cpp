//
// Created by fifthzoner on 19/09/24.
//
#include "MemoryStructure.hpp"
#include "GenerateCode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "LinearAnalysis.hpp"
#include "TheGenerator.hpp"
#include "CodeGeneratorOld/GenerateCode.hpp"

namespace internal {
    ContentEntry::ContentEntry(std::string value) : value(value) {}

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
    if (Options::targetArchitecture == "X86_64") {
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
    if (Options::targetArchitecture == "X86_64") {
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

DataLocation dataLocationDummy;

DataLocation GetConvertedValue(const std::string& name) {
    // the value requested does not exist!
    // take the main type value for this value and convert it to requested type

    // first find the main variable
    std::string searched = name.substr(3, name.size() - 3);
    DataLocation stat;
    // this is the location where we want the variable if possible
    VariableStatistics* lifetime = nullptr;
    stat.type = Operand::none;
    VariableType sourceType, targetType;
    targetType.size = std::stoull(name.substr(1, 1));
    targetType.type = GetOperandType(name);
    for (auto& n : variableLifetimes.map) {
        if (n.first.ends_with(searched) and n.second.isMainValue) {
            stat = generatorMemory.findThing(n.first);
            sourceType.size = std::stoull(n.first.substr(1, 1));
            sourceType.type = GetOperandType(n.first);
        }
    }
    if (stat.type == Operand::none) {
        CodeGeneratorError("Base variable ending with: " + searched + " does not exist!");
    }

    for (auto& n : variableLifetimes.map) {
        if (n.first == name) {
            lifetime = &n.second;
        }
    }
    DataLocation where;
    if (lifetime->assignStatus == VariableStatistics::reg) {
        where = {Operand::reg, uint64_t(lifetime->assigned)};
    }
    else {
        where = {Operand::sta, AddStackVariable(name)->offset};
    }

    // now the base if found, convert it and return its location
    if (Options::targetArchitecture == "X86_64") {
        switch (sourceType.type) {
            case ParserType::Type::signedInteger:
            case ParserType::Type::unsignedInteger:
                switch (targetType.type) {
                    case ParserType::Type::signedInteger:
                    case ParserType::Type::unsignedInteger:
                        if (sourceType.size > targetType.size) {
                            if (stat.type == Operand::sta) {
                                return {Operand::sta, stat.offset + sourceType.size - targetType.size};
                            }
                            else {
                                return stat;
                            }
                        }
                        else {
                            // in this case get the extended value
                            if (where.type == Operand::reg) {
                                Instruction ins;
                                if (targetType.type == ParserType::unsignedInteger) {
                                    ins.type = x86_64::movzx;
                                }
                                else if (targetType.type == ParserType::signedInteger) {
                                    ins.type = x86_64::movsx;
                                }
                                ins.sizeBefore = sourceType.size;
                                ins.sizeAfter = targetType.size;
                                ins.postfix1 = AddInstructionPostfix(sourceType.size);
                                ins.postfix2 = AddInstructionPostfix(targetType.size);
                                ins.op1 = where;
                                if (sourceType.type == Operand::sta) {
                                    ins.op2 = {Operand::sta, stat.offset};
                                }
                                else {
                                    ins.op2 = {Operand::reg, uint64_t(stat.number)};
                                }
                                finalInstructions.push_back(ins);
                                return where;

                            }
                            else if (where.type == Operand::sta) {
                                // TODO: make a free register function and then do a movsx/movzx to rax
                                if (sourceType.type == Operand::reg) {
                                    // for now move a 0 to that location and then the value
                                    Instruction ins;
                                    ins.type = x86_64::mov;
                                    ins.sizeAfter = ins.sizeBefore = where.size;
                                    ins.postfix1 = AddInstructionPostfix(where.size);
                                    ins.op1 = {Operand::sta, stat.offset};
                                    ins.op2 = {Operand::imm, uint64_t(0)};
                                    finalInstructions.push_back(ins);
                                    // now move the value
                                    ins.sizeAfter = ins.sizeBefore = sourceType.size;
                                    ins.postfix1 = AddInstructionPostfix(sourceType.size);
                                    ins.op2 = {Operand::reg, uint64_t(stat.number)};
                                }
                                else {
                                    // move 0 to given register
                                    MoveValue("$0", "%" + GetSizedRegister(0, targetType.size), "$0");
                                    Instruction ins;
                                    ins.type = x86_64::mov;
                                    ins.sizeAfter = ins.sizeBefore = sourceType.size;
                                    ins.postfix1 = AddInstructionPostfix(sourceType.size);
                                    ins.op1 = {Operand::reg, uint64_t(0)};
                                    ins.op2 = {Operand::sta, stat.offset};
                                    finalInstructions.push_back(ins);
                                    // now move the value
                                    ins.op1 = {Operand::sta, where.offset};
                                    ins.op2 = {Operand::reg, uint64_t(0)};
                                    ins.sizeAfter = ins.sizeBefore = targetType.size;
                                    ins.postfix1 = AddInstructionPostfix(targetType.size);
                                    finalInstructions.push_back(ins);
                                }

                            }
                        }
                        break;
                }
                break;
        }
    }
    CodeGeneratorError("Reached end of conversion function!");
    return dataLocationDummy;
}

DataLocation MemoryStructure::findThing(std::string name) {
    if (name.front() == '$') {
        return {Operand::imm, uint64_t(std::stoull(name.substr(1, name.size() - 1)))};
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
        return {Operand::reg, uint64_t(std::stoull(name.substr(1, name.size() - 1)))};
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
    return {Operand::none, uint64_t(0)};
}

/*
std::ostream& operator<<(std::ostream& out, const DataLocation& data) {
    if (options::targetArchitecture == "X86_64") {
        switch (data.type) {
            case DataLocation::reg:
                return out << '%' << generatorMemory.registers[data.number].nameBySize(data.size);
            case DataLocation::sta:
                return out << data.offset << "(%rbp)";
            case DataLocation::hea:
                CodeGeneratorError("Heap memory not supported!");
            case DataLocation::las:
                CodeGeneratorError("Text constants not yet supported!");
            case DataLocation::lal:
                return out << ".LC" << data.label.number;
            case DataLocation::laf:
                CodeGeneratorError("Function labels not yet supported in output!");
            case DataLocation::val:
                return out << data.value;
        }
    }
    CodeGeneratorError("Unsupported target for assembly output!");
    return out;
}
*/

DataLocation::DataLocation(uint8_t type, uint32_t number, uint32_t size) : type(type), number(number), size(size) {}

DataLocation::DataLocation(uint8_t type, int64_t offset) : type(type), offset(offset) {}

DataLocation::DataLocation(uint8_t type, uint64_t value) : type(type), value(value) {}

DataLocation::DataLocation(uint8_t type, ParserFunction* functionPtr) : type(type), functionPtr(functionPtr) {}

void DataLocation::print(std::ofstream& out, uint8_t size) {
    if (Options::assemblyFlavor == Options::AssemblyFlavor::GNU_AS and Options::targetArchitecture == "X86_64") {
        switch (this->type) {
            case Operand::imm:
                out << '$' << value;
                return;
            case Operand::sta:
                out << offset << "(%rbp)";
                return;
            case Operand::reg:
                for (auto& n : generatorMemory.registers[number].sizeNamePairs) {
                    if (n.first == size) {
                        out << '%' << n.second;
                        return;
                    }
                }
                CodeGeneratorError("Could not find valid register!");
                return;
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
    }
    CodeGeneratorError("Invalid operand in data location constructor!");
}

std::string X86_64GNUASPrefix(uint8_t size) {
    switch (size) {
        case 1:
            return "b";
        case 2:
            return "w";
        case 4:
            return "l";
        case 8:
            return "q";
    }
    CodeGeneratorError("Invalid prefix size!");
    return "";
}

void PrintWithSpaces(std::string input, std::ofstream& out) {
    out << input;

    if (input.size() < Options::spaceOnLeft) {
        for (uint16_t n = input.size(); n < Options::spaceOnLeft; n++) {
            out << " ";
        }
    }
    else {
        out << " ";
    }
}

void Instruction::outputX86_64(std::ofstream& out) {
    if (Options::assemblyFlavor == Options::AssemblyFlavor::GNU_AS) {
        switch (this->type) {
            case x86_64::ret:
                out << "popq    %rbp\n";
                out << "ret\n";
                break;
            case x86_64::mov:
                PrintWithSpaces("mov" + X86_64GNUASPrefix(sizeAfter), out);
                op2.print(out, sizeAfter);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::movzx:
                PrintWithSpaces("movz" + X86_64GNUASPrefix(sizeBefore) + X86_64GNUASPrefix(sizeAfter), out);
                op2.print(out, sizeBefore);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
            case x86_64::movsx:
                PrintWithSpaces("movs" + X86_64GNUASPrefix(sizeBefore) + X86_64GNUASPrefix(sizeAfter), out);
                op2.print(out, sizeBefore);
                out << ", ";
                op1.print(out, sizeAfter);
                out << "\n";
                break;
        }
    }
}
