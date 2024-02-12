#ifndef DODO_LANG_PARSER_VARIABLES_HPP
#define DODO_LANG_PARSER_VARIABLES_HPP

#include <unordered_map>
#include <string>
#include <cstdint>

bool IsType(const std::string& token);

struct ParserType {
    enum type {
        SIGNED_INTEGER, UNSIGNED_INTEGER, FLOATING_POINT
    };
    uint8_t type:2;                         // allowed values 0-2
    uint8_t size:6;                         // allowed values 0-8 (maybe 16 in the future)
    ParserType(uint8_t type, uint8_t size); // assumes valid input
};

inline std::unordered_map <std::string, ParserType> parserTypes;

#endif //DODO_LANG_PARSER_VARIABLES_HPP
