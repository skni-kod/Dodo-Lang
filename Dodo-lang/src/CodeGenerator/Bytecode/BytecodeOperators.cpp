#include <complex>
#include <GenerateCode.hpp>
#include <iostream>
#include <Lexing.hpp>

#include "BytecodeInternal.hpp"

struct TypeMetaPair {
    TypeMeta meta{};
    bool isLiteral = false;
    TypeObject* type = nullptr;
    Bytecode code;
};

std::vector<TypeMetaPair> FindArgumentTypes(BytecodeContext& context, std::vector<ParserTreeValue>& values, ParserTreeValue& node) {

    if (not node.argument) return {};

    std::vector <TypeMetaPair> arguments;
    // now going through the arguments and adding them to the context
    for (auto* current = &values[node.argument]; ; current = &values[current->argument]) {
        TypeMetaPair arg;

        GetTypes(context, values, arg.type, arg.meta, current->left);
        arg.isLiteral = values[current->left].operation == ParserOperation::Literal;

        arguments.push_back(arg);
        if (not current->argument) break;
    }

    return arguments;
}

bool DoesFunctionMatchAndAddCall(std::vector<TypeMetaPair>& arguments, BytecodeContext& context, ParserFunctionMethod& overload, std::vector<ParserTreeValue>& values, ParserTreeValue& node, Bytecode& code, bool isGlobal) {
    // TODO: change this after adding default parameters
    if (overload.parameters.size() != arguments.size()) return false;

    for (auto n = 0; n < arguments.size(); n++) {
        // pointer mismatch
        if (arguments[n].meta.pointerLevel != overload.parameters[n].typeMeta().pointerLevel) return false;;
        // literal reference
         if (not arguments[n].meta.isReference and overload.parameters[n].typeMeta().isReference and arguments[n].isLiteral) return false;
        // immutable argument to mutable parameter
        if (not arguments[n].meta.isMutable and overload.parameters[n].typeMeta().isMutable and not arguments[n].isLiteral and arguments[n].meta != TypeMeta(0, true, false)) return false;

        if (overload.parameters[n].typeObject == nullptr)
            overload.parameters[n].typeObject = &types[*overload.parameters[n].definition[0].identifier];

        // non-literal type mismatch
        if (not arguments[n].isLiteral and overload.parameters[n].typeObject != arguments[n].type) return false;
        // TODO: add check for literal non-convertible to other types
        // else if (...) return false;
    }

    std::vector<Bytecode> argumentCodes;
    uint64_t numbers = 0;
    if (node.operation == ParserOperation::Call) {
        for (auto* current = &values[node.argument]; ; current = &values[current->argument]) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op3(GenerateExpressionBytecode(context, values, overload.parameters[numbers].typeObject, overload.parameters[numbers].typeMeta(), current->left, isGlobal));
            arg.op1Value = numbers++;
            argumentCodes.push_back(arg);
            if (not current->argument) break;
        }
    }
    else {
        if (node.left) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op3(GenerateExpressionBytecode(context, values, overload.parameters[numbers].typeObject, overload.parameters[numbers].typeMeta(), node.left, isGlobal));
            arg.op1Value = numbers++;
            argumentCodes.push_back(arg);
        }
        if (node.right) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op3(GenerateExpressionBytecode(context, values, overload.parameters[numbers].typeObject, overload.parameters[numbers].typeMeta(), node.right, isGlobal));
            arg.op1Value = numbers++;
            argumentCodes.push_back(arg);
        }
    }


    code.op1Location = Location::Call;
    code.op1Value.function = &overload;

    if (overload.returnType.typeName != nullptr)
        code.result(context.insertTemporary(&types[*overload.returnType.typeName], overload.returnType.type));

    for (auto& n : argumentCodes) {
        context.codes.push_back(n);
    }

    return true;
}

void BytecodeCall(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, Bytecode& code, ParserTreeValue& node, bool isGlobal, bool isMethod, BytecodeOperand caller) {
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

    // first off, let's get the types of the arguments
    auto arguments = FindArgumentTypes(context, values, node);

    if (isMethod) {
        // TODO: check if this actually works
        for (auto& n : type->methods) {
            if (n.name != nullptr and !n.name->empty() and *n.name == *node.identifier) {
                if (DoesFunctionMatchAndAddCall(arguments, context, n, values, node, code, isGlobal)) return;
            }
        }

        CodeGeneratorError("Could not find a valid overload for method: " + *node.identifier + "!");
    }
    else {
        if (not functions.contains(*node.identifier))
            CodeGeneratorError("Function identifier: " + *node.identifier + " not found!");

        auto& overloads = functions[*node.identifier];
        for (auto& overload : overloads) {
            if (DoesFunctionMatchAndAddCall(arguments, context, overload, values, node, code, isGlobal)) return;
        }

        CodeGeneratorError("Could not find a valid overload for function: " + *node.identifier + "!");

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
            and type->methods[n].returnType.type.pointerLevel == typeMeta.pointerLevel) {

            anyFound = true;

            std::vector <TypeMetaPair> arguments;
            if (node.left) {
                TypeMetaPair arg;
                GetTypes(context, values, arg.type, arg.meta, node.left);
                arg.isLiteral = values[node.left].operation == ParserOperation::Literal;
                arguments.emplace_back(arg);
            }
            if (node.right) {
                TypeMetaPair arg;
                GetTypes(context, values, arg.type, arg.meta, node.right);
                arg.isLiteral = values[node.right].operation == ParserOperation::Literal;
                arguments.emplace_back(arg);
            }





            //auto arguments = FindArgumentTypes(context, values, node);

            if (DoesFunctionMatchAndAddCall(arguments, context, type->methods[n], values, node, code, isGlobal)) {
                code.type = Bytecode::Method;
                context.codes.push_back(code);
                return true;
            }
        }
    }

    if (anyFound) CodeGeneratorError("Could not find any viable overload!");

    return false;
}

BytecodeOperand HandleCondition(BytecodeContext& context, std::vector<ParserTreeValue>& values, uint16_t index, bool isGlobal, Bytecode::BytecodeInstruction condition) {
    Bytecode code;
    code.type = condition;
    // we need to get the left side type first to know how to even compare this

    GetTypes(context, values, code.opType, code.opTypeMeta, values[index].left);

    code.op1(GenerateExpressionBytecode(context, values, code.opType, code.opTypeMeta.noReference(), values[index].left,  isGlobal));
    code.op2(GenerateExpressionBytecode(context, values, code.opType, code.opTypeMeta.noReference(), values[index].right, isGlobal));
    code.op3(context.insertTemporary(&types["bool"], {}));
    context.codes.push_back(code);
    return code.op3();
}

BytecodeOperand InsertOperatorExpression(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal, BytecodeOperand passedOperand) {
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
    auto current = values[index];
    if (type->isPrimitive) {
        if (current.operation == ParserOperation::Operator or current.operation == ParserOperation::SingleOperator or current.operation == ParserOperation::Group) {
            switch (current.operatorType) {
                case Operator::Address:
                    code.type = Bytecode::Address;
                    if (typeMeta.pointerLevel == 0) CodeGeneratorError("Cannot assign an address into a non-pointer!");
                    // TODO: taking addresses from referenences?
                    code.op1(GenerateExpressionBytecode(context, values, type, TypeMeta(typeMeta, -1).noReference(), current.prefix, isGlobal));
                    code.op3(context.insertTemporary(type, typeMeta));
                    break;
                case Operator::Cast: {
                    code.type = Bytecode::Cast;
                    auto& castType = values[current.right];
                    // TODO: implicit type conversion?
                    // TODO: rename cast to convert
                    if (*castType.identifier != "auto") {
                        if (not types.contains(*castType.identifier)) CodeGeneratorError("Casting to non-type!");
                        if (&types[*castType.identifier] != type) CodeGeneratorError("Casting to invalid type!");
                        if (castType.typeMeta.pointerLevel != typeMeta.pointerLevel or castType.typeMeta.isReference != typeMeta.isReference) CodeGeneratorError("Casting to invalid type modification!");
                    }

                    // result is of the cast type
                    code.op3(context.insertTemporary(type, typeMeta));
                    // get the type of the last member of the lvalue used
                    bool mutability = typeMeta.isMutable;
                    if (not type->isPrimitive) CodeGeneratorError("Cannot cast to non-primitive types!");
                    GetTypes(context, values, type, typeMeta, current.left);
                    typeMeta.isMutable = mutability;
                    if (not type->isPrimitive) CodeGeneratorError("Cannot cast from non-primitive types!");
                    code.op1(GenerateExpressionBytecode(context, values, type, typeMeta, current.left, isGlobal));
                }
                    break;
                case Operator::Dereference:
                    if (typeMeta.isReference)
                        code.type = Bytecode::ToReference;
                    else code.type = Bytecode::Dereference;
                    // TODO: add the case for when the target is a reference already or something
                    code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, 1}, current.prefix, isGlobal));
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
                        code.op3(context.insertTemporary(type, typeMeta));
                    }
                    else {
                        code.type = Bytecode::AssignTo;
                        code.op3(code.op1());
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
                    Warning("Expressions grouped by brackets are serverly undertested and may not work correctly!");
                    code.result(GenerateExpressionBytecode(context, values, type, typeMeta, current.value, isGlobal));
                    // sets the meta to the result of the main code in the bracket
                    if (context.codes.size() > 1
                        && context.codes.back().result().location == Location::Variable)
                        typeMeta = context.getVariableObject(context.codes.back().result()).meta;
                    return GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, code.result());
                case Operator::Brace: {
                    code.type = Bytecode::BraceListStart;
                    code.opType = type;
                    code.opTypeMeta = TypeMeta(typeMeta, -1).noReference();
                    auto startCodeIndex = context.codes.size();
                    context.codes.push_back(code);

                    uint64_t amount = 0;
                    auto* element = &current;
                    while (element->value != 0) {
                        element = &values[element->value];
                        Bytecode elementCode{};
                        elementCode.type = Bytecode::BraceListElement;
                        elementCode.opType = code.opType;
                        elementCode.opTypeMeta = code.opTypeMeta;
                        elementCode.op1(GenerateExpressionBytecode(context, values, type, code.opTypeMeta, element->left, isGlobal));
                        elementCode.op2({Location::Literal, ++amount, Type::unsignedInteger, Options::addressSize});
                        context.codes.emplace_back(elementCode);
                    }

                    context.codes[startCodeIndex].op1({Location::Literal, amount, Type::unsignedInteger, Options::addressSize});

                    // adding the last element with the return value passing it to the start too to set stack content
                    code.type = Bytecode::BraceListEnd;
                    context.codes[startCodeIndex].op3(code.op3(context.insertTemporary(type, code.opTypeMeta)));
                    context.codes.push_back(code);

                    // now we need the get the address from the stack into a variable
                    code.opTypeMeta = typeMeta;
                    code.type = Bytecode::Address;
                    code.op1(code.op3());
                    code.op3(context.insertTemporary(code.opType, code.opTypeMeta));


                    }

                    break;
                case Operator::Index:
                    code.type = Bytecode::GetIndexValue;
                    code.op1(passedOperand);
                    code.op2(GenerateExpressionBytecode(context, values, type, TypeMeta(), current.value, isGlobal));
                    if (typeMeta.pointerLevel == 0) CodeGeneratorError("Internal: cannot index in a value!");
                    typeMeta.pointerLevel--;
                    code.op3(context.insertTemporary(type, typeMeta));
                    context.codes.push_back(code);
                    if (current.next)
                        return GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, code.result());
                    return code.result();
                default:
                    CodeGeneratorError("Invalid operator for default implementation!");
            }
        }
        else CodeGeneratorError("Invalid operator type for default implementation!");
    }
    else {
        if ((current.operation == ParserOperation::Operator or current.operation == ParserOperation::SingleOperator or current.operation == ParserOperation::Group) and current.operatorType == Operator::Address) {
            code.type = Bytecode::Address;
            if (not typeMeta.pointerLevel) CodeGeneratorError("Cannot assign an address into a non-pointer!");
            code.op1(GenerateExpressionBytecode(context, values, type, {typeMeta, -1}, current.prefix, isGlobal));
            code.op3(context.insertTemporary(type, typeMeta));
        }
        else CodeGeneratorError("Complex type default operations not implemented!");

    }

    context.codes.push_back(code);
    return code.result();
}