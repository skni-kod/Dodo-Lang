#ifndef DODO_LANG_STACK_VECTOR_HPP
#define DODO_LANG_STACK_VECTOR_HPP

#include <vector>
#include <cstdint>
#include <string>
#include "ParserVariables.hpp"

struct RegisterNames;

struct StackVariable {
    uint64_t offset = 0;
    uint8_t singleSize = 4;
    uint8_t type = ParserValue::Value::signedInteger;
    bool isMutable = false;
    uint32_t amount = 1;
    std::string name;
    std::string typeName;
    [[nodiscard]] std::string getAddress() const;
    [[nodiscard]] RegisterNames getAddressAsRegisterNames() const;
};

struct StackVector {
    
    std::vector<std::vector<StackVariable>> vec;
    uint64_t registerOffset = 0;
    StackVariable& find(const std::string& name);
    StackVariable& find(uint64_t offset);
    StackVariable& findByOffset(const std::string& offset);
    uint64_t lastOffset();
    StackVector();
    // reserves the given amount of space on the stack
    const StackVariable& push(StackVariable var);
    std::string pushAndStr(StackVariable var);
    // frees the last element from the stack
    void free_back();
    // frees the element at given offset
    void free(uint64_t offset);
    void free(std::string result);
};

#endif //DODO_LANG_STACK_VECTOR_HPP
