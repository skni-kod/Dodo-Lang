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
};

struct ParserObject;

struct ObjectMember {
    union {
        ParserType* dataPointer = nullptr;
        ParserObject* objectPointer;
    };

};

struct ParserObject {
    std::unordered_map <std::string, ParserObject> subObjects;
    std::vector <ObjectMember> members;
    //std::vector <MethodOverload> methods;
    //std::vector <ObjectConstructor> constructors;
    //ObjectDestructor destructor;
    enum Access {
        publicAccess, protectedAccess, privateAccess
    };
};

inline MapWrapper <std::string, ParserType> parserTypes;
inline MapWrapper <std::string, ParserObject> parserObjects;

#endif //DODO_LANG_PARSER_VARIABLES_HPP
