#ifndef DODO_LANG_ANALYSIS_INTERNAL_HPP
#define DODO_LANG_ANALYSIS_INTERNAL_HPP

#include "../../LexicalToken.hpp"
#include "../Generator.tpp"
#include "../ParserVariables.hpp"

void CreateType(Generator<const LexicalToken*>& generator);
void CreateFunction(Generator<const LexicalToken*>& generator, const std::string& returnTypeName);
FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken);
ParserValue ParseMath(const std::vector<const LexicalToken*>& tokens);
ParserValue ParseMath(Generator<const LexicalToken*> &generator);
// use this one when doing stuff like a += 5 ---> a = a + 5
ParserValue ParseMath(Generator<const LexicalToken*> &generator, std::vector<const LexicalToken*> front, bool addBraces = true);

#endif //DODO_LANG_ANALYSIS_INTERNAL_HPP