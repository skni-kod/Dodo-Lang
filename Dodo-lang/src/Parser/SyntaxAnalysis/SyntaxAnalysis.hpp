#ifndef DODO_LANG_SYNTAX_ANALYSIS_HPP
#define DODO_LANG_SYNTAX_ANALYSIS_HPP

#include "Lexing.hpp"
#include "Generator.tpp"

void RunSyntaxAnalysis(Generator<LexerToken*>& generator, bool isInType = false, TypeObject* type = nullptr);

#endif //DODO_LANG_SYNTAX_ANALYSIS_HPP
