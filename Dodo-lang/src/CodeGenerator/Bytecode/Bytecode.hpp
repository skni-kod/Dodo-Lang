#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "GenerateCode.hpp"

struct BytecodeOld {
    enum {
        // !!! keep this updated with ParserValue list, maybe do a macro

        // s + t -> n
        add,
        // s - t -> n
        subtract,
        // s * t -> n
        multiply,
        // s / t -> n
        divide,
        // t = s()
        callFunction,
        // &s -> n
        getAddress,
        // *s -> n
        getValue,
        // s -> (...[n])
        moveArgument,
        // return s
        returnValue,
        // push
        pushLevel,
        // pop
        popLevel,
        // jmp s
        jump,
        // <cond. jump> s, condition in number
        jumpConditionalFalse,
        // <cond. jump> s, condition in number
        jumpConditionalTrue,
        // cmp s t
        compare,
        // tp t = s
        declare,
        // t = s, 1 in number means to change value pointed to
        assign,
        // none
        none,
        // type letter + number in s
        addLabel,
        // t = s
        moveValue,
        // t = s
        addFromArgument,
        // call syscall number n, return value in source
        syscall

    };

    // l exp r
    INSERT_CONDITION_ENUM

    uint64_t code = none;
    // number will be used for things like argument number etc
    std::string source;
    std::string target;
    uint64_t number = 0;
    VariableType type;

    BytecodeOld() = default;

    explicit BytecodeOld(uint64_t code);

    BytecodeOld(uint64_t code, std::string source);

    BytecodeOld(uint64_t code, std::string source, uint64_t number);

    BytecodeOld(uint64_t code, std::string source, VariableType type);

    BytecodeOld(uint64_t code, std::string source, uint64_t number, VariableType type);

    BytecodeOld(uint64_t code, std::string source, std::string target, VariableType type);

    BytecodeOld(uint64_t code, std::string source, std::string target, uint64_t number, VariableType type);
};

std::ostream& operator<<(std::ostream& out, const BytecodeOld& code);

// bytecodes inside given function
inline std::vector<BytecodeOld> bytecodes;

// new code

#define LOCATION_SIZE 4
// bytecode location size can be smaller, as it does not have the information in which type of memory something is
#define BYTECODE_LOCATION_SIZE 4
namespace Location {
    enum Type {
        None, Variable, Literal, String, Label, Call, Temporary, Register, Heap, Stack
    };
}

union BytecodeValue {
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
    uint64_t function;
    // TODO: make this a dedicated structure
    uint64_t variable;
};

inline std::vector <ParserFunctionMethod*> converterFunctions;
inline std::vector <ParserMemberVariableParameter*> converterGlobals;

struct BytecodeOperand {
#ifdef ENUM_VARIABLES
    Location::Type location = Location::None;
    uint8_t literalSize : 4 = 0;
    Type::TypeEnum literalType = Type::none;
#else
    uint8_t type : BYTECODE_LOCATION_SIZE = Location::None;
    uint8_t literalSize : 4 = 0;
    uint8_t literalType = Type::none;
#endif
    BytecodeValue value;
    BytecodeOperand() = default;
    BytecodeOperand(Location::Type location, BytecodeValue value, Type::TypeEnum literalType = Type::none, uint8_t literalSize = 0);
};

// represents a single bytecode instruction
// now multiple times smaller!
struct Bytecode {

    enum BytecodeInstruction {
        None,
        // variable manipulation
        //      syntax: op1 (variable) = op3 / result
        Define,
        //      syntax: op1 (variable) = op3 / result
        Assign,
        //      syntax: address of op1 (variable) => op3 / result
        Address,
        //      syntax: value at op1 => op3 / result
        Dereference,
        //      syntax: op1 (member number) => op3 / result
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
        // conditional and control flow statements
        //      syntax: op1
        If,
        //      syntax: (empty)
        Else,
        //      syntax: op1
        ElseIf,
        //      syntax: op1, op2, op3
        For,
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
#ifdef ENUM_VARIABLES
    BytecodeInstruction type : 7 = BytecodeInstruction::None;
    // types of locations of operands and result
    Location::Type op1Location    : BYTECODE_LOCATION_SIZE = Location::None;
    Location::Type op2Location    : BYTECODE_LOCATION_SIZE = Location::None;
    Location::Type op3Location    : BYTECODE_LOCATION_SIZE = Location::None;
    Type::TypeEnum op1LiteralType : 2 = Type::none;
    Type::TypeEnum op2LiteralType : 2 = Type::none;
    Type::TypeEnum op3LiteralType : 2 = Type::none;
    uint8_t op1LiteralSize : 4 = 0;
    uint8_t op2LiteralSize : 4 = 0;
    uint8_t op3LiteralSize : 4 = 0;
#else
    uint8_t type : 7 = BytecodeInstruction::None;
    // types of locations of operands and result
    uint8_t op1Location    : BYTECODE_LOCATION_SIZE = Location::None;
    uint8_t op2Location    : BYTECODE_LOCATION_SIZE = Location::None;
    uint8_t op3Location    : BYTECODE_LOCATION_SIZE = Location::None;
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
    BytecodeValue op1Value, op2Value, op3Value;
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

// export functions

std::vector<Bytecode> GenerateGlobalVariablesBytecode();
std::vector<Bytecode> GenerateFunctionBytecode(ParserFunction& function);
void OptimizeBytecode(std::vector<Bytecode>& bytecode);

// printing functions

std::ostream& operator<<(std::ostream& out, const Bytecode& code);
std::ostream& operator<<(std::ostream& out, const BytecodeOperand& op);

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
