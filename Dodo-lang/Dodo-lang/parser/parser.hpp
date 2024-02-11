#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../lexicalToken.h"
#include "../lexical_analysis.h"
#include "../flags.hpp"

struct ASTTree {
    // empty for now
};

ASTTree RunParsing(const std::vector<ProgramLine>& tokens);

#endif //DODO_LANG_PARSER_HPP
