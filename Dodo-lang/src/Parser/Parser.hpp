#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../LexicalToken.hpp"
#include "../LexicalAnalysis.hpp"
#include "../Flags.hpp"

inline const LexicalToken* lastToken = nullptr;

const std::string* GetCurrentFile();
uint64_t GetCurrentLine();

class __ParserException : public std::exception {
public:
    const char* what();
};
inline bool doneParsing = false;
void ParserError(const std::string& message);

void RunParsing(const std::vector<ProgramPage>& tokens);

#endif //DODO_LANG_PARSER_HPP
