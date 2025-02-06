#ifndef TYPE_OBJECT_HPP
#define TYPE_OBJECT_HPP
#include <cstdint>

#include "ParserVariables.hpp"

struct TypeObject;

// represents a type entry
struct VariableObject {
    uint64_t subtype     : 2  = Subtype::none;
    uint64_t isPrimitive : 1  = true;
    uint64_t amount      : 61 = 1;
    union {
        TypeObject* objectType = nullptr;
        struct {
            uint8_t primitiveType;
            uint8_t primitiveSize;
        };
    };
    std::string name;
};

struct TypeObject {
    std::vector <VariableObject> members;
    std::string typeName;
};

#endif //TYPE_OBJECT_HPP
