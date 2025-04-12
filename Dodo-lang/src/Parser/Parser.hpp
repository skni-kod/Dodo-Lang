#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include <fstream>
#include "Lexing.hpp"

inline std::vector <std::string*> passedStrings;

const std::string* GetCurrentFile();

uint64_t GetCurrentLine();

class ParserException final : public std::exception {
public:
    const char* what();
};

inline const LexerToken* lastToken = nullptr;

inline bool doneParsing = false;

void ParserError(const std::string& message);

void RunParsing(std::vector<LexerFile>& lexed);

void PrepareFunctionArguments();

void CalculateTypeSizes();

#endif //DODO_LANG_PARSER_HPP
