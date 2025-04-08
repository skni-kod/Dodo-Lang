#ifndef BYTECODE_INTERNAL_HPP
#define BYTECODE_INTERNAL_HPP

#include <vector>
#include "Bytecode.hpp"



// generates any bytecode instruction
BytecodeOperand GenerateExpressionBytecode(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false, BytecodeOperand passedOperand = {});
// only inserts default or non default operators
BytecodeOperand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false);
// calls a function or method depending on passed arguments with overloads
void BytecodeCall(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, Bytecode& code, ParserTreeValue& node, bool isGlobal, bool isMethod = false, BytecodeOperand caller = {}) ;

void GetTypes(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject*& type, TypeMeta& typeMeta, uint16_t index);

#endif //BYTECODE_INTERNAL_HPP
