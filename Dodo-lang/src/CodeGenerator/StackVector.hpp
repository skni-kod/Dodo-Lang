#ifndef DODO_LANG_STACK_VECTOR_HPP
#define DODO_LANG_STACK_VECTOR_HPP

#include <vector>
#include <cstdint>
#include <string>
#include "ParserVariables.hpp"

struct RegisterNames;

struct StackVariable {
    int64_t offset = 0;
    uint8_t singleSize = 4;
    uint8_t type = ParserValue::Value::signedInteger;
    bool isMutable = false;
    bool isArgument = false;
    uint64_t amount = 1;
    std::string name;
    std::string typeName;
    [[nodiscard]] std::string getAddress() const;
    [[nodiscard]] RegisterNames getAddressAsRegisterNames() const;
};

struct StackVector {
    std::vector<StackVariable> arguments;
    void addArguments(const ParserFunction& function);
    std::vector<std::vector<StackVariable>> vec;
    int64_t registerOffset = 0;
    StackVariable& find(const std::string& name);
    StackVariable& find(int64_t offset);
    StackVariable& findByOffset(const std::string& offset);
    int64_t lastOffset();
    StackVector();
    // reserves the given amount of space on the stack
    const StackVariable& push(StackVariable var);
    std::string pushAndStr(StackVariable var);
    // frees the last element from the stack
    void free_back();
    // frees the element at given offset
    void free(int64_t offset);
    void free(std::string address);
    void addLevel();
    void popLevel();
    // aligns last level to start at offset divisible by 16
    void alignTo16();
};

#endif //DODO_LANG_STACK_VECTOR_HPP
