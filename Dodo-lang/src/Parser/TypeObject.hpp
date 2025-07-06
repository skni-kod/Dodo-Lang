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

namespace Type {
    enum TypeEnum {
        none = 0, address = 0, unsignedInteger, signedInteger, floatingPoint
    };
}

struct TypeMeta {
    uint8_t pointerLevel : 6 = 0;
    #ifdef PACKED_ENUM_VARIABLES
    bool isMutable : 1 = false;
    bool isReference : 1 = false;
    #else
    uint8_t isMutable : 1 = false;
    uint8_t isReference : 1 = false;
    #endif

    bool operator==(const TypeMeta& other) const;
    TypeMeta() = default;
    TypeMeta(uint8_t pointerLevel, bool isMutable, bool isReference);
    TypeMeta(const TypeMeta& old, int8_t amountToChange);
    TypeMeta noReference() const;
    TypeMeta reference() const;
};

struct TypeObject;

// only applies to non-primitive types
// primitives always default to 0 of their type
struct TypeObjectValue {
    // TODO: expand this, probably an unique_ptr to some structure with function call data, etc
    enum DefaultValueType {
        defaultConstructor, numeric
    };
#ifdef PACKED_ENUM_VARIABLES
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
        None, Expression, Return, If, Else, ElseIf, Switch, Case, While, For, Do, Break, Continue, BeginScope, EndScope, Syscall
    };
}

struct ParserTreeValue;

struct ParserInstructionObject {
#ifdef PACKED_ENUM_VARIABLES
    Instruction::Type type : 8 = Instruction::None;
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
    bool isMethod : 1 = false;
    bool isOperator : 1 = false;
    std::vector <ParserTreeValue> definition{};

    [[nodiscard]] const TypeMeta& typeMeta() const;
    [[nodiscard]] std::string& name() const;
    [[nodiscard]] std::string& typeName() const;
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
        // type identifier and meta for a cast
        TypeIdentifier,
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
#ifdef PACKED_ENUM_VARIABLES
    ParserOperation::Type operation : 8 = ParserOperation::Operator;
#else
    uint8_t operation = ParserOperation::Operator;
#endif
    uint8_t isLValued : 1 = false;
    uint8_t isBeingDefined : 1 = false;
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
        LexerToken* literal;
        uint64_t code;
        Operator::Type operatorType;
        TypeObject* definitionType;
    };
};

struct ParserFunctionMethod {
    std::vector <ParserInstructionObject> instructions;
    std::string* name = nullptr;
    Operator::Type overloaded = Operator::None;
    bool isMethod = false;
    bool isOperator = false;
    std::vector<ParserMemberVariableParameter> parameters;
    ParserValueTypeObject returnType;
    TypeObject* parentType = nullptr;

    std::string getFullName() const;
};

// TODO: figure out a way to change this to temp type
struct TypeObject {
    std::string typeName;
    uint64_t isPrimitive   : 1 = false;
#ifdef PACKED_ENUM_VARIABLES
    Type::TypeEnum primitiveType : 2 = Type::none;
#else
    uint8_t primitiveType : 2 = Type::none;
#endif
    uint64_t typeAlignment : 4 = 0;
    uint64_t typeSize : 57 = 0;
    std::vector <ParserMemberVariableParameter> members;
    std::vector <ParserFunctionMethod> methods;

    uint64_t getMemberOffsetAndType(std::string* identifier, TypeObject*& typeToSet, TypeMeta& typeMetaToSet);
    ParserFunctionMethod& findMethod(std::string* identifier);
};

inline std::unordered_map<std::string, TypeObject> types;
inline std::unordered_map <std::string, ParserFunctionMethod> functions;
//inline std::unordered_map <std::string, std::vector<ParserFunctionMethod>> functions;
inline std::unordered_map <std::string, ParserMemberVariableParameter> globalVariables;

std::ostream& operator<<(std::ostream& out, const ParserMemberVariableParameter& variable);
std::ostream& operator<<(std::ostream& out, const TypeObject& type);


#endif //TYPE_OBJECT_HPP
