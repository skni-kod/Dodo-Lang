#include <complex>
#include <GenerateCode.hpp>
#include <iostream>
#include <tbb/internal/_template_helpers.h>

#include "Bytecode.hpp"
#include "Lexing.hpp"

#include "BytecodeInternal.hpp"
#include "ErrorHandling.hpp"

std::vector<TypeInfo> FindArgumentTypes(Context& context, std::vector<ParserTreeValue>& values,
                                        ParserTreeValue& node) {
    if (not node.argument) return {};

    std::vector<TypeInfo> arguments;
    // now going through the arguments and adding them to the context
    for (auto* current = &values[node.argument]; ; current = &values[current->argument]) {
        TypeInfo arg;
        GetTypes(context, values, arg, current->left);
        arguments.push_back(arg);
        if (not current->argument) break;
    }

    return arguments;
}

bool AddCallIfMatches(Context& context, ParserFunctionMethod* called,
                    std::vector<ParserTreeValue>& values, ParserTreeValue& node, std::vector<TypeInfo>& arguments,
                    Bytecode& code, BytecodeOperand passedOperand, bool isGlobal) {
    // TODO: change this after adding default parameters
    if (called != nullptr and called->parameters.size() != arguments.size()) return false;

    auto secondaryPassedOperand = passedOperand;
    if (called != nullptr) {
        for (auto n = 0; n < arguments.size(); n++) {
            auto& arg = arguments[n];
            auto par = TypeInfo(called->parameters[n].typeObject, called->parameters[n].typeMeta());

            if (arg.type != par.type) return false;
            if (arg.pointerLevel != par.pointerLevel) return false;
            if (arg.isMutable < par.isMutable and (arg.isReference or arg.pointerLevel)) return false;
            // TODO: allow for reference arguments somehow, maybe somehow check for lvalues or just assume that that's it
            if (not arg.isReference and par.isReference)
                Unimplemented();

            // TODO: think about these checks a little more
        }

        // at this point it is a valid call and any errors mean the code is invalid

        // in case of constructors we need to add a definition for the new variable
        if (called->isConstructor) {
            Bytecode res{};
            res.type = Bytecode::Define;
            res.opType = called->parentType;
            res.opMeta = TypeMeta(0, true, false);
            secondaryPassedOperand = passedOperand = res.op1(context.insertTemporary(res.opType, res.opMeta));
            context.codes.push_back(res);
        }
    }



    uint64_t numbers = 0;
    std::vector <Bytecode> argumentCodes;

    // adding the pointer to type instance for methods, constructors and destructors
    if (called != nullptr and called->isMethod and not called->isOperator) {
        DebugError(passedOperand.location != Location::Var, "Expected a variable passed in call!");


        Bytecode arg;
        arg.type = Bytecode::Argument;
        arg.AssignType(called->parentType, {0, not called->isConst, true});
        if (not context.getVariableObject(passedOperand).meta.isReference)
            passedOperand = GetAddress(context, passedOperand, {called->parentType, {0, not called->isConst, true}});
        arg.op3(passedOperand);
        arg.op1Value = ++numbers;
        argumentCodes.push_back(arg);
    }

    // now we can actually prepare the arguments and do the call
    if (node.operation == ParserOperation::Call or node.operation == ParserOperation::Syscall) {
        uint64_t index = 0;
        if (called == nullptr or not called->parameters.empty())
            for (auto* current = &values[node.argument]; ; current = &values[current->argument]) {
                Bytecode arg;
                arg.type = Bytecode::Argument;

                TypeInfo expected;
                if (called != nullptr)
                    expected = TypeInfo(called->parameters[index].typeObject, called->parameters[index].typeMeta());
                else
                    GetTypes(context, values, expected, current->left);
                arg.op3(GenerateExpressionRunner(context, values,  expected, isGlobal, current->left));
                arg.op1Value = ++numbers;
                argumentCodes.push_back(arg);

                ++index;
                if (not current->argument) break;
            }
    }
    else if (called->isOperator) {
        if (node.left) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op3(GenerateExpressionRunner(context, values, {called->parameters[numbers].typeObject, called->parameters[numbers].typeMeta()}, isGlobal, node.left));
            arg.op1Value = ++numbers;
            argumentCodes.push_back(arg);
        }
        if (node.right) {
            Bytecode arg;
            arg.type = Bytecode::Argument;
            arg.op3(GenerateExpressionRunner(context, values, {called->parameters[numbers].typeObject, called->parameters[numbers].typeMeta()}, isGlobal, node.right));

            arg.op1Value = ++numbers;
            argumentCodes.push_back(arg);
        }
    }
    else Unimplemented();

    for (auto& n : argumentCodes)
        context.codes.push_back(n);

    if (node.operation == ParserOperation::Syscall)
        code.type = Bytecode::Syscall;
    else if (called->isMethod)
        code.type = Bytecode::Method;
    else
        code.type = Bytecode::Function;

    code.op1Value.function = called;

    if (code.type != Bytecode::Syscall) {
        if (called != nullptr and called->returnType.typeName != nullptr) {
            if (not types.contains(*called->returnType.typeName))
                Error("Given return type does not exist!");
            code.result(context.insertTemporary(code.AssignType({&types[*called->returnType.typeName], called->returnType.type})));
        }
        else if (called != nullptr and called->isConstructor)
            code.result(secondaryPassedOperand);
    }
    else {
        code.result(context.insertTemporary(code.AssignType({&types["u" + std::to_string(8 * Options::addressSize)], {}})));
    }

    return true;
}

// checks if the operator has been redefined for this type
bool AssignOverloadedOperatorIfPossible(Context& context, std::vector<ParserTreeValue>& values, Bytecode& code,
                                        ParserTreeValue& node, const TypeInfo expected, TypeInfo& actual,
                                        bool isGlobal) {
    bool anyFound = false;

    if (actual.type == nullptr) {
        GetTypes(context, values, actual, node);

        DebugError(actual.type == nullptr, "Expected a type to be found!");
    }

    std::vector<TypeInfo> arguments{};
    for (auto& method : actual.type->methods) {
        if (method.overloaded != Operator::None
                    and method.overloaded == node.operatorType
                    and method.returnType.type.pointerLevel == actual.pointerLevel) {


            if (not anyFound) {
                anyFound = true;

                if (node.left) {
                    TypeInfo arg;
                    GetTypes(context, values, arg, node.left);
                    arguments.emplace_back(arg);
                }
                if (node.right) {
                    TypeInfo arg;
                    GetTypes(context, values, arg, node.right);
                    arguments.emplace_back(arg);
                }
            }

            if (AddCallIfMatches(context, &method, values, node, arguments, code, {}, isGlobal)) {
                code.type = Bytecode::Method;
                context.codes.push_back(code);
                return true;
            }

        }
    }

    if (anyFound)
        Error("Found no viable overload for operator!");

    return false;
}


void PerformTwoOperatorExpression(Context& context, std::vector<ParserTreeValue>& values, const TypeInfo expected, TypeInfo& actual, Bytecode& code, ParserTreeValue current, bool isGlobal) {

    code.op1(GenerateExpressionBytecode(context, values, expected, actual, current.left, isGlobal));
    auto leftType = actual;
    auto rightExpected = actual;
    rightExpected.isReference = false;
    auto right = GenerateExpressionBytecode(context, values, rightExpected, actual, current.right, isGlobal);
    code.op2(CheckCompatibilityAndConvertReference(context, rightExpected, actual, right));
    actual = leftType;
    code.AssignType(actual);
}

/// <summary>
/// Handles a simple operation that takes 2 operands and returns a mofified R-VALUE (!) (there are checks for references)
/// </summary>
/// <param name="instruction">Instruction type</param>
/// <param name="context">Context</param>
/// <param name="values">tree value array</param>
/// <param name="expected">Expected type</param>
/// <param name="actual">Actual type</param>
/// <param name="code">Current Bytecode</param>
/// <param name="current">Current tree value</param>
/// <param name="isGlobal">Is in global context</param>
/// <returns></returns>
BytecodeOperand HandleSimpleOperation(Bytecode::BytecodeInstruction instruction, Context& context, std::vector<ParserTreeValue>& values, const TypeInfo expected, TypeInfo& actual, Bytecode& code, ParserTreeValue current, bool isGlobal) {
    code.type = instruction;

    if (expected.isReference)
        Error("Cannot generate a reference from assignment!");

    PerformTwoOperatorExpression(context, values, expected, actual, code, current, isGlobal);

    code.result(context.insertTemporary(actual));
    return context.addCodeReturningResult(code);
}

BytecodeOperand HandleConditionalOperation(Bytecode::BytecodeInstruction instruction, Context& context, std::vector<ParserTreeValue>& values, const TypeInfo expected, TypeInfo& actual, Bytecode& code, ParserTreeValue current, bool isGlobal) {
    code.type = instruction;

    if (expected.isReference)
        Error("Cannot generate a reference from assignment!");

    PerformTwoOperatorExpression(context, values, expected, actual, code, current, isGlobal);

    // since it's a condition we convert the resulting type to a bool
    actual = {&types["bool"], {}};

    code.result(context.insertTemporary(actual));
    return context.addCodeReturningResult(code);
}

BytecodeOperand InsertOperatorExpression(Context& context, std::vector<ParserTreeValue>& values,
                                         const TypeInfo expected, TypeInfo& actual,
                                         uint16_t index, bool isGlobal, BytecodeOperand passedOperand) {
    // first of all let's see if the type has the operator redefined
    // if it was then the default implementation goes straight to trash and only overloads can be used
    Bytecode code;
    if (AssignOverloadedOperatorIfPossible(context, values, code, values[index], expected, actual, isGlobal)) {
        if (values[index].operation != ParserOperation::Operator)
            Error("Non double operator overloads not implemented!");
        // it's an operator so we don't add a pointer to the type "object"
        return code.result();
    }

    auto current = values[index];

    DebugError(current.operation != ParserOperation::Operator
               and current.operation != ParserOperation::SingleOperator
               and current.operation != ParserOperation::Group, "Invalid operator type for default implementation!");

    switch (current.operatorType) {

    case Operator::Address: {
        code.type = Bytecode::Address;

        // TODO: should references be treated as pointers? Probably not?
        if ((actual.type != nullptr and actual.pointerLevel == 0)
            or (actual.type == nullptr and expected.pointerLevel == 0))
            Error("Cannot assign an address to a value!");

        if (not current.value)
            Error("Address operator must have a value!");

        code.op1(GenerateExpressionBytecode(context, values, expected, actual, current.value, isGlobal));
        DebugError(actual.type == nullptr, "Type should be assigned after an address operator");

        // now let's make the found type a pointer
        actual = {actual, +1};
        code.AssignType(actual);

        // and return it
        code.result(context.insertTemporary(actual));
        return context.addCodeReturningResult(code);
    }

    case Operator::Dereference: {
        code.type = Bytecode::Dereference;

        if (not current.value)
            Error("Dereference operator must have a value!");

        code.op1(GenerateExpressionBytecode(context, values, expected, actual, current.value, isGlobal));
        DebugError(actual.type == nullptr, "Type should be assigned after a dereference operator");

        // now let's make the found less of a pointer
        actual = {actual, -1};
        code.AssignType(actual);

        // and return it
        code.result(context.insertTemporary(actual));
        return context.addCodeReturningResult(code);
    }

    case Operator::Convert: {
        code.type = Bytecode::Convert;
        auto& assumedType = values[current.right];

        DebugError(assumedType.identifier == nullptr, "Type conversion type identifier not assigned!");

        TypeInfo typeToUse;
        if (*assumedType.identifier == "auto") {
            if (expected.type == nullptr)
                Error("Cannot automatically perform a type conversion if no type is expected!");
            code.AssignType(actual = typeToUse = expected);
        }
        else {
            if (not types.contains(*assumedType.identifier))
                Error("Type: " + *assumedType.identifier + " does not exist!");
            auto& type = types[*assumedType.identifier];
            actual = typeToUse = {&type, current.typeMeta};
        }

        // now let's finally perform the operation and check if the type can be converted
        code.op1(GenerateExpressionBytecode(context, values, typeToUse, actual, current.left, isGlobal));
        if (not typeToUse.type->attributes.primitiveSimplyConvert or not actual.type->attributes.primitiveSimplyConvert) {
            // TODO: add default conversions type attribute
            // TODO: add a method that looks for constructors for given types to convert, like type.TryToConvert(...)
            Unimplemented();
        }
        actual = typeToUse;

        code.result(context.insertTemporary(actual));
        code.AssignType(actual);
        return context.addCodeReturningResult(code);
    }

    case Operator::Assign: {

        PerformTwoOperatorExpression(context, values, {}, actual, code, current, isGlobal);

        DebugError(actual.type == nullptr, "Expected lvalue type to be assigned!");
        if (not actual.isMutable and not isGlobal and not current.isBeingDefined)
            Error("Assignment target is immutable!");

        // assigning the correct variant
        if (actual.isReference) {
            code.type = Bytecode::AssignAt;
        }
        else
            code.type = Bytecode::AssignTo;

        code.result(context.insertTemporary(actual));
        return context.addCodeReturningResult(code);
    }

    case Operator::BraceOpen: {
        code.type = Bytecode::BeginScope;
        return context.addCodeReturningResult(code);
    }

    case Operator::BraceClose: {
        code.type = Bytecode::EndScope;
        return context.addCodeReturningResult(code);
    }

    case Operator::Bracket: {
        auto result = GenerateExpressionBytecode(context, values, expected, actual, current.value, isGlobal);

        if (not current.next)
            return result;

        return GenerateExpressionBytecode(context, values, expected, actual, current.next, isGlobal, result);
    }

    case Operator::Brace: {
        code.type = Bytecode::BraceListStart;

        if (expected.pointerLevel == 0)
            Error("Cannot assign a braced array list to a value!");

        actual = expected;

        auto startCodeIndex = context.codes.size();
        actual = {actual, -1};
        actual.isReference = false;
        code.AssignType(actual);

        context.codes.push_back(code);

        uint64_t amount = 0;
        auto* element = &current;
        while (element->value != 0) {
            element = &values[element->value];
            Bytecode elementCode{};
            elementCode.type = Bytecode::BraceListElement;
            elementCode.AssignType(actual);
            elementCode.op1(GenerateExpressionBytecode(context, values, actual, actual, element->left, isGlobal));
            elementCode.op2({Location::Literal, ++amount, Type::unsignedInteger, Options::addressSize});
            context.codes.emplace_back(elementCode);
        }

        context.codes[startCodeIndex].op1({
            Location::Literal, amount, Type::unsignedInteger, Options::addressSize
        });

        // adding the last element with the return value passing it to the start too to set stack content
        code.type = Bytecode::BraceListEnd;
        context.codes[startCodeIndex].op3(code.op3(context.insertTemporary(expected)));
        context.codes.push_back(code);

        // now we need the get the address from the stack into a variable
        code.AssignType(actual = expected);
        code.type = Bytecode::Address;
        code.op1(code.op3());
        code.op3(context.insertTemporary(code.opType, code.opMeta));
        return context.addCodeReturningResult(code);
    }

    case Operator::Index: {
        DebugError(passedOperand.location == Location::None, "Expected an operand to be passed!");
        DebugError(not actual.isReference, "Expected previous value to provide a reference!");

        if (expected.isReference)
            code.type = Bytecode::GetIndexAddress;
        else {
            code.type = Bytecode::GetIndexValue;
            if (actual.isReference) {
                actual.isReference = false;
                passedOperand = Dereference(context, passedOperand, actual);
            }
        }

        if (actual.pointerLevel == 0)
            Error("Cannot index in a value!");
        actual = {actual, -1};

        code.op1(passedOperand);
        code.op2(GenerateExpressionBytecode(context, values, expected, actual, current.value, isGlobal));
        code.op3(context.insertTemporary(actual));
        context.codes.push_back(code);
        if (current.next)
            return GenerateExpressionBytecode(context, values, expected, actual, current.next, isGlobal, code.result());
        return code.result();
    }

    // simply handled operations

    case Operator::Add:
        return HandleSimpleOperation(Bytecode::Add, context, values, expected, actual, code, current, isGlobal);

    case Operator::Subtract:
        return HandleSimpleOperation(Bytecode::Subtract, context, values, expected, actual, code, current, isGlobal);

    case Operator::Multiply:
        return HandleSimpleOperation(Bytecode::Multiply, context, values, expected, actual, code, current, isGlobal);

    case Operator::Divide:
        return HandleSimpleOperation(Bytecode::Divide, context, values, expected, actual, code, current, isGlobal);

    case Operator::Modulo:
        return HandleSimpleOperation(Bytecode::Modulo, context, values, expected, actual, code, current, isGlobal);

    case Operator::Power:
        return HandleSimpleOperation(Bytecode::Power, context, values, expected, actual, code, current, isGlobal);

    case Operator::And:
        return HandleConditionalOperation(Bytecode::And, context, values, expected, actual, code, current, isGlobal);

    case Operator::NAnd:
        return HandleConditionalOperation(Bytecode::NAnd, context, values, expected, actual, code, current, isGlobal);

    case Operator::Or:
        return HandleConditionalOperation(Bytecode::Or, context, values, expected, actual, code, current, isGlobal);

    case Operator::NOr:
        return HandleConditionalOperation(Bytecode::NOr, context, values, expected, actual, code, current, isGlobal);

    case Operator::XOr:
        return HandleConditionalOperation(Bytecode::XOr, context, values, expected, actual, code, current, isGlobal);

    case Operator::Imply:
        return HandleConditionalOperation(Bytecode::Imply, context, values, expected, actual, code, current, isGlobal);

    case Operator::NImply:
        return HandleConditionalOperation(Bytecode::NImply, context, values, expected, actual, code, current, isGlobal);

    case Operator::Equals:
        return HandleConditionalOperation(Bytecode::Equals, context, values, expected, actual, code, current, isGlobal);

    case Operator::NotEqual:
        return HandleConditionalOperation(Bytecode::NotEqual, context, values, expected, actual, code, current, isGlobal);

    case Operator::Greater:
        return HandleConditionalOperation(Bytecode::Greater, context, values, expected, actual, code, current, isGlobal);

    case Operator::GreaterEqual:
        return HandleConditionalOperation(Bytecode::GreaterEqual, context, values, expected, actual, code, current, isGlobal);

    case Operator::Lesser:
        return HandleConditionalOperation(Bytecode::Lesser, context, values, expected, actual, code, current, isGlobal);

    case Operator::LesserEqual:
        return HandleConditionalOperation(Bytecode::LesserEqual, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinAnd:
        return HandleSimpleOperation(Bytecode::BinAnd, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinNAnd:
        return HandleSimpleOperation(Bytecode::BinNAnd, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinImply:
        return HandleSimpleOperation(Bytecode::BinImply, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinNImply:
        return HandleSimpleOperation(Bytecode::BinNImply, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinNOr:
        return HandleSimpleOperation(Bytecode::BinNOr, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinNot:
        return HandleSimpleOperation(Bytecode::BinNot, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinOr:
        return HandleSimpleOperation(Bytecode::BinOr, context, values, expected, actual, code, current, isGlobal);

    case Operator::BinXOr:
        return HandleSimpleOperation(Bytecode::BinXOr, context, values, expected, actual, code, current, isGlobal);

    case Operator::ShiftLeft:
        return HandleSimpleOperation(Bytecode::ShiftLeft, context, values, expected, actual, code, current, isGlobal);

    case Operator::ShiftRight:
        return HandleSimpleOperation(Bytecode::ShiftRight, context, values, expected, actual, code, current, isGlobal);

    default:
        break;
    }

    Unimplemented();
}
