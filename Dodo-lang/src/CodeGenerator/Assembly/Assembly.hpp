#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include <X86_64Enums.hpp>

#include "Bytecode.hpp"
#include "X86_64Enums.hpp"

// This is the scary file when all the terrifying assembly stuff roots from

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

    BytecodeOperand content = {}; // the content that is assumed to be present in the register

    bool canBeLongStored(const VariableObject& variable) const;
    bool canBeStored(const VariableObject& variable) const;
};

// represents a single entry on the stack
struct StackEntry {
    BytecodeOperand content;
    // offset from base pointer, beware that value must be AT least equal in negative to size since it's negative indexing,
    // alternatively it can be positive value if it's an argument for a function call
    int32_t offset = 0;
    uint32_t size = 0;
};

// represents a virtual processor
struct Processor {
    std::vector <Register> registers;
    std::vector <StackEntry> stack;

    // clears the data to default state before use
    void clear();
};


struct AsmInstruction {
    // TODO: move to union when other targets are added
    x86_64::InstructionCode code = x86_64::none;

    #ifdef PACKED_ENUM_VARIABLES
    Location::Type op1Location : 4 = Location::None;
    Location::Type op2Location : 4 = Location::None;
    Location::Type op3Location : 4 = Location::None;
    Location::Type op4Location : 4 = Location::None;
    #else
    uint8_t op1Location : 4 = Location::None;
    uint8_t op2Location : 4 = Location::None;
    uint8_t op3Location : 4 = Location::None;
    uint8_t op4Location : 4 = Location::None;
    #endif
    uint8_t op1Size : 4 = 0;
    uint8_t op2Size : 4 = 0;
    uint8_t op3Size : 4 = 0;
    uint8_t op4Size : 4 = 0;
    OperandValue op1Value = {}, op2Value = {}, op3Value = {}, op4Value = {};

};


#endif //ASSEMBLY_HPP
