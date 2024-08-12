#ifndef DODO_LANG_SYNTAX_ANALYSIS_HPP
#define DODO_LANG_SYNTAX_ANALYSIS_HPP

#include "../../LexicalToken.hpp"
#include "../Generator.tpp"

bool RunSyntaxAnalysis(Generator<const LexicalToken*>& generator);

#endif //DODO_LANG_SYNTAX_ANALYSIS_HPP
