#include "ParserVariables.hpp"
#include "Parser.hpp"

bool IsType(const std::string& token) {
    if (parserTypes.find(token) == parserTypes.end()) {
        return false;
    }
    return true;
}

ParserType::ParserType(uint8_t type, uint8_t size) : type(type), size(size) {}
