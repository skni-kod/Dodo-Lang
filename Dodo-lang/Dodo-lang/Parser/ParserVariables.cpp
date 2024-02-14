#include "ParserVariables.hpp"
#include "Parser.hpp"

bool IsType(const std::string& token) {
    return parserTypes.IsKey(token);
}

bool IsObject(const std::string& token) {
    return parserObjects.IsKey(token);
}

ParserType::ParserType(uint8_t type, uint8_t size) : type(type), size(size) {}
