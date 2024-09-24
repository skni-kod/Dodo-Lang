#ifndef DODO_LANG_THE_GENERATOR_HPP
#define DODO_LANG_THE_GENERATOR_HPP

#include "MemoryStructure.hpp"

// these MASSIVE structs define everything there is to want to define about what an instruction needs
// thankfully this compiler doesn't need to be blazing fast

struct OpCombination {
    // here we need to define a possible combination of operands, with the inclusion of different register combinations
    enum Type {
        none, reg, sta, imm, adr, // add other types here if needed
    };
    // general types of operands
    uint8_t type1:4 = Type::none;
    uint8_t type2:4 = Type::none;
    uint8_t type3:4 = Type::none;
    uint8_t type4:4 = Type::none;
    // these contain allowed registers
    std::vector <uint16_t> allowed1, allowed2, allowed3, allowed4;
    OpCombination() = default;
    OpCombination(uint8_t type1, std::vector <uint16_t> allowed1);
    OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2);
    OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2,
                  uint8_t type3, std::vector <uint16_t> allowed3);
    OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2,
                  uint8_t type3, std::vector <uint16_t> allowed3, uint8_t type4, std::vector <uint16_t> allowed4);
};

struct InstructionRequirements {
    // we need to define:
    // - which vars or constants it needs, probably in intel operator order
    // - possible combinations of registers/memory locations, etc. that can be used for that

    // operands to be used, empty means it and further ones are unused
    std::string op1, op2, op3, op4;
    uint64_t instructionNumber;

    // now the dreaded combinations
    std::vector <OpCombination> combinations;

    InstructionRequirements() = default;
    InstructionRequirements(uint64_t instructionNumber, std::vector <OpCombination> combinations);
    InstructionRequirements(uint64_t instructionNumber, std::string op1, std::vector <OpCombination> combinations);
    InstructionRequirements(uint64_t instructionNumber, std::string op1, std::string op2, std::vector <OpCombination> combinations);
    InstructionRequirements(uint64_t instructionNumber, std::string op1, std::string op2, std::string op3, std::vector <OpCombination> combinations);
    InstructionRequirements(uint64_t instructionNumber, std::string op1, std::string op2, std::string op3, std::string op4, std::vector <OpCombination> combinations);
};

void GenerateInstruction(InstructionRequirements req);

#endif //DODO_LANG_THE_GENERATOR_HPP