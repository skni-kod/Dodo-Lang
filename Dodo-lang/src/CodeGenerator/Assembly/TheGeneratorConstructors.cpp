#include "TheGenerator.hpp"

OpCombination::OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2,
                             uint8_t type3, std::vector <uint16_t> allowed3, uint8_t type4, std::vector <uint16_t> allowed4) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3), type4(type4), allowed4(allowed4) {}

OpCombination::OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2,
                             uint8_t type3, std::vector <uint16_t> allowed3) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3) {}

OpCombination::OpCombination(uint8_t type1, std::vector <uint16_t> allowed1, uint8_t type2, std::vector <uint16_t> allowed2) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2) {}

OpCombination::OpCombination(uint8_t type1, std::vector <uint16_t> allowed1) :
        type1(type1), allowed1(allowed1) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber,
                                                 std::string op1, std::string op2, std::string op3, std::string op4,
                                                 std::vector <OpCombination> combinations) :
        op1(op1), op2(op2), op3(op3), op4(op4), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber,
                                                 std::string op1, std::string op2, std::string op3,
                                                 std::vector <OpCombination> combinations) :
        op1(op1), op2(op2), op3(op3), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber,
                                                 std::string op1, std::string op2,
                                                 std::vector <OpCombination> combinations) :
        op1(op1), op2(op2), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber,
                                                 std::string op1,
                                                 std::vector <OpCombination> combinations) :
        op1(op1), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber,
                                                 std::vector <OpCombination> combinations) :
        combinations(combinations) {}