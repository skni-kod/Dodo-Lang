#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../LexicalToken.hpp"
#include "../LexicalAnalysis.hpp"
#include "Lexer/Lexing.hpp"

inline const LexicalToken* lastTokenDEPRECATED = nullptr;

inline std::vector <std::string> passedStrings;

const std::string* GetCurrentFile();

uint64_t GetCurrentLine();

class ParserException final : public std::exception {
public:
    const char* what();
};

inline const LexerToken* lastToken = nullptr;

inline bool doneParsing = false;

void ParserError(const std::string& message);

//void RunParsing(const std::vector<ProgramPage>& tokens);
void RunParsing(std::vector<LexerFile>& lexed);

void PrepareFunctionArguments();

bool IsNumeric(const LexicalToken* token);

void CalculateTypeSizes();

#endif //DODO_LANG_PARSER_HPP
