#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "GenerateCode.hpp"

struct SourceTargetTypes {
    VariableType source, target;
};

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
        // s -> (...[n])
        moveArgument,
        // s()
        prepareArguments,
        // return s
        returnValue,
        // push
        pushLevel,
        // pop
        popLevel,
        // jmp s
        jump,
        // <cond. jump> s
        jumpConditional,
        // cmp s t
        compare,
        // tp t = s
        declare,
        // t = s
        assign,
        // none
        none,
        // s -> other type
        convert

    };
    enum Condition {
        // TODO: add conditional enum
    };
    uint64_t code = none;
    // number will be used for things like argument number etc
    std::string source;
    std::string target;
    union {
        uint64_t number = 0;
        SourceTargetTypes types;
        const ParserType* typePointer;
    };
    Bytecode() = default;
    Bytecode(Bytecode& b) = default;
    Bytecode(Bytecode&& b) = default;
    explicit Bytecode(uint64_t code);
    Bytecode(uint64_t code, std::string source);
    Bytecode(uint64_t code, std::string source, std::string target, const ParserType* typePointer);
    Bytecode(uint64_t code, std::string source, std::string target);
    Bytecode(uint64_t code, std::string source, std::string target, uint64_t number);
    Bytecode(uint64_t code, std::string source, uint64_t number);
    Bytecode(uint64_t code, std::string source, VariableType sourceType, VariableType targetType);
    Bytecode(uint64_t code, std::string source, VariableType bothType);
    Bytecode(uint64_t code, std::string source, std::string target, VariableType bothType);
    Bytecode(uint64_t code, std::string source, std::string target, VariableType sourceType, VariableType targetType);
};

std::ostream& operator<<(std::ostream& out, const Bytecode& code);

// bytecodes inside given function
inline std::vector <Bytecode> bytecodes;

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
