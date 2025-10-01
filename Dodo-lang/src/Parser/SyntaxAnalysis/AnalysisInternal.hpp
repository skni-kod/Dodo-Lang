#ifndef DODO_LANG_ANALYSIS_INTERNAL_HPP
#define DODO_LANG_ANALYSIS_INTERNAL_HPP

#include "Generator.tpp"

void CreateType(Generator<LexerToken*>& generator, LexerToken*& firstToken);

ParserFunctionMethod CreateMethodOrFunction(Generator<LexerToken*>& generator, const ParserValueTypeObject& type, LexerToken* identifier, bool isMethod = false, Operator::Type operatorType = Operator::None, bool isExtern = false);
ParserInstructionObject ParseInstruction(Generator<LexerToken*>& generator, LexerToken* first, uint32_t* braceCounter);

std::pair<ParserValueTypeObject, LexerToken*> ParseValueType(Generator<LexerToken*>& generator, LexerToken* first);

// parses an expression, be it lvalue, rvalue or condition
// stops when it encounters a ';', '=' or some type of bracket beyond those opened in it
// returns a pointer to token it stopped on
LexerToken* ParseExpression(Generator <LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <LexerToken*> tokens);
void CheckGlobalVariables();
bool IsLValue(std::vector <ParserTreeValue>& valueArray, uint32_t start);

#endif //DODO_LANG_ANALYSIS_INTERNAL_HPP
