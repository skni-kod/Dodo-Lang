#include "BytecodeInternal.hpp"

// checks if the operator has been dedefined for this type
bool AssignOverloadedOperatorIfPossible(Bytecode& code, ParserTreeValue& node, TypeObject* type, TypeMeta typeMeta, bool isGlobal) {
    for (uint32_t n = 0; n < type->methods.size(); n++) {
        // TODO: add support for overloading by arguments
        // also a check for multiple overloads of given type could be added here
        if (type->methods[n].overloaded != Operator::None and type->methods[n].overloaded == node.operatorType and *type->methods[n].returnType.typeName == type->typeName and type->methods[n].returnType.type == typeMeta) {
            // now we have a correct overload
            code.type = Bytecode::Method;
            code.op1(Location::Call, BytecodeValue{n});
            // TODO: add arguments
            return true;
        }
    }

    return false;
}

BytecodeOperand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal) {
    // first of all let's see if the type has the operator redefined
    Bytecode code;
    code.opType = type;
    code.opTypeMeta = typeMeta;
    if (AssignOverloadedOperatorIfPossible(code, values[index], type, typeMeta, isGlobal)) return code.result();

    return code.result();
}