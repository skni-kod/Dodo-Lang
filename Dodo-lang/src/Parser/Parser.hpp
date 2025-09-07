#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include <fstream>
#include "Lexing.hpp"


void AddString(std::string* string);
uint64_t FindString(std::string* string);
// a map might be a bad idea for this
inline std::unordered_map <std::string, uint64_t> passedStrings;
#define LONG_STRING_THRESHOLD 50
inline std::vector <std::pair<std::string*, uint64_t>> passedLongStrings;

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
