#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../lexicalToken.h"
#include "../lexical_analysis.h"
#include "../Flags.hpp"

struct ASTTree {
    // empty for now
};

class ParserException : public std::exception {
public:
    const char* what();
};

void ParserError(std::string message);

ASTTree RunParsing(const std::vector<ProgramLine>& tokens);

#endif //DODO_LANG_PARSER_HPP
