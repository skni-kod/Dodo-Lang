#ifndef TYPE_OBJECT_HPP
#define TYPE_OBJECT_HPP
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct LexerToken;

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

struct TypeInfo {
    uint8_t type = Type::none;
    uint8_t subType = Subtype::none;
    uint8_t pointerLevel = 0;
    uint8_t isMutable = false;
};

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

struct ParserTypeObjectMember {
    std::string memberName;
    TypeObject* memberType = nullptr;
    std::unique_ptr<std::string> memberTypeName = nullptr;
    uint32_t memberOffset = 0;
    TypeInfo type;
    TypeObjectValue memberDefaultValue;
};

std::ostream& operator<<(std::ostream& out, const ParserTypeObjectMember& member);


struct ParserValueTypeObject {
    std::string typeName;
    TypeInfo type;
};

struct ParserArgument {
    std::string name;
    ParserValueTypeObject type;
    TypeObjectValue defaultValue;
};

namespace ParserOperation {
    enum Type {
        // an operation of 2 operators, resolved by the bytecode generator with possible overloads
        // lvalue and rvalue of the operation
        Operator,
        // a group operation, resolved by the bytecode generator with possible overloads, used by (), [], {}
        // next for members, etc. and left for value calculated
        Group,
        // represents a member of given variable
        // next value for next member
        Member,
        // call a function
        // next value for next member, rvalue for parameter, pointer for name
        Call,
        // syscall
        // next value for next member, rvalue for parameter, code for number
        Syscall,
        // call parameter
        // parameter value for value, rvalue for next parameter
        Argument,
        // constant literal
        // pointer to token with value
        Constant,
        // variable
        // pointer to identifier, next value for its members, etc.
        Variable
    };
}

// defines a single operation in lvalue or rvalue expression
// and sadly due to the damn pointer it takes up 16 bytes
struct ParserTreeValue {
    ParserOperation::Type operation = ParserOperation::Operator;
    uint8_t isLValued = false;
    union {
        uint16_t left = 0;
        uint16_t next;

    };
    union {
        uint16_t right = 0;
        uint16_t value;
        uint16_t argument;
    };
    union {
        std::string* identifier = nullptr;
        const LexerToken* literal;
        uint64_t code;
    };
};

namespace Instruction {
    // TODO: think about how to classify instructions
    enum Type {
        None, Assign, Declare, Return, Call, If, Else, ElseIf, Switch, While, For, Do, Break, Continue
    };
}

struct ParserFunctionMethodInstructionObject {
    uint8_t type = Instruction::None;
    uint16_t rValueIndex = 0;
    // lValue index might be not needed
    uint16_t lValueIndex = 0;
    std::vector <ParserTreeValue> valueArray;
};

struct ParserFunctionMethodObject {
    std::vector <ParserFunctionMethodInstructionObject> instructions;
    std::string name;
    std::vector<ParserArgument> arguments;
    ParserValueTypeObject returnType;
};

// TODO: figure out a way to change this to temp type
struct TypeObject {
    std::string typeName;
    uint64_t isPrimitive   : 1 = false;
    uint64_t primitiveType : 2 = Type::none;
    uint64_t typeAlignment : 4 = 0;
    uint64_t typeSize : 57 = 0;
    std::vector <ParserTypeObjectMember> members;
    std::vector <ParserFunctionMethodObject> methods;
};

inline std::unordered_map <std::string, ParserFunctionMethodObject> unpreparedFunctions;

std::ostream& operator<<(std::ostream& out, const TypeObject& type);


#endif //TYPE_OBJECT_HPP
