#ifndef DODO_LANG_ANALYSIS_INTERNAL_HPP
#define DODO_LANG_ANALYSIS_INTERNAL_HPP

#include "../../LexicalToken.hpp"
#include "../Generator.tpp"
#include "../ParserVariables.hpp"

void CreateType(Generator<const LexicalToken*>& generator);
void CreateFunction(Generator<const LexicalToken*>& generator, const std::string& returnTypeName);
FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken);

#endif //DODO_LANG_ANALYSIS_INTERNAL_HPP
