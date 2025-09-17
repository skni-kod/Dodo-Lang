#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include <cstdint>
#include <stack>

#include "Bytecode.hpp"
#include "TypeObject.hpp"

// This is the scary file when all the terrifying assembly stuff roots from




struct AsmInstruction {
    uint16_t code = 0;
    AsmOperand op1, op2, op3, op4;
};


// structure responsible for defining everything an instruction needs and results in
// TODO: change to a faster and smaller array size, maybe stack allocation?
struct RegisterRange {
    uint16_t first = 0;
    uint16_t last = 0;
    bool canBeOp1 = false;
    bool canBeOp2 = false;
    bool canBeOp3 = false;
    bool canBeOp4 = false;
};

struct AsmInstructionResultInput {
    // if it's a fixed location modify it with result
    // however if it's not modify the specified operand location in range 1-4
    bool isFixedLocation : 1 = false;
    bool isInput : 1 = false;
    union {
        // if size is 0 it will be the same as in operation
        AsmOperand fixedLocation = {};
        struct {
            uint8_t operandNumber;
        };
    };
    AsmOperand value;

    AsmInstructionResultInput() = default;
    AsmInstructionResultInput(bool isInput, AsmOperand location, AsmOperand value);
    AsmInstructionResultInput(bool isInput, uint8_t operandNumber, AsmOperand value);
};

struct AsmOpDefinition {
    Location::Type opType : 8 = Location::None;
    uint8_t sizeMin = 0;
    uint8_t sizeMax = 0;
    bool isInput  : 1 = false;
    bool isOutput : 1 = false;
};

struct AsmInstructionVariant {

    // instruction code of the given variant
    // some instructions have
    uint16_t code = 0;

    // minimum version of the architecture for this variant
    Options::ArchitectureVersion minimumVersion : 8 = Options::None;

    AsmOpDefinition op1 = {};
    AsmOpDefinition op2 = {};
    AsmOpDefinition op3 = {};
    AsmOpDefinition op4 = {};

    // ranges of registers allowed for the operation
    std::vector <RegisterRange> allowedRegisters = {};
    // results and inputs of the given variant, inputs will be checked to see if it's possible to use it from them
    std::vector <AsmInstructionResultInput> resultsAndInputs = {};

    AsmInstructionVariant() = default;
    AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        std::vector <AsmInstructionResultInput> resultsAndInputs);
    AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1,
        std::vector <RegisterRange> allowedRegisters, std::vector <AsmInstructionResultInput> resultsAndInputs);
    AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2,
        std::vector <RegisterRange> allowedRegisters, std::vector <AsmInstructionResultInput> resultsAndInputs);
    AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2, AsmOpDefinition op3,
        std::vector <RegisterRange> allowedRegisters, std::vector <AsmInstructionResultInput> resultsAndInputs);
    AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2, AsmOpDefinition op3, AsmOpDefinition op4,
        std::vector <RegisterRange> allowedRegisters, std::vector <AsmInstructionResultInput> resultsAndInputs);
};

struct AsmInstructionInfo {
    std::vector <AsmInstructionVariant> variants;
    AsmOperand source1 = {}; // source 1 for operation
    AsmOperand source2 = {}; // source 2 for operation
    AsmOperand source3 = {}; // source 3 for operation
    AsmOperand source4 = {}; // source 4 for operation
    AsmOperand destination = {}; // destination for operation result
    AsmInstructionVariant* selected = nullptr;
};

struct MoveInfo {
    AsmOperand source, target;
};

inline std::stack <std::vector <MemorySnapshotEntry>> memorySnapshots;

// functions

void ExecuteInstruction(Context& context, AsmInstructionInfo& instruction, std::vector<AsmInstruction>& instructions, uint32_t index);
void AddConversionsToMove(MoveInfo& move, Context& context, std::vector<AsmInstruction>& instructions, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters = nullptr, bool setContent = true);

#endif //ASSEMBLY_HPP
