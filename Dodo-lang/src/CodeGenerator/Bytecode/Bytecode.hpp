#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <TypeObject.hpp>
#include <vector>
#include "Options.hpp"

namespace Location {
    enum Type {
        None = 0, Unknown = 0,
        Variable = 1,
        Literal = 2, Immediate = 2, Imm = 2, imm = 2,
        String = 3,
        Label = 4,
        Call = 5,
        Argument = 6,
        Register = 7, Reg = 7, reg = 7,
        Memory = 8, Mem = 8, mem = 8,
        Stack, Sta = 9, sta = 9,
        Offset = 10, Off = 10, off = 10,
        Zeroed = 11, Zer = 11, zer = 11, Zero = 11, zero = 11,
        Operand = 12, Op = 12, op = 12
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

union OperandValue {
    uint64_t ui = 0;
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
    BytecodeOperand(Location::Type location, OperandValue value, Type::TypeEnum literalType = Type::none, uint8_t literalSize = 0);
    BytecodeOperand(Location::Type type, uint8_t operandSize, OperandValue value);
};

// represents a single bytecode instruction
// now multiple times smaller!
struct Bytecode {

    enum BytecodeInstruction {
        None,
        // variable manipulation
        //      syntax: op1 (variable) as type op2 -> op3 / result
        Cast,
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
        //      syntax: op1 (source), op2 (offset) => op3 / result
        Member,
        //      syntax: op1 => op3 / result
        Save,
        //      syntax: value at op1 + op2 * type size => op3 / result
        Index,
        //      syntax: function number at op1, first argument at op2 (optional) => op3 / result (optional)
        Function,
        //      syntax: method number at op1, first argument at op2 (optional) => op3 / result (optional)
        Method,
        //      syntax: syscall number at op1, first argument at op2 (optional) => op3 / result (optional)
        Syscall,
        //      syntax: nothing at op1, next argument at op2 (optional) => op3 / result as value to set
        Argument,
        //      syntax: value to return at op1
        Return,
        // conditional and control flow statements
        //      syntax: op1
        If,
        //      syntax: (empty)
        Else,
        //      syntax: op1
        ElseIf,
        //      syntax: (empty) - begins loop statement for jump label
        LoopLabel,
        //      syntax: op1 (initial statement)
        ForInitial,
        //      syntax: op1 (continue condition)
        ForCondition,
        //      syntax: op1 (continue statement)
        ForStatement,
        //      syntax: op1
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
    TypeMeta opTypeMeta{};
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
    int32_t assignedOffset = 0;
    bool isPointedTo = false;

    void use(uint32_t index);
};

struct BytecodeContext {
    std::vector <Bytecode> codes;
    std::vector <std::vector <VariableObject>> localVariables = { {} };
    std::vector <VariableObject> temporaries;
    // after ending a scope a level is not deleted, instead it's marked as inactive
    std::vector <uint16_t> activeLevels = { 0 };
    // if it's constant then
    bool isConstExpr = false;
    bool isMutable = false;
    // this one is not passed as it's context specific
    bool isGeneratingReference = false;

    // ALWAYS update the current() method after adding variables

    // makes a copy with empty vector
    [[nodiscard]] BytecodeContext current() const;

    // adds another context into this one
    void merge(BytecodeContext& context);

    // inserts a new local scope variable
    BytecodeOperand insertVariable(std::string* identifier, TypeObject* type, TypeMeta meta);
    // inserts a new local scope temporary variable
    BytecodeOperand insertTemporary(TypeObject* type, TypeMeta meta);
    // returns requested variable and throws and exception if there is no match for name or type is wrong
    BytecodeOperand getVariable(std::string* identifier, TypeObject* type, TypeMeta meta);
    // the same, not implemented
    BytecodeOperand getVariable(BytecodeOperand operand);

    VariableObject& getVariableObject(const std::string* identifier);
    VariableObject& getVariableObject(BytecodeOperand operand);

    void addLoopLabel();
};

inline std::vector <VariableObject> globalVariableObjects;

// export functions

BytecodeContext GenerateGlobalVariablesBytecode();
BytecodeContext GenerateFunctionBytecode(ParserFunctionMethod& function);
void OptimizeBytecode(std::vector<Bytecode>& bytecode);

// printing functions

std::ostream& operator<<(std::ostream& out, const Bytecode& code);
std::ostream& operator<<(std::ostream& out, const BytecodeOperand& op);
std::ostream& operator<<(std::ostream& out, const VariableLocation& op);

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
