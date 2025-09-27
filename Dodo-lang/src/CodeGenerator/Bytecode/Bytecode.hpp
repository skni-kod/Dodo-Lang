#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <TypeObject.hpp>
#include "Options.hpp"

struct VariableObject;

namespace Location {
    enum Type {
        None = 0, Unknown = 0,
        Variable = 1, Var = 1, var = 1,
        Literal = 2, Immediate = 2, Imm = 2, imm = 2,
        String = 3,
        Label = 4,
        Call = 5,
        Argument = 6,
        Register = 7, Reg = 7, reg = 7,
        Memory = 8, Mem = 8, mem = 8,
        Stack, Sta = 9, sta = 9,
        Offset = 10, Off = 10, off = 10,
        ComplexOffset = 11, COff = 11, coff = 11,
        Zeroed = 12, Zer = 12, zer = 12, Zero = 12, zero = 12,
        Operand = 13, Op = 13, op = 13,
        Element = 14
    };
}

struct VariableLocation {
    enum Type {
        None, Local, Global, Temporary
    };

    #ifdef PACKED_ENUM_VARIABLES
    VariableLocation::Type type : 2 = VariableLocation::None;
    #else
    uint64_t type : 2 = VariableLocation::None;
    #endif

    uint64_t level : 14 = 0;
    uint64_t number : 48 = 0;
};

#define NO_REGISTER_IN_OFFSET 0x7F

// represents the data about how and what to offset as an operand
struct RegisterOffset {
    uint8_t  addressRegister : 7 = NO_REGISTER_IN_OFFSET;
    bool     isPrefixLabel   : 1 = false;
    uint8_t  indexRegister   : 7 = NO_REGISTER_IN_OFFSET;
    bool     isNStringLabel  : 1 = false;
    uint16_t indexScale          = 0   ;
    union {
        uint32_t labelIndex      = 0   ;
        int32_t  offset                ;
    };

};

union OperandValue {
    RegisterOffset regOff{};
    uint64_t ui;
    uint64_t u64;
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
    int64_t si;
    int64_t i64;
    int32_t i32;
    int16_t i16;
    int8_t i8;
    double fl;
    double f64;
    float f32;
    uint64_t string;
    uint64_t label;
    int64_t offset;
    uint64_t reg;
    ParserFunctionMethod* function;
    VariableLocation variable;

    OperandValue() = default;
    OperandValue(uint64_t val);
    OperandValue(VariableLocation val);

    bool operator==(const OperandValue& operand_value) const = default;
};

struct BytecodeOperand {
#ifdef PACKED_ENUM_VARIABLES
    Location::Type location : 4 = Location::None;
    uint8_t size = 0;
    Type::TypeEnum literalType : 4 = Type::none;
#else
    uint8_t type : 4 = Location::None;
    uint8_t size = 0;
    uint8_t literalType = Type::none;
#endif
    bool isTheRestZeroes : 1 = false;
    OperandValue value;
    BytecodeOperand() = default;
    BytecodeOperand(Location::Type location, OperandValue value, Type::TypeEnum literalType, uint8_t literalSize);
};

// represents a single bytecode instruction
// now multiple times smaller!
struct Bytecode {

    enum BytecodeInstruction {
        None,
        // variable manipulation

        //      syntax: op1 (variable) -> op3 / result
        Convert,
        //      syntax: op1 (variable) = op3 / result
        Define,
        //      syntax: op1 (variable) = op2 (any scalar), result in op3 / result
        AssignTo,
        //      syntax: value at op1 (address) = op2 (any scalar), result in op3 / result
        AssignAt,
        //      syntax: address of op1 (variable) => op3 / result
        Address,
        // gets value from address
        //      syntax: value at op1 => op3 / result
        Dereference,
        //      syntax: value at op1 => op3 / result
        ToReference,
        //      syntax: op1 (source), op2 (offset) => op3 / result
        Member,
        //      syntax: op1 => op3 / result
        Save,
        //      syntax: value at op1 + op2 * type size => op3 / result
        GetIndexValue,
        //      syntax: value at op1 + op2 * type size => op3 / result
        GetIndexAddress,
        //      syntax: amount in op1, first value in op2 (returns 1 zeroed element if not present) => op3 / result
        BraceListStart,
        //      syntax: (empty)
        BraceListEnd,
        //      syntax: value in op1, number in op2
        BraceListElement,
        //      syntax: function pointer at op1, first argument at op2 (optional) => op3 / result (optional)
        Function,
        //      syntax: method pointer at op1, first argument at op2 (optional) => op3 / result (optional)
        Method,
        //      syntax: syscall number at op1, first argument at op2 (optional) => op3 / result (optional)
        Syscall,
        //      syntax: nothing at op1, next argument at op2 (optional) => op3 / result as value to set
        Argument,
        //      syntax: value to return at op1
        Return,
        // conditional and control flow statements
        //      syntax: op1 (condition), op2 (false jump label)
        If,
        //      syntax: (empty)
        Else,
        //      syntax: op1
        ElseIf,
        //      syntax: (empty) - marks a loop starting to allow for lifetime extension
        LoopLabel,
        //      syntax: op1 (number)
        Label,
        //      syntax: op1 (number)
        Jump,
        //      syntax: op1 (initial statement)
        ForInitial,
        //      syntax: op1 (continue condition)
        ForCondition,
        //      syntax: op1 (continue statement)
        ForStatement,
        //      syntax: op1 (condition), op2 (false jump label)
        While,
        //      syntax: (empty)
        Do,
        //      syntax: op1
        Switch, Case,
        //      syntax: (empty)
        GoTo, Break, Continue,
        BeginScope, EndScope,
        // arithmetic and logical statements
        //      syntax: op1 <operator> op2 => op3 / result
        //      TODO: rethink conditions storage
        Power, Multiply, Divide, Modulo, Add, Subtract,
        ShiftRight, ShiftLeft,
        NAnd, BinNAnd, And, BinAnd,
        XOr, BinXOr,
        NOr, BinNOr, Or, BinOr,
        NImply, Imply, BinNImply, BinImply,
        Lesser, Greater, Equals, LesserEqual, GreaterEqual, NotEqual,
        Not, BinNot
    };

    // type of bytecode instruction
#ifdef PACKED_ENUM_VARIABLES
    BytecodeInstruction type : 7 = BytecodeInstruction::None;
    // types of locations of operands and result
    Location::Type op1Location    : 4 = Location::None;
    Location::Type op2Location    : 4 = Location::None;
    Location::Type op3Location    : 4 = Location::None;
    Type::TypeEnum op1LiteralType : 2 = Type::none;
    Type::TypeEnum op2LiteralType : 2 = Type::none;
    Type::TypeEnum op3LiteralType : 2 = Type::none;
    uint8_t op1LiteralSize : 4 = 0;
    uint8_t op2LiteralSize : 4 = 0;
    uint8_t op3LiteralSize : 4 = 0;
#else
    uint8_t type : 7 = BytecodeInstruction::None;
    // types of locations of operands and result
    uint8_t op1Location    : 4 = Location::None;
    uint8_t op2Location    : 4 = Location::None;
    uint8_t op3Location    : 4 = Location::None;
    uint8_t op1LiteralType : 2 = Type::none;
    uint8_t op2LiteralType : 2 = Type::none;
    uint8_t op3LiteralType : 2 = Type::none;
    uint8_t op1LiteralSize : 4 = 0;
    uint8_t op2LiteralSize : 4 = 0;
    uint8_t op3LiteralSize : 4 = 0;
#endif
    // probably 2 bits of space here
    // metadata of the used type
    TypeMeta    opMeta{};
    // 4 bytes used at this point
    // 4 are free for some kind of metadata
    // variable unions
    OperandValue op1Value, op2Value, op3Value;
    TypeObject* opType = nullptr;

    // methods
    // these return a reconstructed operand struct for easier use
    [[nodiscard]] BytecodeOperand op1() const;
    BytecodeOperand op1(BytecodeOperand op);
    [[nodiscard]] BytecodeOperand op2() const;
    BytecodeOperand op2(BytecodeOperand op);
    [[nodiscard]] BytecodeOperand op3() const;
    BytecodeOperand op3(BytecodeOperand op);
    [[nodiscard]] BytecodeOperand result() const;
    BytecodeOperand result(BytecodeOperand op);

    void AssignType(VariableObject& variable);
    void AssignType(TypeInfo info);
    void AssignType(TypeObject* typeObject, TypeMeta meta);
};

// represents a location in memory where a thing is or is to be stored
// TODO: maybe this can be made smaller? Might not be possible though
struct MemoryLocationBase {
    Location::Type location : 7 = Location::None;
    bool isMutable          : 1 = false;
    union {
        uint32_t number = 0;
        uint32_t size;
    };
};

// extends memory location base by offset for use with stack, not needed in initial allocation
struct MemoryLocation : MemoryLocationBase {
    int64_t offset = 0;
};

// represents a variable used in the context
struct VariableObject {
    TypeObject* type = nullptr;
    std::string* identifier = nullptr;
    TypeMeta meta{};
    // lifetime statistics
    uint32_t firstUse = 0;
    uint32_t lastUse = 0;
    uint32_t uses = 0;
    bool isPointedTo = false;

    uint8_t variableSize();
    void use(uint32_t index);
};

struct AsmOperand;
struct Register;
struct StackEntry;
struct Place;
struct AsmInstruction;
struct MemorySnapshotEntry;

struct Context {
    std::vector <Bytecode> codes;
    std::vector <std::vector <VariableObject>> localVariables = { {} };
    std::vector <VariableObject> temporaries;
    // after ending a scope a level is not deleted, instead it's marked as inactive
    std::vector <uint16_t> activeLevels = { 0 };

    std::vector <Register> registers;
    std::vector <StackEntry> stack;
    uint32_t index = 0;

    // if it's constant then
    bool isConstExpr = false;
    bool isMutable = false;
    // this one is not passed as it's context specific
    bool isGeneratingReference = false;

    // ALWAYS update the current() method after adding variables

    // makes a copy with empty vector
    [[nodiscard]] Context current() const;

    // adds another context into this one
    void merge(Context& context);

    BytecodeOperand addCodeReturningResult(Bytecode code);
    // inserts a new local scope variable
    BytecodeOperand insertVariable(std::string* identifier, TypeInfo info);
    BytecodeOperand insertTemporary(const TypeInfo& info);
    // inserts a new local scope temporary variable
    BytecodeOperand insertTemporary(TypeObject* type, TypeMeta meta);
    // returns requested variable and throws and exception if there is no match for name or type is wrong
    BytecodeOperand getVariable(std::string* identifier, TypeObject* type, TypeMeta meta);
    BytecodeOperand getVariable(std::string* identifier, TypeInfo typeInfo);
    // the same, not implemented
    BytecodeOperand getVariable(BytecodeOperand operand);

    VariableObject& getVariableObject(const std::string* identifier);
    VariableObject& getVariableObject(BytecodeOperand operand);

    Place get(AsmOperand& op);
    AsmOperand getContent(AsmOperand& op);
    AsmOperand getContentAtOffset(int32_t offset);
    AsmOperand& getContentRefAtOffset(int32_t offset);
    AsmOperand getLocation(AsmOperand& op);
    // returns a default value if not found
    AsmOperand getLocationIfExists(AsmOperand& op);
    AsmOperand getLocationStackBias(AsmOperand& op);
    AsmOperand getLocationRegisterBias(AsmOperand& op);
    AsmOperand& getContentRef(AsmOperand& op);
    // pushes variable at the back of the stack
    AsmOperand pushStack(BytecodeOperand value, int32_t amount = 1);
    AsmOperand pushStack(AsmOperand value, int32_t amount = 1);
    // returns a stack location for a temporary operation, does not set a value
    AsmOperand pushStackTemp(uint32_t size, uint32_t alignment);
    // returns a viable location for this size and alignment
    AsmOperand tempStack(uint8_t size, uint8_t alignment = 0);
    // clears the data to default state before use
    void clearProcessor();

    AsmOperand getFreeRegister(Type::TypeEnum valueType, uint16_t size) const;
    // assigns new value to a variable at assigned location and disposes of all other instances of it in memory so that only the newest version exists
    // TODO: add support for pointers to ensure there is always a value at the address pointed to
    void assignVariable(AsmOperand variable, AsmOperand source, std::vector<AsmInstruction>& instructions);
    // call AFTER the actions as it removes everything not used beyond this index
    void cleanUnusedVariables();
    std::vector<MemorySnapshotEntry> createSnapshot();
    void restoreSnapshot(std::vector<MemorySnapshotEntry>& snapshot, std::vector <AsmInstruction>& instructions);

    void addLoopLabel();
};


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
    AsmOperand(BytecodeOperand op, Context& context);
    // overrides the location to something else while preserving the size and type of operand
    AsmOperand(BytecodeOperand op, Context& context, Location::Type location, OperandValue value);
    AsmOperand(ParserFunctionMethod* functionMethod);
    [[nodiscard]] AsmOperand copyTo(Location::Type location, OperandValue value) const;
    VariableObject& object(Context& context) const;
    void print(std::ostream& out, Context& context);
    // moves the value away if it doesn't need to be there and if it doesn't exist elsewhere and returns the same value or
    // if this is the assigned place then it moves it elsewhere and returns the new place
    AsmOperand moveAwayOrGetNewLocation(Context& context, std::vector<AsmInstruction>& instructions, uint32_t index, std::vector <AsmOperand>* forbiddenLocations = nullptr, bool stackOnly = false);
    std::vector<AsmOperand> getAllLocations(Context& context);

    bool operator==(const AsmOperand& target) const;
};

struct MemorySnapshotEntry {
    AsmOperand where, what;
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



inline std::vector <VariableObject> globalVariableObjects;

// export functions

Context GenerateGlobalVariablesBytecode();
Context GenerateFunctionBytecode(ParserFunctionMethod& function);
void OptimizeBytecode(std::vector<Bytecode>& bytecode);

// printing functions

std::ostream& operator<<(std::ostream& out, const Bytecode& code);
std::ostream& operator<<(std::ostream& out, const BytecodeOperand& op);
std::ostream& operator<<(std::ostream& out, const VariableLocation& op);

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
