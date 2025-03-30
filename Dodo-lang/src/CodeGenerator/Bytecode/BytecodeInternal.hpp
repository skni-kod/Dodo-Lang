#ifndef BYTECODE_INTERNAL_HPP
#define BYTECODE_INTERNAL_HPP

#include <vector>
#include "Bytecode.hpp"

struct BytecodeContext {
    std::vector <Bytecode> codes;
    uint64_t tempCounter = 0;
    // if it's constant then
    bool isConstExpr = false;
    bool isMutable = false;

    // ALWAYS update the current() method after adding variables

    // makes a copy with empty vector
    [[nodiscard]] BytecodeContext current() const;

    // adds another context into this one
    void merge(const BytecodeContext& context);
};

// generates any bytecode instruction
BytecodeOperand GenerateExpressionBytecode(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false);
// only inserts default or non default operators
BytecodeOperand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index = 0, bool isGlobal = false);

#endif //BYTECODE_INTERNAL_HPP
