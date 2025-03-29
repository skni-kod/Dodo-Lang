#include "BytecodeInternal.hpp"

// goes through the bytecode for the given method/function and parses arguments with type check
// if ti's successful the argument values are added to the bytecode array
bool DoesArgumentTypesMatch(BytecodeContext& context, std::vector<ParserTreeValue>& values, ParserFunctionMethod& target, ParserTreeValue& node, bool isGlobal) {

    return true;
}

// checks if the operator has been redefined for this type
bool AssignOverloadedOperatorIfPossible(BytecodeContext& context, std::vector<ParserTreeValue>& values, Bytecode& code, ParserTreeValue& node, TypeObject* type, TypeMeta typeMeta, bool isGlobal) {
    for (uint32_t n = 0; n < type->methods.size(); n++) {
        // TODO: add support for overloading by arguments
        // also a check for multiple overloads of given type could be added here
        if (type->methods[n].overloaded != Operator::None and type->methods[n].overloaded == node.operatorType and *type->methods[n].returnType.typeName == type->typeName and type->methods[n].returnType.type == typeMeta) {
            // now we have a correct overload
            code.type = Bytecode::Method;
            code.op1(Location::Call, BytecodeValue{n});
            if (node.operation == ParserOperation::SingleOperator) {

            }
            else if (node.operation == ParserOperation::Operator) {

            }
            else {
                CodeGeneratorError("Invalid operation for overload!");
            }
            if (not DoesArgumentTypesMatch(context, values, type->methods[n],node, isGlobal )) continue;

            context.codes.push_back(code);
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
    if (AssignOverloadedOperatorIfPossible(context, values, code, values[index], type, typeMeta, isGlobal)) return code.result();

    // default operations for primitive and complex types
    // lots of repeated code
    if (type->isPrimitive) {
        auto current = values[index];
        if (current.operation == ParserOperation::Operator or current.operation == ParserOperation::SingleOperator or current.operation == ParserOperation::Group) {
            // TODO: add type checks
            switch (current.operatorType) {
                case Operator::Address:
                    code.type = Bytecode::Address;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Dereference:
                    code.type = Bytecode::Dereference;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    break;
                case Operator::Not:
                    code.type = Bytecode::Not;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinNot:
                    code.type = Bytecode::BinNot;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Increment:
                    code.type = Bytecode::Increment;
                    if (current.prefix)  code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    else
                    if (current.postfix) code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Decrement:
                    code.type = Bytecode::Decrement;
                    if (current.prefix)  code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    else
                    if (current.postfix) code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Power:
                    code.type = Bytecode::Power;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Multiply:
                    code.type = Bytecode::Multiply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Divide:
                    code.type = Bytecode::Divide;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Modulo:
                    code.type = Bytecode::Modulo;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Add:
                    code.type = Bytecode::Add;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Subtract:
                    code.type = Bytecode::Subtract;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::ShiftRight:
                    code.type = Bytecode::ShiftRight;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::ShiftLeft:
                    code.type = Bytecode::ShiftLeft;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::NAnd:
                    code.type = Bytecode::NAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinNAnd:
                    code.type = Bytecode::BinNAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::And:
                    code.type = Bytecode::And;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinAnd:
                    code.type = Bytecode::BinAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::XOr:
                    code.type = Bytecode::XOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinXOr:
                    code.type = Bytecode::BinXOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::NOr:
                    code.type = Bytecode::NOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinNOr:
                    code.type = Bytecode::BinNOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Or:
                    code.type = Bytecode::Or;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinOr:
                    code.type = Bytecode::BinOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::NImply:
                    code.type = Bytecode::NImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Imply:
                    code.type = Bytecode::Imply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinNImply:
                    code.type = Bytecode::BinNImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BinImply:
                    code.type = Bytecode::BinImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Assign:
                    code.type = Bytecode::Assign;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Lesser:
                    code.type = Bytecode::Lesser;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Greater:
                    code.type = Bytecode::Greater;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::Equals:
                    code.type = Bytecode::Equals;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::LesserEqual:
                    code.type = Bytecode::LesserEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::GreaterEqual:
                    code.type = Bytecode::GreaterEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::NotEqual:
                    code.type = Bytecode::NotEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3(Location::Temporary, {context.tempCounter++});
                    break;
                case Operator::BraceOpen:
                    code.type = Bytecode::BeginScope;
                    break;
                case Operator::BraceClose:
                    code.type = Bytecode::EndScope;
                    break;
                case Operator::Bracket:
                    if (not current.next) return GenerateExpressionBytecode(context, values, type, typeMeta, current.value, isGlobal);
                    CodeGeneratorError("Next values for grouping not implemented!");
                    break;
                case Operator::Brace:
                    CodeGeneratorError("Brace operator not implemented!");
                case Operator::Index:
                    code.type = Bytecode::Index;
                    if (not current.next) {
                        // TODO: what about the variable information in index?
                        code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.value, isGlobal));
                        code.op3(Location::Temporary, {context.tempCounter++});
                    }
                    else CodeGeneratorError("Next values for grouping not implemented!");
                    break;
                default:
                    CodeGeneratorError("Invalid operator for default implementation!");
            }
        }
        else CodeGeneratorError("Invalid operator type for default implementation!");
    }
    else {
        CodeGeneratorError("Complex type default operations not implemented!");
    }

    context.codes.push_back(code);
    return code.result();
}