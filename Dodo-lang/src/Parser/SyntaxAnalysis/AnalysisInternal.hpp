#ifndef DODO_LANG_ANALYSIS_INTERNAL_HPP
#define DODO_LANG_ANALYSIS_INTERNAL_HPP

#include "../../LexicalToken.hpp"
#include "../Generator.tpp"
#include "../ParserVariables.hpp"

void CreateType(Generator<const LexerToken*>& generator, const LexerToken*& firstToken);

ParserFunctionMethodObject CreateMethodOrFunction(Generator<const LexerToken*>& generator, const ParserValueTypeObject& type, const LexerToken* identifier, uint32_t operatorType = 0);

std::pair<std::string, ParserVariable> CreateVariable(Generator<const LexicalToken*>& generator, const std::string& firstToken, bool isGlobal);

ParserFunctionMethodInstructionObject ParseInstruction(Generator<const LexerToken*>& generator, const LexerToken* first);

ParserValue ParseMath(const std::vector<const LexicalToken*>& tokens);

ParserValue ParseMath(Generator<const LexicalToken*>& generator);

std::pair<ParserValueTypeObject, const LexerToken*> ParseValueType(Generator<const LexerToken*>& generator, const LexerToken* first);

// use this one when doing stuff like a += 5 ---> a = a + 5
ParserValue ParseMath(Generator<const LexicalToken*>& generator, std::vector<const LexicalToken*> front, bool addBraces = true,
          uint64_t bracketLevel = 0);

// parses an expression, be it lvalue, rvalue or condition
// stops when it encounters a ';', '=' or some type of bracket beyond those opened in it
// returns a pointer to token it stopped on
const LexerToken* ParseExpression(Generator <const LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <const LexerToken*> tokens);

#endif //DODO_LANG_ANALYSIS_INTERNAL_HPP
