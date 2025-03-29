#ifndef TYPE_OBJECT_HPP
#define TYPE_OBJECT_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Options.hpp"
#include "LexingEnums.hpp"

struct LexerToken;

#define INSERT_TYPE_ENUM \
enum TypeEnum { \
none, unsignedInteger, signedInteger, floatingPoint \
}; \

#define INSERT_SUBTYPE_ENUM \
enum Subtype { \
none, value, pointer, reference \
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

struct TypeMeta {
    uint8_t pointerLevel : 7 = 0;
    uint8_t isMutable : 1 = false;

    bool operator==(const TypeMeta& type_meta) const = default;
};

struct TypeObject;

// only applies to non-primitive types
// primitives always default to 0 of their type
struct TypeObjectValue {
    // TODO: expand this, probably an unique_ptr to some structure with function call data, etc
    enum DefaultValueType {
        defaultConstructor, numeric
    };
#ifdef ENUM_VARIABLES
    DefaultValueType type = defaultConstructor;
#else
    uint8_t type = defaultConstructor;
#endif
    uint8_t isMemberValid = false;
    union {
        uint64_t numericValue = 0;
        uint64_t _unsigned;
        uint64_t _signed;
        uint64_t _float;
    };
};

namespace Instruction {
    enum Type {
        None, Expression, Return, If, Else, ElseIf, Switch, While, For, Do, Break, Continue, BeginScope, EndScope
    };
}

struct ParserTreeValue;

struct ParserInstructionObject {
#ifdef ENUM_VARIABLES
    Instruction::Type type = Instruction::None;
#else
    uint8_t type = Instruction::None;
#endif
    uint16_t expression1Index = 0;
    uint16_t expression2Index = 0;
    uint16_t expression3Index = 0;
    std::vector <ParserTreeValue> valueArray{};
};

struct ParserMemberVariableParameter {
    TypeObject* typeObject = nullptr;
    uint32_t offset = 0;
    std::vector <ParserTreeValue> definition{};

    [[nodiscard]] const TypeMeta& typeMeta() const;
    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] const std::string& typeName() const;
};

struct ParserTypeObjectMember {
    std::string memberName;
    TypeObject* memberType = nullptr;
    std::unique_ptr<std::string> memberTypeName = nullptr;
    uint32_t memberOffset = 0;
    TypeMeta type;
    TypeObjectValue memberDefaultValue;
};

std::ostream& operator<<(std::ostream& out, const ParserTypeObjectMember& member);


struct ParserValueTypeObject {
    // TODO: change to pointer to type in union
    std::string* typeName = nullptr;
    TypeMeta type;
};

struct ParserArgument {
    std::string name;
    ParserValueTypeObject type;
    TypeObjectValue defaultValue;
};

namespace ParserOperation {
    enum Type {
        None,
        // an operation of 2 operators, resolved by the bytecode generator with possible overloads
        // lvalue and rvalue of the operation
        Operator,
        // an operation of an operator and identifier, resolved by the bytecode generator with possible overloads
        // depending on the side variable on left for postfix and right for prefix
        SingleOperator,
        // a group operation, resolved by the bytecode generator with possible overloads, used by (), [], {}
        // next for members, etc. and left for value calculated, code for operator type
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
        Literal,
        // variable
        // pointer to identifier, next value for its members, etc.
        Variable,
        // string
        // pointer to string, next value for its members, etc.
        String,
        // definition
        // pointer to string with typename, value for value, next for variable name, pointer level also
        Definition
    };
}
// defines a single operation in expression
struct ParserTreeValue {
#ifdef ENUM_VARIABLES
    ParserOperation::Type operation = ParserOperation::Operator;
#else
    uint8_t operation = ParserOperation::Operator;
#endif
    uint8_t isLValued = false;
    TypeMeta typeMeta;
    union {
        TypeMeta typeInfo = {};
        struct {
            union {
                uint16_t left;
                uint16_t postfix;
                uint16_t next;
            };
            union {
                uint16_t right;
                uint16_t prefix;
                uint16_t value;
                uint16_t argument;
            };
        };
    };

    union {
        std::string* identifier = nullptr;
        const LexerToken* literal;
        uint64_t code;
        Operator::Type operatorType;
        TypeObject* definitionType;
    };
};

struct ParserFunctionMethod {
    std::vector <ParserInstructionObject> instructions;
    std::string name;
    Operator::Type overloaded = Operator::None;
    std::vector<ParserMemberVariableParameter> parameters;
    ParserValueTypeObject returnType;
};

// TODO: figure out a way to change this to temp type
struct TypeObject {
    std::string typeName;
    uint64_t isPrimitive   : 1 = false;
#ifdef ENUM_VARIABLES
    Type::TypeEnum primitiveType = Type::none;
#else
    uint8_t primitiveType : 2 = Type::none;
#endif
    uint64_t typeAlignment : 4 = 0;
    uint64_t typeSize : 57 = 0;
    std::vector <ParserMemberVariableParameter> members;
    std::vector <ParserFunctionMethod> methods;
};

inline std::unordered_map <std::string, ParserFunctionMethod> functions;
inline std::unordered_map <std::string, ParserMemberVariableParameter> globalVariables;

std::ostream& operator<<(std::ostream& out, const ParserMemberVariableParameter& variable);
std::ostream& operator<<(std::ostream& out, const TypeObject& type);


#endif //TYPE_OBJECT_HPP
