#include <complex>
#include <iostream>

#include <GenerateCode.hpp>

#include "BytecodeInternal.hpp"
#include <Lexing.hpp>
#include <Parser.hpp>

#include "ErrorHandling.hpp"
#include "X86_64Enums.hpp"

/// <summary>
/// Throws an error if the literal can't be assigned to the given type
/// </summary>
/// <param name="literal">A token that contains a literal value</param>
/// <param name="info"></param>
void CheckLiteralMatch(LexerToken* literal, TypeInfo info) {
    if (info.type == nullptr) return;

    DebugError(literal->type != Token::Number, "Token passed to literal match check is not a literal!");

    if (info.pointerLevel > 0) {
        // pointers are always n bit unsigned integers
        // the literal type checks tends to go towards unsigned, then signed then floats at the end
        if (literal->literalType != Type::unsignedInteger)
            Error("Cannot set pointer with a non unsigned integer value!");
    }
    else {
        if (not info.type->isPrimitive)
            Error("Cannot assign literals to complex types!");

        if (info.type->primitiveType == Type::floatingPoint) {
            if (literal->literalType == Type::unsignedInteger)
                literal->_double = static_cast<double>(literal->_unsigned);
            else if (literal->literalType == Type::signedInteger)
                literal->_double = static_cast<double>(literal->_signed);
            else if (literal->literalType != Type::floatingPoint)
                Error("Unsupported literal type for convertion to floating point!");

            // TODO: add float size warnings
            // converting size
            switch (info.type->typeSize) {
            case 8:
                break;
            case 4: {
                auto value = static_cast<float>(literal->_double);
                literal->_unsigned = 0;
                literal->float32 = value;
            }
            break;
            case 2:
                Error("16 bit floats literals not supported!");
            default:
                Error("Unsupported floating point literal size!");
            }
        }
        else if (info.type->primitiveType == Type::unsignedInteger) {
            if (literal->literalType == Type::floatingPoint) {
                if (literal->_double < 0)
                    Warning(
                        "Conversion from negative floating point literal to unsigned integer will result in a radically different value!");
                literal->_unsigned = static_cast<uint64_t>(literal->_double);
            }
            else if (literal->literalType == Type::signedInteger) {
                literal->_unsigned = static_cast<uint64_t>(literal->_signed);
                Warning("Conversion from negative signed to unsigned integer will result in a different value!");
            }
            else if (literal->literalType != Type::unsignedInteger)
                Error("Unsupported literal type for convertion to floating point!");

            // converting size
            switch (info.type->typeSize) {
            case 8:
                break;
            case 4:
                if (literal->_unsigned > 0x00000000FFFFFFFF)
                    Warning("Unsigned literal is too big and will be truncated to 4 bytes!");
                literal->_unsigned &= 0x00000000FFFFFFFF;
                break;
            case 2:
                if (literal->_unsigned > 0x000000000000FFFF)
                    Warning("Unsigned literal is too big and will be truncated to 2 bytes!");
                literal->_unsigned &= 0x000000000000FFFF;
                break;
            case 1:
                if (literal->_unsigned > 0x00000000000000FF)
                    Warning("Unsigned literal is too big and will be truncated to 1 byte!");
                literal->_unsigned &= 0x00000000000000FF;
                break;
            default:
                Error("Unsupported literal size!");
            }
        }
        else if (info.type->primitiveType == Type::signedInteger) {
            if (literal->literalType == Type::floatingPoint) {
                literal->_signed = static_cast<int64_t>(literal->_double);
                Warning("Conversion from floating point literal to integer will result in loss of non integer data!");
            }
            else if (literal->literalType == Type::unsignedInteger) {
                if (literal->_unsigned & 0x8000000000000000)
                    Warning("Conversion from unsigned to signed integer will result in a different value!");
                //literal->_signed = static_cast <int64_t>(literal->_unsigned);
            }
            else if (literal->literalType != Type::signedInteger)
                Error("Unsupported literal type for convertion to floating point!");

            // converting sizes
            switch (info.type->typeSize) {
            case 8:
                break;
            case 4:
                if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFF80000000)
                    Warning("Signed literal is too big into negative and will be truncated to 4 bytes!");
                else if (not(literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7FFFFFFF)
                    Warning("Signed literal is too big and will be truncated to 4 bytes!");
                if (literal->_unsigned & 0x8000000000000000) {
                    literal->_unsigned &= 0x7FFFFFFF;
                    literal->_unsigned ^= 0x80000000;
                }
                else
                    literal->_unsigned &= 0x7FFFFFFF;
                break;
            case 2:
                if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFFFFFF8000)
                    Warning("Signed literal is too big into negative and will be truncated to 2 bytes!");
                else if (not(literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7FFF)
                    Warning("Signed literal is too big and will be truncated to 2 bytes!");
                if (literal->_unsigned & 0x8000000000000000) {
                    literal->_unsigned &= 0x7FFF;
                    literal->_unsigned ^= 0x8000;
                }
                else
                    literal->_unsigned &= 0x7FFF;
                break;
            case 1:
                if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFFFFFFFF80)
                    Warning("Signed literal is too big into negative and will be truncated to 1 byte!");
                else if (not(literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7F)
                    Warning("Signed literal is too big and will be truncated to 1 byte!");
                if (literal->_unsigned & 0x8000000000000000) {
                    literal->_unsigned &= 0x7F;
                    literal->_unsigned ^= 0x80;
                }
                else
                    literal->_unsigned &= 0x7F;
                break;
            default:
                Error("Unsupported literal size!");
            }
        }
        else
            Error("Invalid literal type to convert from!");

        literal->literalType = info.type->primitiveType;
    }
}

BytecodeOperand Dereference(Context& context, BytecodeOperand op, TypeInfo target) {

    DebugError(op.location != Location::var, "Can only dereference variables!");
    DebugError(not context.getVariableObject(op).meta.isReference
           and not context.getVariableObject(op).meta.pointerLevel, "Cannot dereference a value!");
    DebugError(context.getVariableObject(op).type != target.type, "Dereference type mismatch!");
    DebugError(context.getVariableObject(op).meta.isMutable < target.isMutable, "Cannot dereference an immutable value to a mutable value!");
    DebugError(context.getVariableObject(op).meta.pointerLevel == target.pointerLevel
           and not context.getVariableObject(op).meta.isReference, "Cannot dereference without changing pointer meta attributes!");
    DebugError(context.getVariableObject(op).meta.isReference == target.isReference
           and context.getVariableObject(op).meta.pointerLevel != target.pointerLevel + 1, "Target pointer level must be smaller by exactly one!");

    Bytecode code;
    code.AssignType(target);
    code.type = Bytecode::Dereference;
    code.op1(op);
    code.result(context.insertTemporary(target));
    return context.addCodeReturningResult(code);
}

BytecodeOperand GetAddress(Context& context, BytecodeOperand op, TypeInfo target) {

    DebugError(op.location != Location::var, "Can only dereference variables!");
    DebugError(context.getVariableObject(op).type != target.type, "Address type mismatch!");
    DebugError(context.getVariableObject(op).meta.isMutable < target.isMutable, "Cannot get an address of an immutable value as a mutable pointer!");
    DebugError(context.getVariableObject(op).meta.pointerLevel == target.pointerLevel
           and context.getVariableObject(op).meta.isReference, "Cannot get an address without changing pointer meta attributes!");
    DebugError(context.getVariableObject(op).meta.isReference == target.isReference
           and context.getVariableObject(op).meta.pointerLevel + 1 != target.pointerLevel, "Target pointer level must be greater by exactly one!");

    Bytecode code;
    code.AssignType(target);
    code.type = Bytecode::Address;
    code.op1(op);
    code.result(context.insertTemporary(target));
    return context.addCodeReturningResult(code);
}

BytecodeOperand CheckCompatibilityAndConvertReference(Context& context, const TypeInfo expected, const TypeInfo actual, BytecodeOperand op) {
    if (expected.type == nullptr)
        return op;

    if (actual.type == nullptr)
        Error("Expected a typed expression!");

    if (actual.type != expected.type)
        Error("Expected expression of type: " + expected.type->typeName + " but got: " + actual.type->typeName);

    if (not actual.isMutable and expected.isMutable)
        Error("Expected expression result to be mutable!");

    if (actual.pointerLevel != expected.pointerLevel)
        Error("Expected a: " + std::to_string(expected.pointerLevel) + " leveled pointer but got: " + std::to_string(actual.pointerLevel));

    DebugError(not actual.isReference and expected.isReference, "Expected a reference to be generated or error to be thrown earlier!");

    if (actual.isReference and not expected.isReference)
        return Dereference(context, op, expected);

    return op;
}

BytecodeOperand GenerateExpressionRunner(Context& context, std::vector<ParserTreeValue>& values, const TypeInfo expected, bool isGlobal) {
    TypeInfo endType{};
    auto result = GenerateExpressionBytecode(context, values, expected, endType, 0, isGlobal);
    return CheckCompatibilityAndConvertReference(context, expected, endType, result);
}

BytecodeOperand GenerateExpressionBytecode(Context& context, std::vector<ParserTreeValue>& values,
                                           const TypeInfo expected, TypeInfo& actual, uint16_t index,
                                           bool isGlobal, BytecodeOperand passedOperand) {
    // first off let's do a switch to know what is going on
    DebugError(values.empty(), "Empty expression generator value array");

    auto& current = values[index];

    Bytecode code{};
    switch (current.operation) {
    case ParserOperation::Operator:
    case ParserOperation::SingleOperator:
    case ParserOperation::Group:
        return InsertOperatorExpression(context, values, expected, actual, index, isGlobal, passedOperand);

    case ParserOperation::Member: {
        DebugError(passedOperand.location == Location::None, "An operand must be passed to members!");
        DebugError(passedOperand.location != Location::Var, "Operand passed to member must be a variable!");
        DebugError(not context.getVariableObject(passedOperand).meta.isReference, "Member's base variable must be a reference!");

        code.type = Bytecode::Member;
        code.op1(passedOperand);

        code.op2Value.offset = actual.type->getMemberOffsetAndSetType(current.identifier, actual);
        actual.isReference = true;
        code.AssignType(actual);

        code.result(context.insertTemporary(actual));
        return context.addCodeReturningResult(code);
    }

    case ParserOperation::Call:  {
        Unimplemented();
        /*
        bool doNotStore = false;
        if (passedOperand.location == Location::Variable) {
            // it's a method
            code.type = Bytecode::Method;
            BytecodeCall(context, values, type, typeMeta, code, current, isGlobal, true, passedOperand, &doNotStore);
        }
        else {
            // a normal function
            code.type = Bytecode::Function;
            BytecodeCall(context, values, type, typeMeta, code, current, isGlobal, false, {}, &doNotStore);
        }
        context.codes.push_back(code);
        if (doNotStore)
            context.codes.back().result({});
        return code.result();
        */
    }

    case ParserOperation::Syscall: {
        Unimplemented();
        /*
        code.type = Bytecode::Syscall;
        BytecodeCall(context, values, type, typeMeta, code, current, isGlobal);
        code.op3(context.insertTemporary(&types["u" + std::to_string(8 * Options::addressSize)], {}));
        context.codes.push_back(code);
        return code.result();
        */
    }

    case ParserOperation::Literal: {
        // if the literal is the first then we need to assign it to be of the expected type
        // in other cases in needs to match the actual one, so we'll assign it here
        if (actual.type == nullptr)
            actual = expected;

        if (expected.isReference)
            Error("Literals cannot be targets of references!");

        CheckLiteralMatch(current.literal, actual);
        return {Location::Literal, current.literal->_unsigned, actual.type->primitiveType, static_cast<uint8_t>(actual.type->typeSize)};
    }

    case ParserOperation::Variable: {
        // we have a variable, let's get its type
        // if it has something after it let's do that too
        if (isGlobal and not current.isBeingDefined)
            Error("Cannot initialize global variables with other variables!");

        auto var = context.getVariableObject(current.identifier);
        actual.AssignType(var);
        auto varCode = context.getVariable(current.identifier, actual);

        // if we need a reference and don't have one then let's get the address of the variable
        if ((not actual.isReference and expected.isReference) or current.next) {
            actual.isReference = true;
            varCode = GetAddress(context, varCode, actual);
        }

        // if there is nothing after this variable we can return it
        if (not current.next)
            return varCode;

        // welp, now we need to actually handle the next expression
        return GenerateExpressionBytecode(context, values, expected, actual, current.next, isGlobal, varCode);
    }


    case ParserOperation::String:
        // checking if the type is valid for a string
        if (not actual.type->isPrimitive
            or actual.type->typeSize != 1
            or actual.type->primitiveType != Type::unsignedInteger
            or actual.pointerLevel != 1
            or actual.isMutable)
            Error("String literals can only be assigned to immutable 1 byte unsigned integer pointers!");

        return {Location::String, FindString(current.identifier), Type::unsignedInteger, Options::addressSize};


    case ParserOperation::Definition: {

        code.type = Bytecode::Define;
        if (not types.contains(*current.identifier))
            Error("Type " + *current.identifier + " does not exist!");

        code.AssignType(actual.AssignType(&types[*current.identifier], current.typeMeta));

        // TODO: implement global variables
        if (isGlobal)
            Unimplemented();

        code.op1(context.insertVariable(values[current.next].identifier, actual));
        context.codes.push_back(code);

        current.operation = ParserOperation::Operator;
        current.operatorType = Operator::Assign;
        current.isBeingDefined = true;

        if (current.value) {
            GenerateExpressionBytecode(context, values, expected, actual, index, isGlobal, code.op1());
        }
        else {
            // TODO: add default constructor call here
            Unimplemented();
        }

        return code.op1();
    }
    default:
        Error("Unimplemented parser tree node passed to generator!");
    }
    return {};
}

Context GenerateGlobalVariablesBytecode() {
    // global variable bytecode is used for things before main is called
    // they cannot use any variables, though this might be changed in the future

    // at the very least this will take up 1 instruction per variable, so let's prepare it
    Context context;
    context.isConstExpr = true;
    context.codes.reserve(globalVariables.size() * 2);

    // let's go through every variable and parse their codes
    for (auto& n : globalVariables) {
        if (n.second.definition[0].operation != ParserOperation::Definition)
            Error("Expected a variable definition!");

        GenerateExpressionRunner(context, n.second.definition, {n.second.typeObject, n.second.definition[0].typeMeta}, true);
    }

    return std::move(context);
}

void GetTypes(Context& context, std::vector<ParserTreeValue>& values, TypeInfo& result, ParserTreeValue& current) {
    switch (current.operation) {
    case ParserOperation::Operator:
        GetTypes(context, values, result, current.left);
        return;
    case ParserOperation::SingleOperator:
        GetTypes(context, values, result, current.right);
        if (current.operatorType == Operator::Dereference)
            result = {result, -1};
        else if (current.operatorType == Operator::Address)
            result = {result, +1};
        return;
    case ParserOperation::Definition: {
        result = {current.typeMeta, &types[*current.identifier]};
        return;
    }
    case ParserOperation::Variable: {
        auto& var = context.getVariableObject(current.identifier);
        result = {var.meta, var.type};

        if (current.next) GetTypes(context, values, result, current.next);
        return;
    }
    case ParserOperation::Member:
        DebugError(result.type == nullptr, "Type cannot be null when member is called!");
        result.type->getMemberOffsetAndSetType(current.identifier, result);
        result.isReference = true;
        if (current.next) GetTypes(context, values, result, current.next);
        return;
    case ParserOperation::Literal:
        if (result.type == nullptr) {
            switch (current.literal->literalType) {
            case Type::address:
            case Type::unsignedInteger:
                result.type = &types["u" + std::to_string(Options::addressSize * 8)];
                break;
            case Type::signedInteger:
                result.type = &types["i" + std::to_string(Options::addressSize * 8)];
                break;
            case Type::floatingPoint:
                result.type = &types["f" + std::to_string(Options::addressSize * 8)];
                break;
            default:
                break;
            }
        }
        else
            Unimplemented();
        return;
    case ParserOperation::String:
        result = {1, true, false, &types["char"]};
        return;
    case ParserOperation::Group:
        GetTypes(context, values, result, current.value);
        return;
    case ParserOperation::Syscall:
        result.type = &types["i" + std::to_string(Options::addressSize * 8)];
        result = {};
        return;
    default:
        Error("Invalid operation in type negotiation!");
    }
    // TODO: add indexes
}

void GetTypes(Context& context, std::vector<ParserTreeValue>& values, TypeInfo& info, uint16_t index) {
    GetTypes(context, values, info, values[index]);
}

void PushScope(Context& context) {
    context.activeLevels.push_back(context.localVariables.size());
    context.localVariables.emplace_back();
    Bytecode code;
    code.type = Bytecode::BeginScope;
    context.codes.push_back(code);
}

void PopScope(Context& context) {
    context.activeLevels.pop_back();
    Bytecode code;
    code.type = Bytecode::EndScope;
    context.codes.push_back(code);
}

// used for methods
std::string ThisDummy = "this";

struct ConditionalInfo {
    Instruction::Type type = Instruction::None;
    uint64_t loopConditionLabel = 0;
    uint64_t afterLabel = 0;
    uint64_t falseLabel = 0;
};

uint64_t labelCounter = 0;

Context GenerateFunctionBytecode(ParserFunctionMethod& function) {
    std::vector<ConditionalInfo> conditions;
    Context context;
    context.codes.reserve(
        function.instructions.size() + function.parameters.size() + (function.isMethod and not function.isOperator));

    // adding the pointer to object in methods
    if (function.isMethod and function.overloaded == Operator::None) {
        Bytecode code;
        code.opType = function.parentType;
        code.opMeta = TypeMeta();
        code.type = Bytecode::Define;
        code.op1(context.insertVariable(&ThisDummy, {function.parentType, {0, context.isMutable, true}}));
        code.op3({Location::Argument, {0}, Type::none, 0});

        context.codes.push_back(code);
    }

    // adding parameters
    for (uint64_t n = 0; n < function.parameters.size(); n++) {
        Bytecode code;
        code.opType = &types[*function.parameters[n].definition[0].identifier];
        code.opMeta = function.parameters[n].typeMeta();
        code.type = Bytecode::Define;
        code.op1(context.insertVariable(&function.parameters[n].name(), {code.opType, function.parameters[n].typeMeta()}));
        code.op3({ Location::Argument, {n + (function.isMethod and function.overloaded == Operator::None)}, Type::none, 0});
        context.codes.push_back(code);
    }

    // TODO: add bracket-less condition support

    bool wasLastConditional = false;
    bool wasPushAdded = false;
    ConditionalInfo currentCondition;

    if (Options::informationLevel == Options::InformationLevel::full)
        std::cout << "INFO L3: Generating function with code: " << function.getFullName() <<
            " with following instructions:\n";

    for (auto& n : function.instructions) {
        if (wasLastConditional and n.type != Instruction::BeginScope)
            Error("Conditional statement without scope begin right after!");
        switch (n.type) {
        case Instruction::Expression:
            switch (n.valueArray[0].operation) {
            case ParserOperation::Definition: {
                if (not types.contains(*n.valueArray[0].identifier))
                    Error("Type: " + *n.valueArray[0].identifier + " does not exist!")
;                GenerateExpressionRunner(context, n.valueArray, {&types[*n.valueArray[0].identifier], n.valueArray[0].typeMeta});
                break;
            }
            case ParserOperation::Operator: {
                if (n.valueArray[0].left == 0
                    or (n.valueArray[n.valueArray[0].left].operation != ParserOperation::Variable
                        and n.valueArray[n.valueArray[0].left].operation != ParserOperation::SingleOperator))
                    Error("Invalid expression!");

                TypeInfo actual;

                // get the type of the last member of the lvalue used
                GetTypes(context, n.valueArray, actual, n.valueArray[0].left);

                GenerateExpressionRunner(context, n.valueArray, actual);
                break;
            }
            case ParserOperation::Call:
            case ParserOperation::Syscall:
                GenerateExpressionRunner(context, n.valueArray, {}, {});
                break;
            default:
                Unimplemented();
            }
            break;
        case Instruction::Return: {
            Bytecode code;
            code.type = Bytecode::Return;
            if (function.returnType.typeName != nullptr) {
                code.opType = &types[*function.returnType.typeName];
                code.opMeta = function.returnType.type;
                code.op1(GenerateExpressionRunner(context, n.valueArray, {code.opType, code.opMeta}));
            }
            context.codes.push_back(code);
            break;
        }
        case Instruction::BeginScope:
            if (not wasPushAdded) PushScope(context);
            wasPushAdded = false;
            wasLastConditional = false;
            conditions.push_back(currentCondition);
            currentCondition = {};
            break;
        case Instruction::EndScope:
            PopScope(context);
            if (conditions.back().type == Instruction::If) {
                if (&function.instructions.back() != &n) {
                    if ((&n + 1)->type == Instruction::Else or (&n + 1)->type == Instruction::ElseIf) {
                        currentCondition.afterLabel = ++labelCounter;
                        Bytecode code;
                        code.type = Bytecode::Jump;
                        code.op1({Location::Literal, labelCounter, Type::none, 0});
                        context.codes.push_back(code);
                    }
                }
            }
            if (conditions.back().type == Instruction::ElseIf) {
                if (&function.instructions.back() != &n) {
                    if ((&n + 1)->type == Instruction::Else or (&n + 1)->type == Instruction::ElseIf) {
                        currentCondition.afterLabel = conditions.back().afterLabel;
                        conditions.back().afterLabel = 0;
                        Bytecode code;
                        code.type = Bytecode::Jump;
                        code.op1({Location::Literal, currentCondition.afterLabel, Type::none, 0});
                        context.codes.push_back(code);
                    }
                }
            }
            if (conditions.back().loopConditionLabel) {
                Bytecode code;
                code.type = Bytecode::Jump;
                code.op1({Location::Literal, conditions.back().loopConditionLabel, Type::none, 0});
                context.codes.push_back(code);
            }
            if (conditions.back().afterLabel) {
                Bytecode code;
                code.type = Bytecode::Label;
                code.op1({Location::Literal, conditions.back().afterLabel, Type::none, 0});
                context.codes.push_back(code);
            }
            if (conditions.back().falseLabel and conditions.back().falseLabel != conditions.back().afterLabel) {
                Bytecode code;
                code.type = Bytecode::Label;
                code.op1({Location::Literal, conditions.back().falseLabel, Type::none, 0});
                context.codes.push_back(code);
            }
            conditions.pop_back();
            break;
        case Instruction::If:
        case Instruction::ElseIf: {
            wasLastConditional = true;
            Bytecode code;
            code.type = Bytecode::If;
            code.opType = &types["bool"];
            code.op1(GenerateExpressionRunner(context, n.valueArray, {code.opType, {}}));
            code.op2({Location::Literal, ++labelCounter, Type::none, 0});
            currentCondition.type = n.type;
            if (not currentCondition.afterLabel) currentCondition.afterLabel = code.op2Value.ui;
            currentCondition.falseLabel = code.op2Value.ui;
            context.codes.push_back(code);
        }
        break;
        case Instruction::Else:
            wasLastConditional = true;
            break;
        case Instruction::While: {
            {
                Bytecode code;
                code.type = Bytecode::Label;
                code.op1({Location::Literal, ++labelCounter, Type::none, 0});
                context.codes.push_back(code);
                code.type = Bytecode::LoopLabel;
                code.op1({});
                context.codes.push_back(code);
            }
            wasLastConditional = true;
            Bytecode code;
            //code.type = Bytecode::While;
            code.type = Bytecode::If;
            code.opType = &types["bool"];
            code.op1(GenerateExpressionRunner(context, n.valueArray, {code.opType, {}}));
            currentCondition.type = n.type;
            currentCondition.loopConditionLabel = labelCounter;
            code.op2({Location::Literal, ++labelCounter, Type::none, 0});
            currentCondition.falseLabel = code.op2Value.ui;
            context.codes.push_back(code);
        }
        break;
        case Instruction::Do: {
            Error("Internal: do while loop unimplemented!");
            wasLastConditional = true;
            context.addLoopLabel();
            Bytecode code;
            code.type = Bytecode::Do;
            context.codes.push_back(code);
        }
        break;
        case Instruction::Break: {
            // break jumps 2 scope ends away
            // TODO: maybe a break with amount of scopes to break, could be useful
            if (conditions.size() < 2)
                Error("Cannot use break without a scope to break!");
            if (not conditions[conditions.size() - 2].falseLabel)
                Error("Invalid expression to break!");
            Bytecode code;
            code.type = Bytecode::Jump;
            code.op1({Location::Literal, conditions[conditions.size() - 2].falseLabel, Type::none, 0});
            context.codes.push_back(code);
        }
        break;
        case Instruction::Continue: {
            if (conditions.size() < 2)
                Error("Cannot use break without a scope to break!");
            if (not conditions[conditions.size() - 2].loopConditionLabel)
                Error("Invalid expression to break!");
            Bytecode code;
            code.type = Bytecode::Jump;
            code.op1({Location::Literal, conditions[conditions.size() - 2].loopConditionLabel, Type::none, 0});
            context.codes.push_back(code);
        }
        break;
        case Instruction::Switch:
            Error("Switch not implemented!");
        case Instruction::Case:
            Error("Case not implemented!");
        case Instruction::For: {
            Error("Internal: for loop unimplemented!");
            wasLastConditional = true;
            wasPushAdded = true;

            Bytecode code;

            // for is split into 3 codes so that they can be easily identified
            // initial statement after the scope was pushed
            PushScope(context);
            // this one does not need a bytecode
            TypeInfo actual = {code.opType, code.opMeta};
            GetTypes(context, n.valueArray, actual, n.expression1Index);
            GenerateExpressionRunner(context, n.valueArray, actual, n.expression1Index);

            // now the after statement that needs a jump label, not sure if it will be before or after condition yet
            context.addLoopLabel();
            code.type = Bytecode::ForStatement;
            GetTypes(context, n.valueArray, actual, n.expression2Index);
            code.op1(GenerateExpressionRunner(context, n.valueArray, actual, n.expression3Index));
            context.codes.push_back(code);

            // conditional statement needs bool type and another jump label for returning here after the after statement
            context.addLoopLabel();
            code.type = Bytecode::ForCondition;
            code.opType = &types["bool"];
            code.opMeta = {};
            code.op1(GenerateExpressionRunner(context, n.valueArray, {code.opType, {}}, n.expression2Index));
            context.codes.push_back(code);
        }
        break;
        default:
            Error("Unhandled instruction type!");
            break;
        }
    }

    // checking if all added brackets were terminated correctly just to be sure
    if (context.activeLevels.size() != 1)
        Error("Function/method scope level invalid!");

    return context;
}

// methods

OperandValue::OperandValue(uint64_t val) {
    ui = val;
}

OperandValue::OperandValue(VariableLocation val) {
    variable = val;
}

BytecodeOperand Bytecode::op1() const {
    return {op1Location, op1Value, op1LiteralType, op1LiteralSize};
}

BytecodeOperand Bytecode::op1(BytecodeOperand op) {
    op1Value = op.value;
    op1Location = op.location;
    op1LiteralType = op.literalType;
    op1LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::op2() const {
    return {op2Location, op2Value, op2LiteralType, op2LiteralSize};
}

BytecodeOperand Bytecode::op2(BytecodeOperand op) {
    op2Value = op.value;
    op2Location = op.location;
    op2LiteralType = op.literalType;
    op2LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::op3() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::op3(BytecodeOperand op) {
    op3Value = op.value;
    op3Location = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::result() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::result(BytecodeOperand op) {
    op3Value = op.value;
    op3Location = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.size;
    return op;
}

void Bytecode::AssignType(VariableObject& variable) {
    opType = variable.type;
    opMeta = variable.meta;
}

void Bytecode::AssignType(TypeInfo info) {
    opType = info.type;
    opMeta = info.meta();
}

void Bytecode::AssignType(TypeObject* typeObject, TypeMeta meta) {
    opType = typeObject;
    opMeta = meta;
}

BytecodeOperand::BytecodeOperand(Location::Type location, OperandValue value, Type::TypeEnum literalType,
                                 uint8_t literalSize) {
    this->location = location;
    this->value = value;
    this->literalType = literalType;
    this->size = literalSize;
}

uint8_t VariableObject::variableSize() {
    return meta.variableSize(*type);
}

void VariableObject::use(uint32_t index) {
    if (uses == 0) {
        firstUse = index;
        lastUse = index;
        uses = 1;
    }
    else {
        ++uses;
        lastUse = index;
    }
}


Context Context::current() const {
    Context newContext;
    newContext.localVariables = localVariables;
    newContext.temporaries = temporaries;
    newContext.isConstExpr = isConstExpr;
    newContext.isMutable = isMutable;
    newContext.activeLevels = activeLevels;
    return newContext;
}

void Context::merge(Context& context) {
    localVariables = std::move(context.localVariables);
    temporaries = std::move(context.temporaries);
    // activeLevels = std::move(context.activeLevels)
    const auto start = codes.size();
    codes.resize(codes.size() + context.codes.size());
    for (size_t n = 0; n < context.codes.size(); n++) {
        codes[start + n] = context.codes[n];
    }
}

BytecodeOperand Context::addCodeReturningResult(Bytecode code) {
    codes.push_back(code);
    return code.result();
}

BytecodeOperand Context::insertVariable(std::string* identifier, TypeInfo info) {
    VariableLocation location;
    location.type = VariableLocation::Local;
    location.level = localVariables.size() - 1;
    location.number = localVariables[location.level].size();
    localVariables[location.level].emplace_back(info.type, identifier, info.meta());
    return {Location::Variable, {location}, info.type->primitiveType, info.variableSize(*info.type)};
}

BytecodeOperand Context::insertTemporary(const TypeInfo& info) {
    return insertTemporary(info.type, info.meta());
}

BytecodeOperand Context::insertTemporary(TypeObject* type, TypeMeta meta) {
    VariableLocation location;
    location.type = VariableLocation::Temporary;
    location.number = temporaries.size();
    temporaries.emplace_back(type, nullptr, meta);
    return {Location::Variable, {location}, type->primitiveType, meta.variableSize(*type)};
}

BytecodeOperand Context::getVariable(std::string* identifier, TypeInfo typeInfo) {
    return getVariable(identifier, typeInfo.type, typeInfo.meta());
}

BytecodeOperand Context::getVariable(std::string* identifier, TypeObject* type, TypeMeta meta) {
    // first let's go through local variables from top and back
    for (int64_t n = activeLevels.size() - 1; n >= 0; n--) {
        for (int64_t m = localVariables[activeLevels[n]].size() - 1; m >= 0 and localVariables[activeLevels[n]].size()
             != 0; m--) {
            auto& current = localVariables[activeLevels[n]][m];
            if (current.identifier == identifier or *current.identifier == *identifier) {
                if (current.type != type)
                    Error("Type mismatch in last local variable match!");
                if (current.meta.pointerLevel != meta.pointerLevel)
                    Error("Pointer level mismatch in last local variable!");
                if (current.meta.isMutable < meta.isMutable)
                    Error("Cannot use non-mutable variables in mutable context! (might need different conditions)");
                if (not current.meta.isReference and meta.isReference) {
                    Bytecode code;
                    code.type = Bytecode::Address;
                    code.op1({
                        Location::Variable, VariableLocation(VariableLocation::Local, n, m), type->primitiveType,
                        meta.variableSize(*type)
                    });
                    code.result(insertTemporary(type, meta));
                    code.opMeta = meta;
                    code.opType = type;
                    codes.push_back(code);
                    return code.result();
                }
                if (current.meta.isReference and not meta.isReference) {
                    // in that case the value of the referred is needed
                    Bytecode code;
                    code.type = Bytecode::Dereference;
                    code.op1({
                        Location::Variable, VariableLocation(VariableLocation::Local, n, m), type->primitiveType,
                        meta.variableSize(*type)
                    });
                    code.result(insertTemporary(type, meta));
                    code.opMeta = meta;
                    code.opType = type;
                    codes.push_back(code);
                    return code.result();
                }
                return {
                    Location::Variable, VariableLocation(VariableLocation::Local, activeLevels[n], m),
                    localVariables[activeLevels[n]][m].type->primitiveType,
                    localVariables[activeLevels[n]][m].variableSize()
                };
            }
        }
    }

    // now global variables
    for (uint64_t n = 0; n < globalVariableObjects.size(); n++) {
        auto& current = globalVariableObjects[n];
        if (current.identifier == identifier or *current.identifier == *identifier) {
            if (current.type != type)
                Error("Type mismatch in last global variable match!");
            if (current.meta.pointerLevel != meta.pointerLevel)
                Error("Pointer level mismatch in last global variable!");
            if (current.meta.isMutable < meta.isMutable)
                Error("Cannot use non-mutable variables in mutable context! (might need different contitions)");
            if (not current.meta.isReference and meta.isReference) {
                Bytecode code;
                code.type = Bytecode::Address;
                code.op1({
                    Location::Variable, VariableLocation(VariableLocation::Global, 0, n), type->primitiveType,
                    meta.variableSize(*type)
                });
                code.result(insertTemporary(type, meta));
                code.opMeta = meta;
                code.opType = type;
                codes.push_back(code);
                return code.result();
            }
            if (current.meta.isReference and not meta.isReference) {
                // in that case the value of the referred is needed
                Bytecode code;
                code.type = Bytecode::Dereference;
                code.op1({
                    Location::Variable, VariableLocation(VariableLocation::Global, 0, n), type->primitiveType,
                    meta.variableSize(*type)
                });
                code.result(insertTemporary(type, meta));
                code.opMeta = meta;
                code.opType = type;
                codes.push_back(code);
                return code.result();
            }
            return {
                Location::Variable, VariableLocation(VariableLocation::Global, 0, n), type->primitiveType,
                meta.variableSize(*type)
            };
        }
    }

    Error("Variable \"" + *identifier + "\" not found!");
    return {};
}


VariableObject& Context::getVariableObject(const std::string* identifier) {
    // first let's go through local variables from top and back
    for (int64_t n = activeLevels.size() - 1; n >= 0; n--) {
        for (int64_t m = localVariables[activeLevels[n]].size() - 1; m >= 0 and localVariables[activeLevels[n]].size()
             != 0; m--) {
            auto& current = localVariables[activeLevels[n]][m];
            if (current.identifier == identifier or *current.identifier == *identifier) {
                return current;
            }
        }
    }

    // now global variables
    for (auto& current : globalVariableObjects) {
        if (current.identifier == identifier or *current.identifier == *identifier) {
            return current;
        }
    }

    throw CompilerException("Variable \"" + *identifier + "\" not found!");
}

VariableObject& Context::getVariableObject(BytecodeOperand operand) {
    if (operand.location != Location::Variable)
        Error("Cannot get a variable from non-variable location!");
    switch (operand.value.variable.type) {
    case VariableLocation::None:
        Error("Invalid variable type in get!");
    case VariableLocation::Global:
        return globalVariableObjects[operand.value.variable.number];
    case VariableLocation::Local:
        return localVariables[operand.value.variable.level][operand.value.variable.number];
    case VariableLocation::Temporary:
        return temporaries[operand.value.variable.number];
    default:
        Error("Somehow could not find a variable object!");
        return globalVariableObjects[0];
    }
}

void Context::addLoopLabel() {
    Bytecode code;
    code.type = Bytecode::LoopLabel;
    codes.push_back(code);
}

int64_t TypeObject::getMemberOffsetAndSetType(std::string* identifier, TypeInfo& info) {
    for (auto& n : members) {
        if (n.name() == *identifier) {
            info = {n.typeObject, n.typeMeta()};
            return n.offset;
        }
    }
    Error("Member \"" + *identifier + "\" is not a member of type \"" + typeName + "\"!");
    return 0;
}

ParserFunctionMethod& TypeObject::findMethod(std::string* identifier) {
    for (auto& n : methods) {
        if (*n.name == *identifier) return n;
    }

    Error("Could not find a method with identifier: " + *identifier + " in type: " + typeName + "!");
    return methods.front();
}

TypeInfo TypeInfo::AssignType(VariableObject& variable) {
    TypeMeta(variable.meta);
    type = variable.type;
    return *this;
}

TypeInfo TypeInfo::AssignType(TypeInfo& info) {
    return *this = info;
}

TypeInfo TypeInfo::AssignType(TypeObject* typeObject, TypeMeta meta) {
    return *this = {typeObject, meta};
}
