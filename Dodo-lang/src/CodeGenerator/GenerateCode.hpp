#ifndef DODO_LANG_GENERATE_CODE_HPP
#define DODO_LANG_GENERATE_CODE_HPP

#include <string>
#include <cstdint>
#include "ParserVariables.hpp"

#define EXPRESSION_SIGN '='

struct VariableType {
    uint8_t size = 1;
    INSERT_SUBTYPE_ENUM
    uint8_t type = ParserType::Type::signedInteger;
    uint8_t subtype = Subtype::value;
    VariableType() = default;
    VariableType(uint8_t size, uint8_t type, uint8_t subtype = Subtype::value);
    bool operator==(const VariableType& var);
};

class __CodeGeneratorException : public std::exception {
public:
    const char* what();
};

void CodeGeneratorError(std::string message);

void GenerateCode();

#endif //DODO_LANG_GENERATE_CODE_HPP
