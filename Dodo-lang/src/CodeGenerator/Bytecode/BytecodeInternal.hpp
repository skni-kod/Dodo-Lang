#ifndef BYTECODE_INTERNAL_HPP
#define BYTECODE_INTERNAL_HPP

#include <vector>
#include "Bytecode.hpp"



// generates any bytecode instruction
BytecodeOperand GenerateExpressionBytecode(Context& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false, BytecodeOperand passedOperand = {});
// only inserts default or non default operators
BytecodeOperand InsertOperatorExpression(Context& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false, BytecodeOperand passedOperand = {});
// calls a function or method depending on passed arguments with overloads
void BytecodeCall(Context& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, Bytecode& code, ParserTreeValue& node, bool isGlobal, bool isMethod = false, BytecodeOperand caller = {}) ;

void GetTypes(Context& context, std::vector<ParserTreeValue>& values, TypeObject*& type, TypeMeta& typeMeta, uint16_t index);
void GetTypes(Context& context, std::vector<ParserTreeValue>& values, TypeObject*& type, TypeMeta& typeMeta, ParserTreeValue& current);

#endif //BYTECODE_INTERNAL_HPP
