#include "TheGenerator.hpp"

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2,
                             uint8_t type3, std::vector<uint16_t> allowed3, uint8_t type4,
                             std::vector<uint16_t> allowed4, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3), type4(type4), allowed4(allowed4), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2,
                             uint8_t type3, std::vector<uint16_t> allowed3, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), results(results) {}

OpCombination::OpCombination(uint8_t type1) : type1(type1) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2,
                             uint8_t type3, std::vector<uint16_t> allowed3, uint8_t type4,
                             std::vector<uint16_t> allowed4,
                             std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3), type4(type4), allowed4(allowed4), registerValues(registerValues), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2,
                             uint8_t type3, std::vector<uint16_t> allowed3,
                             std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2),
        type3(type3), allowed3(allowed3), registerValues(registerValues), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2,
                             std::vector<uint16_t> allowed2,
                             std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), type2(type2), allowed2(allowed2), registerValues(registerValues), results(results) {}

OpCombination::OpCombination(uint8_t type1, std::vector<uint16_t> allowed1,
                             std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results) :
        type1(type1), allowed1(allowed1), registerValues(registerValues), results(results) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize,
                                                 std::string op1, std::string op2, std::string op3, std::string op4,
                                                 std::vector<OpCombination> combinations) :
        instructionNumber(instructionNumber), instructionSize(instructionSize), op1(op1), op2(op2), op3(op3), op4(op4), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize,
                                                 std::string op1, std::string op2, std::string op3,
                                                 std::vector<OpCombination> combinations) :
        instructionNumber(instructionNumber), instructionSize(instructionSize), op1(op1), op2(op2), op3(op3), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize,
                                                 std::string op1, std::string op2,
                                                 std::vector<OpCombination> combinations) :
        instructionNumber(instructionNumber), instructionSize(instructionSize), op1(op1), op2(op2), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize,
                                                 std::string op1,
                                                 std::vector<OpCombination> combinations) :
        instructionNumber(instructionNumber), instructionSize(instructionSize), op1(op1), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize,
                                                 std::vector<OpCombination> combinations) :
        instructionNumber(instructionNumber), instructionSize(instructionSize), combinations(combinations) {}

InstructionRequirements::InstructionRequirements(uint64_t instructionNumber) : instructionNumber(instructionNumber) {}