#ifndef DODO_LANG_PARSER_VARIABLES_HPP
#define DODO_LANG_PARSER_VARIABLES_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include "MapWrapper.tpp"
#include "Parser.hpp"
#include "Generator.tpp"

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
    uint8_t type = 0;
    // this constructor automatically parses mathematical expressions
    // BEWARE it also takes the ',' or ';' at the end
    ParserValue(Generator<const LexicalToken*>& generator);
    ParserValue() = default;
};

struct DeclarationInstruction {
    std::string typeName;
    std::string name;
    ParserValue expression;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
};

struct ReturnInstruction {
    ParserValue expression;
    INSERT_SUBTYPE_ENUM
    uint8_t subtype = 0;
    explicit ReturnInstruction(Generator<const LexicalToken*>& generator);
};

struct FunctionInstruction {
    enum Type {
        declaration, returnValue, mathematical, functionCall
    };
    union Variant {
        DeclarationInstruction* declarationInstruction = nullptr;
        ReturnInstruction* returnInstruction;
    }Variant;
    uint8_t type = 0;
    ~FunctionInstruction();
    FunctionInstruction() = default;
    FunctionInstruction(const FunctionInstruction& F);
    FunctionInstruction(FunctionInstruction&& F);

};

struct ParserFunction {
    INSERT_SUBTYPE_ENUM
    std::string returnType;
    std::string name;
    uint8_t returnValueType:2 = 0;
    std::vector <FunctionArgument> arguments;
    std::vector <FunctionInstruction> instructions;
};

inline MapWrapper <std::string, ParserType> parserTypes;
inline MapWrapper <std::string, ParserFunction> parserFunctions;


#endif //DODO_LANG_PARSER_VARIABLES_HPP