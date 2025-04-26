#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include <X86_64Enums.hpp>

#include "Bytecode.hpp"
#include "X86_64Enums.hpp"

// This is the scary file when all the terrifying assembly stuff roots from

struct AsmInstruction;
struct Processor;

struct AsmOperand {
    #ifdef PACKED_ENUM_VARIABLES
    Location::Type op : 4 = Location::None;
    // type of value
    Type::TypeEnum type : 2 = Type::none;
    #else
    uint8_t op : 3 = Operand::None;
    // type of value
    uint8_t type : 2 = Type::none;
    #endif
    // if set to true, then the address in the operand should be used instead of it
    bool useAddress : 1 = false;
    // if set to true, then it's an argument move that might need to have its stack location changed
    bool isArgumentMove : 1 = false;
    // size for operation in bytes
    enum LabelType {
        none, function, jump, string
    };
    union {
        uint8_t size = 0;
        LabelType labelType : 8;
    };

    // value to set in the operand
    OperandValue value = {};
    AsmOperand() = default;
    AsmOperand(Location::Type op, Type::TypeEnum type, bool useAddress, uint8_t size, OperandValue value);
    AsmOperand(Location::Type op, Type::TypeEnum type, bool useAddress, LabelType label, OperandValue value);
    AsmOperand(int32_t stackOffset);
    AsmOperand(BytecodeOperand op, BytecodeContext& context);
    // overrides the location to something else while preserving the size and type of operand
    AsmOperand(BytecodeOperand op, BytecodeContext& context, Location::Type location, OperandValue value);
    AsmOperand(ParserFunctionMethod* functionMethod);
    [[nodiscard]] AsmOperand copyTo(Location::Type location, OperandValue value) const;
    VariableObject& object(BytecodeContext& context, Processor& processor) const;
    void print(std::ostream& out, BytecodeContext& context, Processor& processor);
    bool isAtAssignedPlace(BytecodeContext& context, Processor& processor);
    // moves the value away if it doesn't need to be there and if it doesn't exist elsewhere and returns the same value or
    // if this is the assigned place then it moves it elsewhere and returns the new place
    AsmOperand moveAwayOrGetNewLocation(BytecodeContext& context, Processor& processor, std::vector<AsmInstruction>& instructions, uint32_t index, std::vector <AsmOperand>* forbiddenLocations = nullptr, bool stackOnly = false);
    std::vector<AsmOperand> getAllLocations(Processor& processor);

    bool operator==(const AsmOperand& target) const;
};

// represents a register in cpu
// all that data will probably not be used in favor of assigning possible locations for each instruction since there's little pattern to it
struct Register {
    // unknown if it will be needed
    uint8_t number = 0;

    // what the register even is

    bool isReservedRegister           : 1 = false; // should the register not be used for long term variable storage?
    bool isOverwrittenOnCall          : 1 = false; // if it is a non-reserved register, does it's value need to be saved before a call?
    bool isGeneralPurposeRegister     : 1 = false; // can the register be used as general purpose?
    bool isInstructionPointerRegister : 1 = false; // does the register contain instruction pointer?
    bool isProgramCounterRegister     : 1 = false; // does the register contain program counter?
    bool isSegmentRegister            : 1 = false; // does the register contain segment data?
    bool isFlagsRegister              : 1 = false; // does the register contain flags?

    // operand sizes accepted

    bool operandSize8   : 1 = false; // can the register accept 8 bit operands?
    bool operandSize16  : 1 = false; // can the register accept 16 bit operands?
    bool operandSize32  : 1 = false; // can the register accept 32 bit operands?
    bool operandSize64  : 1 = false; // can the register accept 64 bit operands?
    bool operandSize128 : 1 = false; // can the register accept 128 bit operands?
    bool operandSize256 : 1 = false; // can the register accept 256 bit operands?
    bool operandSize512 : 1 = false; // can the register accept 512 bit operands?

    // simd information

    bool isSIMD  : 1 = false; // can the register perform SIMD operations?
    // TODO: expand SIMD data to include things like operand sizes for types, etc., not needed for now

    // accepted data information

    uint8_t size : 8 = 0; // the full size of the register in bytes

    bool canStoreUnsignedIntegers          : 1 = false; // can the register store values of unsigned integers?
    bool canStoreSignedIntegers            : 1 = false; // can the register store values of signed integers?
    bool canStoreFloatingPointValues       : 1 = false; // can the register store values of floating point numbers?
    bool canStoreAddresses                 : 1 = false; // can the register store values of addresses?

    bool canOperateOnUnsignedIntegers      : 1 = false; // can the register operate on values of unsigned integers?
    bool canOperateOnSignedIntegers        : 1 = false; // can the register operate on values of signed integers?
    bool canOperateOnFloatingPointValues   : 1 = false; // can the register operate on values of floating point numbers?
    bool canOperateOnAddresses             : 1 = false; // can the register operate on values of addresses?

    bool canBeUsedAsAddressSource          : 1 = false; // can an address in the register be used directly in instructions?

    bool canUseHalfPrecisionFloats         : 1 = false; // can the register perform operations on half precision floats?
    bool canUseSinglePrecisionFloats       : 1 = false; // can the register perform operations on single precision floats?
    bool canUseDoublePrecisionFloats       : 1 = false; // can the register perform operations on double precision floats?

    AsmOperand content = {}; // the content that is assumed to be present in the register

    bool canBeLongStored(const VariableObject& variable) const;
    bool canBeStored(const VariableObject& variable) const;
};

// represents a single entry on the stack
struct StackEntry {
    AsmOperand content;
    // offset from base pointer, beware that value must be AT least equal in negative to size since it's negative indexing,
    // alternatively it can be positive value if it's an argument for a function call
    int32_t offset = 0;
    uint32_t size = 0;
};

struct Place {
    union {
        Register* reg = nullptr;
        StackEntry* sta;
    };
    Location::Type where : 8 = Location::None;
    Place() = default;
    Place(Register* reg, Location::Type where);
    Place(StackEntry* sta, Location::Type where);
};

// represents a virtual processor
struct Processor {
    std::vector <Register> registers;
    std::vector <StackEntry> stack;
    uint32_t index = 0;

    // clears the data to default state before use
    void clear();

    Place get(AsmOperand& op);
    AsmOperand getContent(AsmOperand& op, BytecodeContext& context);
    AsmOperand getContentAtOffset(int32_t offset);
    AsmOperand& getContentRefAtOffset(int32_t offset);
    AsmOperand getLocation(AsmOperand& op);
    // returns a default value if not found
    AsmOperand getLocationIfExists(AsmOperand& op);
    AsmOperand getLocationStackBias(AsmOperand& op);
    AsmOperand getLocationRegisterBias(AsmOperand& op);
    AsmOperand& getContentRef(AsmOperand& op);
    // pushes variable at the back of the stack
    AsmOperand pushStack(BytecodeOperand value, BytecodeContext& context);
    AsmOperand pushStack(AsmOperand value, BytecodeContext& context);
    // returns a viable location for this size and alignment
    AsmOperand tempStack(uint8_t size, uint8_t alignment = 0);

    AsmOperand getFreeRegister(Type::TypeEnum valueType, uint16_t size) const;
    // assigns new value to a variable at assigned location and disposes of all other instances of it in memory so that only the newest version exists
    // TODO: add support for pointers to ensure there is always a value at the address pointed to
    void assignVariable(AsmOperand variable, AsmOperand source, BytecodeContext& context, std::vector<AsmInstruction>& instructions);
    // call AFTER the actions as it removes everything not used beyond this index
    void cleanUnusedVariables(BytecodeContext& context, uint32_t index);
};


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

// functions

void ExecuteInstruction(BytecodeContext& context, Processor& processor, AsmInstructionInfo& instruction, std::vector<AsmInstruction>& instructions, uint32_t index);
void AddConversionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc, std::vector<AsmInstruction>& instructions, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters = nullptr);

#endif //ASSEMBLY_HPP
