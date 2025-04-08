#include "BytecodeInternal.hpp"

// goes through the bytecode for the given method/function and parses arguments with type check
// if ti's successful the argument values are added to the bytecode array
bool DoArgumentTypesMatch(BytecodeContext& context, std::vector<ParserTreeValue>& values, ParserFunctionMethod& target, ParserTreeValue& node, bool isGlobal, bool isMethod = false, uint32_t limit = 0xFFFFFFFF) {
    if (not node.argument) {
        if (target.parameters.empty()) {
            return true;
        }
        return false;
    }

    ParserTreeValue currentNode = values[node.argument];
    uint32_t parameterCounter = 0;

    BytecodeContext tempContext = context.current();
    std::vector <Operand> operands;
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
            bool skip = false;

            if (node.operation == ParserOperation::Call or node.operation == ParserOperation::Syscall)
                operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, currentParameter.typeMeta(), currentNode.left, isGlobal));
            else {
                if (counter == 1) operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, currentParameter.typeMeta(), node.value, isGlobal));
                else {
                    operands.emplace_back(GenerateExpressionBytecode(tempContext, values, currentParameter.typeObject, currentParameter.typeMeta(), node.left, isGlobal));
                    counter++;
                    parameterCounter++;
                    skip = true;
                }
            }

            if (skip) continue;

        }
        catch (__CodeGeneratorException& e) {
            return false;
        }

        parameterCounter++;
        if (not currentNode.argument) break;
        currentNode = values[currentNode.argument];
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
        arg.op2({Location::Literal, {n + isMethod}});
        arg.op3(operands[n]);
        arg.opType = target.parameters[n].typeObject;
        arg.opTypeMeta = target.parameters[n].typeMeta();
        tempContext.codes.push_back(arg);
    }

    context.merge(tempContext);

    return true;
}

// TODO: split into 2 functions?
void BytecodeCall(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, Bytecode& code, ParserTreeValue& node, bool isGlobal, bool isMethod, Operand caller) {
    if (code.type == Bytecode::Syscall) {
        code.op1Location = Location::Literal;
        code.op1Value = values[values[node.argument].left].literal->unsigned64;

        if (not values[node.argument].argument) return;

        std::vector <Bytecode> arguments;
        uint64_t numbers = 0;
        // now going through the arguments and adding them to the context
        for (auto* current = &values[values[node.argument].argument]; ; current = &values[current->argument]) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op1Value = numbers++;
            GetTypes(context, values, arg.opType, arg.opTypeMeta, current->left);
            arg.op3(GenerateExpressionBytecode(context, values, arg.opType, arg.opTypeMeta, current->left, isGlobal));
            arguments.push_back(arg);
            if (not current->argument) break;
        }

        for (auto& n : arguments) {
            context.codes.push_back(n);
        }

        return;
    }

    if (isMethod) {
        for (auto& n : type->methods) {
            if (n.name != nullptr and *n.name != "" and *n.name == *node.identifier) {
                // TODO: what about return type?
                //if (&types[*n.returnType.typeName] != type) continue;
                //if (n.returnType.type.pointerLevel != typeMeta.pointerLevel) continue;
                //if (n.returnType.type.isMutable < typeMeta.isMutable) continue;
                //if (n.returnType.type.isReference < typeMeta.isReference) continue;

                // inserting the reference to object as first argument
                Bytecode arg;
                arg.type = Bytecode::Argument;
                arg.op1Location = Location::Literal;
                arg.op1Value.ui = 0;
                arg.result(caller);
                context.codes.push_back(arg);

                if (not DoArgumentTypesMatch(context, values, n,node, isGlobal, true)) continue;

                code.op1Location = Location::Call;
                code.op1Value.function = &n;
                code.result(context.insertTemporary(&types[*n.returnType.typeName], n.returnType.type));
                return;
            }
        }
    }
    else {
        // TODO: add overloads for global functions
        for (auto& n : functions) {
            if (n.second.name != nullptr and *n.second.name != "" and *n.second.name == *node.identifier) {
                // TODO: what about return type?
                //if (&types[*n.returnType.typeName] != type) continue;
                //if (n.returnType.type.pointerLevel != typeMeta.pointerLevel) continue;
                //if (n.returnType.type.isMutable < typeMeta.isMutable) continue;
                //if (n.returnType.type.isReference < typeMeta.isReference) continue;

                if (not DoArgumentTypesMatch(context, values, n.second,node, isGlobal)) continue;

                code.op1Location = Location::Call;
                code.op1Value.function = &n.second;
                code.result(context.insertTemporary(&types[*n.second.returnType.typeName], n.second.returnType.type));
                return;
            }
        }
    }

    CodeGeneratorError("Could not find any viable overload for: " + *node.identifier + "!");
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
            if (not DoArgumentTypesMatch(context, values, type->methods[n],node, isGlobal, false, 2)) continue;
            if (not type->methods[n].returnType.typeName->empty()) code.op3(context.insertTemporary(type, typeMeta));
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

Operand HandleCondition(BytecodeContext& context, std::vector<ParserTreeValue>& values, uint16_t index, bool isGlobal, Bytecode::BytecodeInstruction condition) {
    Bytecode code;
    code.type = condition;
    // we need to get the left side type first to know how to even compare this

    GetTypes(context, values, code.opType, code.opTypeMeta, values[index].left);

    code.op1(GenerateExpressionBytecode(context, values, code.opType, code.opTypeMeta.noReference(), values[index].left,  isGlobal));
    code.op2(GenerateExpressionBytecode(context, values, code.opType, code.opTypeMeta.noReference(), values[index].right, isGlobal));
    code.op3(context.insertTemporary(code.opType, code.opTypeMeta.noReference()));
    context.codes.push_back(code);
    return code.op3();
}

Operand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal) {
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
                    if (typeMeta.pointerLevel) CodeGeneratorError("Cannot assign an address into a non-pointer!");
                    code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, -1}, current.prefix, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta));
                    break;
                case Operator::Dereference:
                    code.type = Bytecode::Dereference;
                    code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, +1}, current.prefix, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta));
                    break;
                case Operator::Not:
                    code.type = Bytecode::Not;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.prefix, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinNot:
                    code.type = Bytecode::BinNot;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.prefix, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Increment:
                    CodeGeneratorError("Unfinished: increment!");
                    if (current.prefix) {
                        // create instructions like: var + 1 => temp, var = temp and give back temp

                        // adding
                        code.type = Bytecode::Add;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3(context.insertTemporary(type, typeMeta));
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::AssignTo;
                        code.op2({});
                        context.codes.push_back(code);

                        return code.op3();
                    }
                    if (current.postfix) {
                        // create instructions like: var => temp1, var + 1 => temp2, var = temp2 and give back temp1

                        // saving the needed value into a temporary
                        code.type = Bytecode::Save;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.postfix, isGlobal));
                        auto value = code.op3(context.insertTemporary(type, typeMeta));

                        // adding
                        code.type = Bytecode::Add;
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3(context.insertTemporary(type, typeMeta));
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::AssignTo;
                        code.op2({});
                        context.codes.push_back(code);

                        return value;
                    }
                    CodeGeneratorError("Invalid increment!");
                case Operator::Decrement:
                    CodeGeneratorError("Unfinished: decrement!");
                    if (current.prefix) {
                        // create instructions like: var + 1 => temp, var = temp and give back temp

                        // adding
                        code.type = Bytecode::Subtract;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.prefix, isGlobal));
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3(context.insertTemporary(type, typeMeta));
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::AssignTo;
                        code.op2({});
                        context.codes.push_back(code);

                        return code.op3();
                    }
                    if (current.postfix) {
                        // create instructions like: var => temp1, var + 1 => temp2, var = temp2 and give back temp1

                        // saving the needed value into a temporary
                        code.type = Bytecode::Save;
                        code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.postfix, isGlobal));
                        auto value = code.op3(context.insertTemporary(type, typeMeta));

                        // adding
                        code.type = Bytecode::Subtract;
                        code.op2({Location::Literal, {1}, type->primitiveType, static_cast<uint8_t>(type->typeSize)});
                        code.op3(context.insertTemporary(type, typeMeta));
                        context.codes.push_back(code);

                        // assigning
                        code.type = Bytecode::AssignTo;
                        code.op2({});
                        context.codes.push_back(code);

                        return value;
                    }
                    CodeGeneratorError("Invalid decrement!");
                    break;
                case Operator::Power:
                    code.type = Bytecode::Power;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Multiply:
                    code.type = Bytecode::Multiply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Divide:
                    code.type = Bytecode::Divide;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Modulo:
                    code.type = Bytecode::Modulo;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Add:
                    code.type = Bytecode::Add;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Subtract:
                    code.type = Bytecode::Subtract;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::ShiftRight:
                    code.type = Bytecode::ShiftRight;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::ShiftLeft:
                    code.type = Bytecode::ShiftLeft;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::NAnd:
                    code.type = Bytecode::NAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinNAnd:
                    code.type = Bytecode::BinNAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::And:
                    code.type = Bytecode::And;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinAnd:
                    code.type = Bytecode::BinAnd;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::XOr:
                    code.type = Bytecode::XOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinXOr:
                    code.type = Bytecode::BinXOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::NOr:
                    code.type = Bytecode::NOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinNOr:
                    code.type = Bytecode::BinNOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Or:
                    code.type = Bytecode::Or;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinOr:
                    code.type = Bytecode::BinOr;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::NImply:
                    code.type = Bytecode::NImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Imply:
                    code.type = Bytecode::Imply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinNImply:
                    code.type = Bytecode::BinNImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::BinImply:
                    code.type = Bytecode::BinImply;
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta.noReference()));
                    break;
                case Operator::Assign:
                    if (not values[current.left].isLValued) CodeGeneratorError("Cannot assign to non-lvalues!");
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left,  isGlobal));
                    code.op2(GenerateExpressionBytecode(context, values, type, typeMeta.noReference(), current.right, isGlobal));
                    if (typeMeta.isReference) {
                        code.type = Bytecode::AssignAt;
                        code.op3(code.op2());
                    }
                    else {
                        code.type = Bytecode::AssignTo;
                        code.op3(context.insertTemporary(type, typeMeta));
                    }

                    break;
                case Operator::Lesser:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::Lesser);
                case Operator::Greater:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::Greater);
                case Operator::Equals:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::Equals);
                case Operator::LesserEqual:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::LesserEqual);
                case Operator::GreaterEqual:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::GreaterEqual);
                case Operator::NotEqual:
                    return HandleCondition(context, values, index, isGlobal, Bytecode::NotEqual);
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
                        code.op3(context.insertTemporary(type, typeMeta));
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