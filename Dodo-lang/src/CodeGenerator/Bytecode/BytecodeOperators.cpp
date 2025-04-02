#include "BytecodeInternal.hpp"

// goes through the bytecode for the given method/function and parses arguments with type check
// if ti's successful the argument values are added to the bytecode array
bool DoArgumentTypesMatch(BytecodeContext& context, std::vector<ParserTreeValue>& values, ParserFunctionMethod& target, ParserTreeValue& node, bool isGlobal, uint32_t limit = 0xFFFFFFFF) {
    if (not node.next) {
        if (target.parameters.empty()) {
            return true;
        }
        return false;
    }

    ParserTreeValue currentNode = values[node.next];
    uint32_t parameterCounter = 0;

    BytecodeContext tempContext = context.current();
    std::vector <BytecodeOperand> operands;
    // going through both at the same time and checking if they match
    uint32_t counter = 0;
    do {
        if (target.parameters.size() <= parameterCounter) return false;
        ParserMemberVariableParameter& currentParameter = target.parameters[parameterCounter];

        // now the actual check, that is by far not be the most efficient way, might change this later
        // try to parse the expression, if it doesn't succeed return false
        try {
            if (currentParameter.typeObject == nullptr) {
                currentParameter.typeObject = &types[*currentParameter.definition[0].identifier];
            }
            TypeMeta meta = currentParameter.typeMeta();
            bool wasReference = meta.isReference;
            meta.isReference = false;
            bool skip = false;

            if (node.operation == ParserOperation::Argument or node.operation == ParserOperation::Call or node.operation == ParserOperation::Syscall)
                operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, meta, currentNode.value, isGlobal));
            else {
                if (counter == 1) operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, meta, node.value, isGlobal));
                else {
                    operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, meta, node.left, isGlobal));
                    counter++;
                    parameterCounter++;
                    skip = true;
                }
            }

            if (wasReference) {
                // adding the address extraction
                Bytecode code;
                code.type = Bytecode::Address;
                code.op1(operands.back());
                code.result({Location::Temporary, {tempContext.tempCounter++}});
                tempContext.codes.push_back(code);
                operands.back() = code.result();
            }

            if (skip) continue;

        }
        catch (__CodeGeneratorException& e) {
            return false;
        }

        parameterCounter++;
        if (not currentNode.next) break;
        counter++;
    }
    // TODO: add a case for when counter works with other operations
    while ((currentNode.operation == ParserOperation::Argument or counter == 1) and counter != limit);

    if (parameterCounter != target.parameters.size()) return false;

    // adding parameters themselves
    tempContext.codes.reserve(tempContext.codes.size() + operands.size());

    Bytecode arg;
    arg.type = Bytecode::Argument;
    for (uint64_t n = 0; n < operands.size(); n++) {
        arg.op2({Location::Literal, {n}});
        arg.op3(operands[n]);
        arg.opType = target.parameters[n].typeObject;
        arg.opTypeMeta = target.parameters[n].typeMeta();
        tempContext.codes.push_back(arg);
    }

    context.merge(tempContext);

    return true;
}

// checks if the operator has been redefined for this type
bool AssignOverloadedOperatorIfPossible(BytecodeContext& context, std::vector<ParserTreeValue>& values, Bytecode& code, ParserTreeValue& node, TypeObject* type, TypeMeta typeMeta, bool isGlobal) {

    bool anyFound = false;

    for (uint32_t n = 0; n < type->methods.size(); n++) {
        // TODO: add support for overloading by arguments
        // TODO: also a check for multiple overloads of given type could be added here
        if (type->methods[n].overloaded != Operator::None
            and type->methods[n].overloaded == node.operatorType
            and *type->methods[n].returnType.typeName == *type->methods[n].returnType.typeName
            and type->methods[n].returnType.type.pointerLevel == typeMeta.pointerLevel) {

            anyFound = true;

            // now we have a correct overload
            if (not DoArgumentTypesMatch(context, values, type->methods[n],node, isGlobal, 2)) continue;
            if (not type->methods[n].returnType.typeName->empty()) code.op3({Location::Temporary, {context.tempCounter++}});
            code.type = Bytecode::Method;
            code.op1Location = Location::Call;
            code.op1Value.function = &type->methods[n];
            context.codes.push_back(code);

            type->methods[n].parentType = type;
            return true;
        }
    }

    if (anyFound) CodeGeneratorError("Could not find any viable overload!");

    return false;
}

BytecodeOperand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal) {
    // first of all let's see if the type has the operator redefined
    Bytecode code;
    code.opType = type;
    code.opTypeMeta = typeMeta;
    if (AssignOverloadedOperatorIfPossible(context, values, code, values[index], type, typeMeta, isGlobal)) {
        if (values[index].operation != ParserOperation::Operator) CodeGeneratorError("Non double operator overloads not implemented!");
        // it's an operator so we don't add a pointer to the type "object"
        return code.result();
    }

    // default operations for primitive and complex types
    // lots of repeated code
    if (type->isPrimitive) {
        auto current = values[index];
        if (current.operation == ParserOperation::Operator or current.operation == ParserOperation::SingleOperator or current.operation == ParserOperation::Group) {
            switch (current.operatorType) {
                case Operator::Address:
                    code.type = Bytecode::Address;
                    code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, -1}, current.prefix, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Dereference:
                    code.type = Bytecode::Dereference;
                    code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, +1}, current.prefix, isGlobal));
                    break;
                case Operator::Not:
                    code.type = Bytecode::Not;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinNot:
                    code.type = Bytecode::BinNot;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Increment:
                    if (current.prefix) {
                        // create instructions like: var + 1 => temp, var = temp and give back temp

                        // adding
                        code.type = Bytecode::Add;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3({Location::Temporary, {context.tempCounter++}});
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::Assign;
                        code.op2({});
                        context.codes.push_back(code);

                        return code.op3();
                    }
                    if (current.postfix) {
                        // create instructions like: var => temp1, var + 1 => temp2, var = temp2 and give back temp1

                        // saving the needed value into a temporary
                        code.type = Bytecode::Save;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.postfix, isGlobal));
                        auto value = code.op3({Location::Temporary, {context.tempCounter++}});

                        // adding
                        code.type = Bytecode::Add;
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3({Location::Temporary, {context.tempCounter++}});
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::Assign;
                        code.op2({});
                        context.codes.push_back(code);

                        return value;
                    }
                    CodeGeneratorError("Invalid increment!");
                case Operator::Decrement:
                    if (current.prefix) {
                        // create instructions like: var + 1 => temp, var = temp and give back temp

                        // adding
                        code.type = Bytecode::Subtract;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3({Location::Temporary, {context.tempCounter++}});
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::Assign;
                        code.op2({});
                        context.codes.push_back(code);

                        return code.op3();
                    }
                    if (current.postfix) {
                        // create instructions like: var => temp1, var + 1 => temp2, var = temp2 and give back temp1

                        // saving the needed value into a temporary
                        code.type = Bytecode::Save;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.postfix, isGlobal));
                        auto value = code.op3({Location::Temporary, {context.tempCounter++}});

                        // adding
                        code.type = Bytecode::Subtract;
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3({Location::Temporary, {context.tempCounter++}});
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::Assign;
                        code.op2({});
                        context.codes.push_back(code);

                        return value;
                    }
                    CodeGeneratorError("Invalid decrement!");
                    break;
                case Operator::Power:
                    code.type = Bytecode::Power;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Multiply:
                    code.type = Bytecode::Multiply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Divide:
                    code.type = Bytecode::Divide;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Modulo:
                    code.type = Bytecode::Modulo;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Add:
                    code.type = Bytecode::Add;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Subtract:
                    code.type = Bytecode::Subtract;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::ShiftRight:
                    code.type = Bytecode::ShiftRight;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::ShiftLeft:
                    code.type = Bytecode::ShiftLeft;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::NAnd:
                    code.type = Bytecode::NAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinNAnd:
                    code.type = Bytecode::BinNAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::And:
                    code.type = Bytecode::And;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinAnd:
                    code.type = Bytecode::BinAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::XOr:
                    code.type = Bytecode::XOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinXOr:
                    code.type = Bytecode::BinXOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::NOr:
                    code.type = Bytecode::NOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinNOr:
                    code.type = Bytecode::BinNOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Or:
                    code.type = Bytecode::Or;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinOr:
                    code.type = Bytecode::BinOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::NImply:
                    code.type = Bytecode::NImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Imply:
                    code.type = Bytecode::Imply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinNImply:
                    code.type = Bytecode::BinNImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::BinImply:
                    code.type = Bytecode::BinImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Assign:
                    code.type = Bytecode::Assign;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Lesser:
                    code.type = Bytecode::Lesser;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Greater:
                    code.type = Bytecode::Greater;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::Equals:
                    code.type = Bytecode::Equals;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::LesserEqual:
                    code.type = Bytecode::LesserEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::GreaterEqual:
                    code.type = Bytecode::GreaterEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
                    break;
                case Operator::NotEqual:
                    code.type = Bytecode::NotEqual;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta, current.right, isGlobal));
                    code.op3({Location::Temporary, {context.tempCounter++}});
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
                        code.op3({Location::Temporary, {context.tempCounter++}});
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