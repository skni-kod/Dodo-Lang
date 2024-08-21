#ifndef DODO_LANG_PARSER_HPP
#define DODO_LANG_PARSER_HPP

#include "../LexicalToken.hpp"
#include "../LexicalAnalysis.hpp"
#include "../Flags.hpp"
#include "CreateTree/CreateTree.hpp"

class ParserException : public std::exception {
public:
    const char* what();
};

void ParserError(const std::string& message);

ASTTree RunParsing(const std::vector<ProgramPage>& tokens);

#endif //DODO_LANG_PARSER_HPP
