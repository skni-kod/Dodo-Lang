#ifndef DODO_LANG_ANALYSIS_INTERNAL_HPP
#define DODO_LANG_ANALYSIS_INTERNAL_HPP

#include "Generator.tpp"
#include "ParserVariables.hpp"

void CreateType(Generator<LexerToken*>& generator, LexerToken*& firstToken);

ParserFunctionMethodObject CreateMethodOrFunction(Generator<LexerToken*>& generator, const ParserValueTypeObject& type, LexerToken* identifier, uint32_t operatorType = 0);
ParserFunctionMethodInstructionObject ParseInstruction(Generator<LexerToken*>& generator, LexerToken* first);

std::pair<ParserValueTypeObject, LexerToken*> ParseValueType(Generator<LexerToken*>& generator, LexerToken* first);

// parses an expression, be it lvalue, rvalue or condition
// stops when it encounters a ';', '=' or some type of bracket beyond those opened in it
// returns a pointer to token it stopped on
LexerToken* ParseExpression(Generator <LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <LexerToken*> tokens);

bool IsLValue(std::vector <ParserTreeValue>& valueArray, const uint32_t start);

#endif //DODO_LANG_ANALYSIS_INTERNAL_HPP
