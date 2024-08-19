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
    std::vector<StackVariable> vec;
    StackVariable& find(const std::string& name);
    uint64_t lastOffset();
};

#endif //DODO_LANG_STACK_VECTOR_HPP
