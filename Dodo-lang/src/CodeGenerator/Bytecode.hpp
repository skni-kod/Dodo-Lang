#ifndef DODO_LANG_BYTECODE_HPP
#define DODO_LANG_BYTECODE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "GenerateCode.hpp"

struct Bytecode {
    enum {
        // !!! keep this updated with ParserValue list, maybe do a macro
        add, subtract, multiply, divide, callFunction, moveArgument, prepareArguments, returnValue, pushLevel,
        popLevel, jump, jumpConditional, compare, declare, assign, none, convert
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
        struct {
            uint16_t sourceType, sourceSize, targetType, targetSize;
        };
    };
    Bytecode() = default;
    Bytecode(Bytecode& b) = default;
    Bytecode(Bytecode&& b) = default;
    Bytecode(uint64_t code, std::string source);
    Bytecode(uint64_t code, std::string source, std::string target);
    Bytecode(uint64_t code, std::string source, std::string target, uint64_t number);
    Bytecode(uint64_t code, std::string source, uint64_t number);
    Bytecode(uint64_t code, std::string source, uint16_t sourceType, uint16_t sourceSize, uint16_t targetType, uint16_t targetSize);
    Bytecode(uint64_t code, std::string source, uint16_t sourceType, uint16_t sourceSize, VariableType targetType);
    Bytecode(uint64_t code, std::string source, VariableType sourceType, VariableType targetType);
    Bytecode(uint64_t code, std::string source, std::string target, VariableType sourceType);
};

std::ostream& operator<<(std::ostream& out, const Bytecode& code);

// bytecodes inside given function
inline std::vector <Bytecode> bytecodes;

#endif //DODO_LANG_GENERAL_BYTECODE_HPP
