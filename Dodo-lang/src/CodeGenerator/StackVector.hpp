#ifndef DODO_LANG_STACK_VECTOR_HPP
#define DODO_LANG_STACK_VECTOR_HPP

#include <vector>
#include <cstdint>
#include <string>

struct StackVariable {
    uint64_t offset = 0;
    uint8_t size = 4;
    uint32_t amount = 1;
    std::string name;
    std::string typeName;
};

struct StackVector {
    std::vector<std::vector<StackVariable>> vec;
    StackVariable& find(const std::string& name);
    StackVariable& find(uint64_t offset);
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
