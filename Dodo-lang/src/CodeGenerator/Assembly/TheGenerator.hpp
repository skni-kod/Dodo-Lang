#ifndef DODO_LANG_THE_GENERATOR_HPP
#define DODO_LANG_THE_GENERATOR_HPP

#include "MemoryStructure.hpp"

#define REGISTER_SIGN '%'

// these MASSIVE structs define everything there is to want to define about what an instruction needs
// thankfully this compiler doesn't need to be blazing fast

struct OpCombination {
    // here we need to define a possible combination of operands, with the inclusion of different register combinations
    // general types of operands
    uint8_t type1: 4 = Operand::none;
    uint8_t type2: 4 = Operand::none;
    uint8_t type3: 4 = Operand::none;
    uint8_t type4: 4 = Operand::none;
    // these contain allowed registers
    std::vector<uint16_t> allowed1, allowed2, allowed3, allowed4;
    // first is register, second is value to set
    std::vector<std::pair<uint64_t, uint64_t>> registerValues;
    // when type is of replace the operand value is changed, operand number is stored in the union, value in string
    std::vector <std::pair<DataLocation, std::string>> results;
    std::pair <std::string, std::string> getOperand(uint16_t number, std::string source);

    OpCombination() = default;

    OpCombination(uint8_t type1);

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1,
                  std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2,
                  std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2,
                  uint8_t type3, std::vector<uint16_t> allowed3, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2,
                  uint8_t type3, std::vector<uint16_t> allowed3,
                  std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2,
                  uint8_t type3, std::vector<uint16_t> allowed3, uint8_t type4, std::vector<uint16_t> allowed4, std::vector <std::pair<DataLocation, std::string>> results = {});

    OpCombination(uint8_t type1, std::vector<uint16_t> allowed1, uint8_t type2, std::vector<uint16_t> allowed2,
                  uint8_t type3, std::vector<uint16_t> allowed3, uint8_t type4, std::vector<uint16_t> allowed4,
                  std::vector<std::pair<uint64_t, uint64_t>> registerValues, std::vector <std::pair<DataLocation, std::string>> results = {});
};

struct InstructionRequirements {
    // we need to define:
    // - which vars or constants it needs, probably in intel operator order
    // - possible combinations of registers/memory locations, etc. that can be used for that

    // operands to be used, empty means it and further ones are unused
    std::string op1, op2, op3, op4;
    uint64_t instructionNumber {};
    uint64_t instructionSize = 1;

    // now the dreaded combinations
    std::vector<OpCombination> combinations;

    InstructionRequirements() = default;

    InstructionRequirements(uint64_t instructionNumber);

    InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize, std::vector<OpCombination> combinations);

    InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize, std::string op1, std::vector<OpCombination> combinations);

    InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize, std::string op1, std::string op2,
                            std::vector<OpCombination> combinations);

    InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize, std::string op1, std::string op2, std::string op3,
                            std::vector<OpCombination> combinations);

    InstructionRequirements(uint64_t instructionNumber, uint64_t instructionSize, std::string op1, std::string op2, std::string op3,
                            std::string op4, std::vector<OpCombination> combinations);
};

// standard structure containing all needed information about a variable
struct VariableInfo {
    VariableType value;
    DataLocation location;
    std::string identifier;

    [[nodiscard]] ParserVariable& extractGlobalVariable() const;

    VariableInfo() = default;
    VariableInfo(VariableType value, DataLocation location, std::string identifier);
    static VariableInfo FromLocation(DataLocation location);
    explicit VariableInfo(const std::string& name);

};

void GenerateInstruction(InstructionRequirements req, uint64_t index);

void InsertValue(std::string target, std::string source);

//void MoveValue(std::string source, std::string target, std::string contentToSet, uint16_t operationSize, uint64_t index);
void MoveValue(VariableInfo source, VariableInfo target, std::string contentToSet, uint64_t operationSize);

uint8_t GetOperandType(const std::string& operand);

void AssignExpressionToVariable(const std::string& exp, const std::string& var);

internal::StackEntry* AddStackVariable(std::string name);

internal::StackEntry* FindStackVariableByOffset(std::uint64_t offset);

void SetContent(DataLocation location, const std::string& content);

void FillDesignatedPlaces(uint64_t index);

void UpdateVariables();

VariableType GetVariableType(const std::string& name);

#endif //DODO_LANG_THE_GENERATOR_HPP