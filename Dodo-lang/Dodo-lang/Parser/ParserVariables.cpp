#include "ParserVariables.hpp"
#include "Parser.hpp"

bool IsType(const std::string& token) {
    return parserTypes.IsKey(token);
}

bool IsObject(const std::string& token) {
    return parserObjects.IsKey(token);
}

bool IsDeclarable(const std::string& token) {
    return IsType(token) or IsObject(token);
}

ParserType::ParserType(uint8_t type, uint8_t size) : type(type), size(size) {}
