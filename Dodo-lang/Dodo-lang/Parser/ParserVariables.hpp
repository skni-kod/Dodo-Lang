#ifndef DODO_LANG_PARSER_VARIABLES_HPP
#define DODO_LANG_PARSER_VARIABLES_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "MapWrapper.tpp"

bool IsType(const std::string& token);
bool IsObject(const std::string& token);
// that is if it can be used for a variable
bool IsDeclarable(const std::string& token);

struct ParserType {
    enum type {
        SIGNED_INTEGER, UNSIGNED_INTEGER, FLOATING_POINT
    };
    uint8_t type:2;                         // allowed values 0-2
    uint8_t size:6;                         // allowed values 0-8 (maybe 16 in the future)
    //std::unique_ptr <TypeBehaviour> behaviour = nullptr;
    ParserType(uint8_t type, uint8_t size); // assumes valid input
    ParserType() = default;
};

struct ParserObject;

struct ObjectMember {
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };
    enum Type {
        value, pointer, reference
    };
    uint8_t type:2 = Type::value;
    uint8_t access:2 = Access::publicAccess;
    uint8_t defaultValue:1 = false;
    uint8_t isObject:1 = false;
    union {
        ParserType* dataPointer = nullptr;
        ParserObject* objectPointer;
    };

};

struct FunctionVariable {
    enum Type {
        value, pointer, reference
    };
    uint8_t type:2 = Type::value;
    uint8_t defaultValue:1 = false;
    uint8_t isVoid:1 = false;
    uint8_t isObject:1 = false;
    std::string name;
    union {
        ParserType* dataPointer = nullptr;
        ParserObject* objectPointer;
    };
};

struct ObjectMethod {
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };
    uint8_t access:2 = Access::publicAccess;
    std::vector <FunctionVariable> arguments;
    FunctionVariable returnType;
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
inline MapWrapper <std::string, ParserObject> parserObjects;

#endif //DODO_LANG_PARSER_VARIABLES_HPP