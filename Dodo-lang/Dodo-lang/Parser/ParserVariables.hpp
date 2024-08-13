#ifndef DODO_LANG_PARSER_VARIABLES_HPP
#define DODO_LANG_PARSER_VARIABLES_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "MapWrapper.tpp"

#define INSERT_SUBTYPE_ENUM \
enum Subtype { \
    value, pointer, reference, none \
};

bool IsType(const std::string& token);
bool IsObject(const std::string& token);
// that is if it can be used for a variable
bool IsDeclarable(const std::string& token);

struct ParserType {
    enum Type {
        SIGNED_INTEGER, UNSIGNED_INTEGER, FLOATING_POINT
    };
    uint8_t type:2;                         // allowed values 0-2
    uint8_t size:6;                         // allowed values 1-8 (maybe more in future)
    //std::unique_ptr <TypeBehaviour> behaviour = nullptr;
    ParserType(uint8_t type, uint8_t size); // assumes valid input
    ParserType() = default;
};

struct ParserObject;

struct ObjectMember {
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };
    INSERT_SUBTYPE_ENUM
    uint8_t type:2 = Subtype::value;
    uint8_t access:2 = Access::publicAccess;
    uint8_t defaultValue:1 = false;
    uint8_t isObject:1 = false;
    union {
        ParserType* dataPointer = nullptr;
        ParserObject* objectPointer;
    };

};

struct FunctionArgument {
    INSERT_SUBTYPE_ENUM
    uint8_t type:2 = Subtype::value;
    std::string name;
    std::string typeName;
};

// TODO: rethink this
struct ParserValue {
    enum Type {
        literalValue, calculatedValue
    };
};

struct InstructionDeclaration {
    std::string typeName;
    std::string name;
    ParserValue initialValue;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
};

struct FunctionInstruction {
    enum Type {
        declaration, returnValue, mathematical, functionCall
    };
    uint8_t type = 0;
    union Variant {
        std::unique_ptr<InstructionDeclaration> declaration;
    };
};

struct ParserFunction {
    INSERT_SUBTYPE_ENUM
    std::string returnType;
    std::string name;
    uint8_t returnValueType:2 = 0;
    std::vector <FunctionArgument> arguments;
};

struct ObjectMethod {
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };
    uint8_t access:2 = Access::publicAccess;
    std::vector <FunctionArgument> arguments;
    FunctionArgument returnType;
    //std::unique_ptr <FunctionFlow> flow;
};

struct ParserObject {
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };

    std::vector <ObjectMember> members;
    std::vector <ObjectMethod> methods;
    //std::vector <ObjectConstructor> constructors;
    //ObjectDestructor destructor;
    ParserObject* parent = nullptr;
    uint8_t parentAccess = Access::publicAccess;

};

inline MapWrapper <std::string, ParserType> parserTypes;
inline MapWrapper <std::string, ParserFunction> parserFunctions;

inline MapWrapper <std::string, ParserObject> parserObjects;

#endif //DODO_LANG_PARSER_VARIABLES_HPP