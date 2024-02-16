#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../LexicalToken.hpp"
#include "../LexicalAnalysis.hpp"
#include "../Flags.hpp"

struct ASTTree {
    // empty for now
};

class ParserException : public std::exception {
public:
    const char* what();
};

void ParserError(std::string message);

ASTTree RunParsing(const std::vector<ProgramPage>& tokens);

#endif //DODO_LANG_PARSER_HPP
