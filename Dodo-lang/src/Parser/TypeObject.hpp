#ifndef TYPE_OBJECT_HPP
#define TYPE_OBJECT_HPP
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define INSERT_TYPE_ENUM \
enum Type { \
unsignedInteger, signedInteger, floatingPoint, none \
}; \

#define INSERT_SUBTYPE_ENUM \
enum Subtype { \
value, pointer, none, reference \
};

#define INSERT_CONDITION_ENUM \
enum Condition { \
equals, notEquals, greater, greaterEqual, lesser, lesserEqual \
};

namespace Type {
    INSERT_TYPE_ENUM
}


// TODO: figure out why it's named that
namespace Value {
    INSERT_TYPE_ENUM
}

namespace Subtype {
    INSERT_SUBTYPE_ENUM
}

namespace Primitive {
    INSERT_TYPE_ENUM
}


struct TypeObject;

// only applies to non-primitive types
// primitives always default to 0 of their type
struct TypeObjectValue {
    // TODO: expand this, probably an unique_ptr to some structure with function call data, etc
    enum DefaultValueType {
        defaultConstructor, numeric
    };
    uint8_t type = defaultConstructor;
    uint8_t isMemberValid = false;
    union {
        uint64_t numericValue = 0;
        uint64_t _unsigned;
        uint64_t _signed;
        uint64_t _float;
    };
};

struct TypeObjectMember {
    std::string memberName;
    TypeObject* memberType = nullptr;
    std::unique_ptr<std::string> memberTypeName = nullptr;
    uint64_t memberOffset = 0;
    TypeObjectValue memberDefaultValue;
};

std::ostream& operator<<(std::ostream& out, const TypeObjectMember& member);

struct TypeObject {
    std::string typeName;
    uint64_t isPrimitive   : 1 = false;
    uint64_t primitiveType : 2 = Primitive::Type::none;
    uint64_t typeSize : 61 = 1;
    std::vector <TypeObjectMember> members;
    // will hold information about operator, constructor, destructor overrides and methods
    //std::vector <SoMeThInG> methods;
};

std::ostream& operator<<(std::ostream& out, const TypeObject& type);

#endif //TYPE_OBJECT_HPP
