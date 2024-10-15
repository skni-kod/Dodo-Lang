#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "GenerateCode.hpp"

struct Bytecode {
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
        // t = s
        assign,
        // none
        none,
        // type letter + number in s
        addLabel,
        // t = s
        moveValue,
        // t = s
        addFromArgument

    };

    // l exp r
    INSERT_CONDITION_ENUM

    uint64_t code = none;
    // number will be used for things like argument number etc
    std::string source;
    std::string target;
    uint64_t number = 0;
    VariableType type;

    Bytecode() = default;

    explicit Bytecode(uint64_t code);

    Bytecode(uint64_t code, std::string source);

    Bytecode(uint64_t code, std::string source, uint64_t number);

    Bytecode(uint64_t code, std::string source, VariableType type);

    Bytecode(uint64_t code, std::string source, uint64_t number, VariableType type);

    Bytecode(uint64_t code, std::string source, std::string target, VariableType type);

    Bytecode(uint64_t code, std::string source, std::string target, uint64_t number, VariableType type);
};

std::ostream& operator<<(std::ostream& out, const Bytecode& code);

void OptimizeBytecode();

// bytecodes inside given function
inline std::vector<Bytecode> bytecodes;

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
