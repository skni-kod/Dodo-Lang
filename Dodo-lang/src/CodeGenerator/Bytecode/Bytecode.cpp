#include <GenerateCode.hpp>

#include "BytecodeInternal.hpp"
#include <iostream>
#include <Lexing.hpp>
#include <Parser.hpp>

// checks literal types, matches then with sizes and issues errors and warnings
void CheckLiteralMatch(LexerToken* literal, TypeObject* type, TypeMeta typeMeta) {
    if (type == nullptr) return;

    if (typeMeta.pointerLevel > 0) {
        // pointers are always n bit unsigned integers
        if (literal->literalType != Type::unsignedInteger) CodeGeneratorError("Cannot set pointer with a non unsigned integer value!");
    }
    else {
        if (not type->isPrimitive) CodeGeneratorError("Cannot assign literals to non primitives if the operator is not overloaded!");

        if (type->primitiveType == Type::floatingPoint) {
            if (literal->literalType == Type::unsignedInteger) literal->_double = static_cast <double>(literal->_unsigned);
            else if (literal->literalType == Type::signedInteger) literal->_double = static_cast <double>(literal->_signed);
            else if (literal->literalType != Type::floatingPoint) CodeGeneratorError("Unsupported literal type for convertion to floating point!");

            // TODO: add float size warnings
            // converting size
            switch (type->typeSize) {
                case 8:
                    break;
                case 4: {
                    float value = static_cast <float>(literal->_double);
                    literal->_unsigned = 0;
                    literal->float32 = value;
                }
                break;
                case 2:
                    CodeGeneratorError("16 bit floats literals not supported!");
                break;
                default:
                    CodeGeneratorError("Unsupported floating point literal size!");
            }
        }
        else if (type->primitiveType == Type::unsignedInteger) {
            if (literal->literalType == Type::floatingPoint) {
                if (literal->_double < 0) Warning("Conversion from negative floating point literal to unsigned integer will result in a completely different value!");
                literal->_unsigned = static_cast <uint64_t>(literal->_double);
            }
            else if (literal->literalType == Type::signedInteger) {
                literal->_unsigned = static_cast <uint64_t>(literal->_signed);
                Warning("Conversion from negative signed to unsigned integer will result in a different value!");
            }
            else if (literal->literalType != Type::unsignedInteger) CodeGeneratorError("Unsupported literal type for convertion to floating point!");

            // converting size
            switch (type->typeSize) {
                case 8:
                    break;
                case 4:
                    if (literal->_unsigned > 0x00000000FFFFFFFF) Warning("Unsigned literal is too big and will be truncated to 4 bytes!");
                    literal->_unsigned &= 0x00000000FFFFFFFF;
                    break;
                case 2:
                    if (literal->_unsigned > 0x000000000000FFFF) Warning("Unsigned literal is too big and will be truncated to 2 bytes!");
                    literal->_unsigned &= 0x000000000000FFFF;
                    break;
                case 1:
                    if (literal->_unsigned > 0x00000000000000FF) Warning("Unsigned literal is too big and will be truncated to 1 byte!");
                    literal->_unsigned &= 0x00000000000000FF;
                    break;
                default:
                    CodeGeneratorError("Unsupported literal size!");
            }
        }
        else if (type->primitiveType == Type::signedInteger) {
            if (literal->literalType == Type::floatingPoint) {
                literal->_signed = static_cast <int64_t>(literal->_double);
                Warning("Conversion from floating point literal to integer will result in loss of non integer data!");
            }
            else if (literal->literalType == Type::unsignedInteger) {
                if (literal->_unsigned & 0x8000000000000000) Warning("Conversion from unsigned to signed integer will result in a different value!");
                //literal->_signed = static_cast <int64_t>(literal->_unsigned);
            }
            else if (literal->literalType != Type::signedInteger) CodeGeneratorError("Unsupported literal type for convertion to floating point!");

            // converting sizes
            switch (type->typeSize) {
                case 8:
                    break;
                case 4:
                    if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFF80000000) Warning("Signed literal is too big into negative and will be truncated to 4 bytes!");
                    else if (not (literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7FFFFFFF) Warning("Signed literal is too big and will be truncated to 4 bytes!");
                    if (literal->_unsigned & 0x8000000000000000) {
                        literal->_unsigned &= 0x7FFFFFFF;
                        literal->_unsigned ^= 0x80000000;
                    }
                    else literal->_unsigned &= 0x7FFFFFFF;
                    break;
                case 2:
                    if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFFFFFF8000) Warning("Signed literal is too big into negative and will be truncated to 2 bytes!");
                    else if (not (literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7FFF) Warning("Signed literal is too big and will be truncated to 2 bytes!");
                    if (literal->_unsigned & 0x8000000000000000) {
                        literal->_unsigned &= 0x7FFF;
                        literal->_unsigned ^= 0x8000;
                    }
                    else literal->_unsigned &= 0x7FFF;
                    break;
                case 1:
                    if (literal->_unsigned & 0x8000000000000000 and literal->_unsigned < 0xFFFFFFFFFFFFFF80) Warning("Signed literal is too big into negative and will be truncated to 1 byte!");
                    else if (not (literal->_unsigned & 0x8000000000000000) and literal->_unsigned > 0x7F) Warning("Signed literal is too big and will be truncated to 1 byte!");
                    if (literal->_unsigned & 0x8000000000000000) {
                        literal->_unsigned &= 0x7F;
                        literal->_unsigned ^= 0x80;
                    }
                    else literal->_unsigned &= 0x7F;
                    break;
                default:
                    CodeGeneratorError("Unsupported literal size!");
            }
        }
        else CodeGeneratorError("Invalid literal type to convert from!");

        literal->literalType = type->primitiveType;
    }
}

// pass reference type
BytecodeOperand Dereference(BytecodeContext& context, BytecodeOperand op, TypeObject* type, TypeMeta meta) {
    if (not meta.isReference) CodeGeneratorError("Cannot dereference a non-reference!");
    if (op.location == Location::Variable) {
        auto& var = context.getVariableObject(op);
        if (var.meta == meta.noReference()) return op;

    }
    Bytecode code;
    code.type = Bytecode::Dereference;
    code.op1(op);
    code.op3(context.insertTemporary(type, meta));
    context.codes.push_back(code);
    return code.op3();
}

// this function takes a vector of existing bytecode, a tree value table, expression type and starting index
// it recursively creates bytecode instructions needed to make the expression work
// it is the most important function of the entire bytecode generator, probably
// also references are a mess
BytecodeOperand GenerateExpressionBytecode(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal, BytecodeOperand passedOperand) {
    // first off let's do a switch to know what is going on
    auto& current = values[index];

    if (typeMeta.isReference) {
        // in that case if it's an argument the address needs to be extracted
        // if it's used anywhere else it needs to be handled as a pointer without being seen as one
        // it's going to be annoying
    }

    Bytecode code;
    code.opType = type;
    code.opTypeMeta = typeMeta;
    switch (current.operation) {
        case ParserOperation::Operator:
        case ParserOperation::SingleOperator:
        case ParserOperation::Group:
            return InsertOperatorExpression(context, values, type, typeMeta, index, isGlobal);
        case ParserOperation::Member:
            // TODO: add method support here
            if (typeMeta.pointerLevel or not typeMeta.isReference) CodeGeneratorError("Cannot use non-reference or pointer types for members!");
            code.type = Bytecode::Member;
            code.op1(passedOperand);
            code.op2Value.offset = type->getMemberOffsetAndType(current.identifier, type, typeMeta);
            code.opType = type;
            code.opTypeMeta = typeMeta;
            code.opTypeMeta.isReference = true;
            code.result(context.insertTemporary(type, typeMeta));
            context.codes.push_back(code);
            return code.result();
        case ParserOperation::Call:
            if (passedOperand.location == Location::Variable) {
                // it's a method
                code.type = Bytecode::Method;
                BytecodeCall(context, values, type, typeMeta, code, current, isGlobal, true, passedOperand);
            }
            else {
                // a normal function
                code.type = Bytecode::Function;
                BytecodeCall(context, values, type, typeMeta, code, current, isGlobal);

            }
            context.codes.push_back(code);
            return code.result();
        break;
        case ParserOperation::Syscall:
            code.type = Bytecode::Syscall;
            BytecodeCall(context, values, type, typeMeta, code, current, isGlobal);
            if (Options::addressSize == 8) code.op3(context.insertTemporary(&types["u64"], {}));
            else if (Options::addressSize == 4) code.op3(context.insertTemporary(&types["u32"], {}));
            else CodeGeneratorError("Unsupported address size for syscall return!");
            context.codes.push_back(code);
            return code.result();
        case ParserOperation::Literal:
            CheckLiteralMatch(current.literal, type, typeMeta);
            if (type != nullptr) return {Location::Literal, {current.literal->_unsigned}, type->primitiveType, static_cast<uint8_t>(type->typeSize)};
            else return {Location::Literal, {current.literal->_unsigned}, current.literal->literalType, Options::addressSize};

        case ParserOperation::Variable: {
            // TODO: rewrite this mess if it acts up so that it accepts only the type that is needed
            if (isGlobal and not current.isBeingDefined) CodeGeneratorError("Cannot initialize global variables with other variables!");
            bool dereferenceBack = false;
            VariableObject var = context.getVariableObject(current.identifier);
            if (not current.isBeingDefined and current.next == 0 and (type != var.type or typeMeta.pointerLevel != var.meta.pointerLevel)) CodeGeneratorError("Variable type mismatch!");
            // if a value is needed but a reference is needed for access it needs to be dereferenced back
            if (current.next and not typeMeta.isReference) dereferenceBack = true;
            type = var.type;
            if (typeMeta.isReference != var.meta.isReference) var.meta.isReference = typeMeta.isReference;
            typeMeta = var.meta;
            if (current.next) {
                if (not typeMeta.isReference) {
                    // getting the reference since it isn't one
                    typeMeta.isReference = true;
                    code.op1(context.getVariable(current.identifier, type, typeMeta));

                    auto val = GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, code.op1());
                    context.isGeneratingReference = false;

                    if (dereferenceBack) val = Dereference(context, val, type, typeMeta);
                    return val;
                }
                auto val = GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, context.getVariable(current.identifier, type, typeMeta));
                context.isGeneratingReference = false;
                if (dereferenceBack) val = Dereference(context, val, type, typeMeta);
                return val;
            }
            return context.getVariable(current.identifier, type, typeMeta);
        }
        case ParserOperation::String:
            // checking if the type is valid for a string
            if (not type->isPrimitive or type->typeSize != 1
                or type->primitiveType != Type::unsignedInteger or typeMeta.pointerLevel != 1)
                    CodeGeneratorError("String literals can only be assigned to 1 byte unsigned integer pointers!");

            passedStrings.emplace_back(current.identifier);
            return {Location::String, {passedStrings.size() - 1}};
        case ParserOperation::Definition:
            code.type = Bytecode::Define;
            if (not types.contains(*current.identifier)) CodeGeneratorError("Type " + *current.identifier + " does not exist!");
            type = &types[*current.identifier];
            if (isGlobal) {
                // if it's global give it a number and push back a pointer
                code.op1({Location::Variable, {{VariableLocation::Global, 0, globalVariableObjects.size()}}});
                globalVariableObjects.emplace_back(type, values[current.next].identifier, typeMeta.reference());
            }
            else {
                code.op1(context.insertVariable(values[current.next].identifier, type, typeMeta));
            }
            values[current.next].isBeingDefined = true;

            if (current.value) {
                current.operation = ParserOperation::Operator;
                current.operatorType = Operator::Assign;
                context.codes.push_back(code);
                
                GenerateExpressionBytecode(context, values, type, typeMeta, index, isGlobal);

                return {};
            }
            // TODO: add user definable default values for primitive types!
            if (type->isPrimitive) CodeGeneratorError("Primitive default value support incomplete!");
            CodeGeneratorError("Default value support incomplete");
        default:
            CodeGeneratorError("Invalid parsed operation!");
    }
    return {};
}

BytecodeContext GenerateGlobalVariablesBytecode() {
    // global variable bytecode is used for things before main is called
    // they cannot use any variables, though this might be changed in the future

    // at the very least this will take up 1 instruction per variable, so let's prepare it
    BytecodeContext context;
    context.isConstExpr = true;
    context.codes.reserve(globalVariables.size());

    // let's go through every variable and parse their codes
    for (auto& n : globalVariables) {
        if (n.second.definition[0].operation != ParserOperation::Definition) CodeGeneratorError("Expected a variable definition!");
        GenerateExpressionBytecode(context, n.second.definition, n.second.typeObject, n.second.definition[0].typeMeta, 0, true);
    }

    return std::move(context);
}

void GetTypes(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject*& type, TypeMeta& typeMeta, uint16_t index) {
    auto& current = values[index];
    switch (current.operation) {
        case ParserOperation::Operator:
        case ParserOperation::SingleOperator:
            GetTypes(context, values, type, typeMeta, current.left);
            return;
        case ParserOperation::Definition: {
            type = &types[*current.identifier];
            typeMeta = current.typeMeta;
            return;
        }
        case ParserOperation::Variable: {
            auto& var = context.getVariableObject(current.identifier);
            type = var.type;
            typeMeta = var.meta;

            if (current.next) GetTypes(context, values, type, typeMeta, current.next);
            return;
        }
        case ParserOperation::Member:
            type->getMemberOffsetAndType(current.identifier, type, typeMeta);
            typeMeta.isReference = true;
            if (current.next) GetTypes(context, values, type, typeMeta, current.next);
            return;
        case ParserOperation::Literal:
            return;
        case ParserOperation::String:
            type = &types["char"];
            typeMeta = {1, true, false};
            return;
        default:
            CodeGeneratorError("Invalid operation in type negotiation!");
    }
    // TODO: add indexes
}

void PushScope(BytecodeContext& context) {
    context.activeLevels.push_back(context.localVariables.size());
    context.localVariables.emplace_back();
    Bytecode code;
    code.type = Bytecode::BeginScope;
    context.codes.push_back(code);
}

void PopScope(BytecodeContext& context) {
    context.activeLevels.pop_back();
    Bytecode code;
    code.type = Bytecode::EndScope;
    context.codes.push_back(code);
}

// used for methods
std::string ThisDummy = "this";

BytecodeContext GenerateFunctionBytecode(ParserFunctionMethod& function) {

    BytecodeContext context;
    context.codes.reserve(function.instructions.size() + function.parameters.size() + function.isMethod);

    // adding the pointer to object in methods
    if (function.isMethod and function.overloaded == Operator::None) {
        Bytecode code;
        code.opType = function.parentType;
        code.opTypeMeta = TypeMeta();
        code.type = Bytecode::Define;
        code.op1(context.insertVariable(&ThisDummy, function.parentType, {0, context.isMutable, true}));
        code.op3({Location::Argument, {0}});
        context.codes.push_back(code);
    }

    // adding parameters
    for (uint64_t n = 0; n < function.parameters.size(); n++) {
        Bytecode code;
        code.opType = &types[*function.parameters[n].definition[0].identifier];
        code.opTypeMeta = function.parameters[n].typeMeta();
        code.type = Bytecode::Define;
        code.op1(context.insertVariable(&function.parameters[n].name(), code.opType, function.parameters[n].typeMeta()));
        code.op3({Location::Argument, {n + (function.isMethod and function.overloaded == Operator::None)}});
        context.codes.push_back(code);
    }

    // TODO: add bracket-less condition support

    bool wasLastConditional = false;
    bool wasPushAdded = false;

    for (auto& n : function.instructions) {
        if (wasLastConditional and n.type != Instruction::BeginScope) CodeGeneratorError("Conditional statement without scope begin right after!");
        switch (n.type) {
            case Instruction::Expression:
                switch (n.valueArray[0].operation) {
                    case ParserOperation::Definition:
                        GenerateExpressionBytecode(context, n.valueArray, n.valueArray[0].definitionType, n.valueArray[0].typeMeta);
                        break;
                    case ParserOperation::Operator:
                    {
                        if (not n.valueArray[0].left or n.valueArray[n.valueArray[0].left].operation != ParserOperation::Variable) CodeGeneratorError("Invalid expression!");

                        TypeObject* type;
                        TypeMeta typeMeta;

                        // get the type of the last member of the lvalue used
                        GetTypes(context, n.valueArray, type, typeMeta, n.valueArray[0].left);

                        GenerateExpressionBytecode(context, n.valueArray, type, typeMeta);
                        break;
                    }
                    case ParserOperation::Call:
                    case ParserOperation::Syscall:
                        GenerateExpressionBytecode(context, n.valueArray, nullptr, {});
                    break;
                    default:
                        CodeGeneratorError("Unhandled expression type!");
                }
                break;
            case Instruction::Return:
            {
                Bytecode code;
                code.type = Bytecode::Return;
                code.opType = &types[*function.returnType.typeName];
                code.opTypeMeta = function.returnType.type;
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, code.opTypeMeta));
                context.codes.push_back(code);
                break;
            }
            case Instruction::BeginScope:
                if (not wasPushAdded) PushScope(context);
                wasPushAdded = false;
                wasLastConditional = false;
                break;
            case Instruction::EndScope:
                PopScope(context);
                break;
            case Instruction::If:
            {
                wasLastConditional = true;
                Bytecode code;
                code.type = Bytecode::If;
                code.opType = &types["bool"];
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, {}));
                context.codes.push_back(code);
            }
            break;
            case Instruction::ElseIf:
            {
                wasLastConditional = true;
                Bytecode code;
                code.type = Bytecode::ElseIf;
                code.opType = &types["bool"];
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, {}));
                context.codes.push_back(code);
            }
            break;
            case Instruction::Else:
            {
                wasLastConditional = true;
                Bytecode code;
                code.type = Bytecode::Else;
                context.codes.push_back(code);
            }
            break;
            case Instruction::While:
            {
                // TODO: what about do while loop?
                wasLastConditional = true;
                context.addLoopLabel();
                Bytecode code;
                code.type = Bytecode::While;
                code.opType = &types["bool"];
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, {}));
                context.codes.push_back(code);
            }
            break;
            case Instruction::Do:
            {
                wasLastConditional = true;
                context.addLoopLabel();
                Bytecode code;
                code.type = Bytecode::Do;
                context.codes.push_back(code);
            }
            break;
            case Instruction::Break:
            {
                // break jumps 2 scope ends away
                // TODO: maybe a break with amount of scopes to break, could be useful
                if (context.activeLevels.size() < 3) CodeGeneratorError("Cannot use break without a scope to break!");
                Bytecode code;
                code.type = Bytecode::Break;
                context.codes.push_back(code);
            }
            break;
            case Instruction::Continue:
            {
                // TODO: how to make this condition?
                if (context.activeLevels.size() < 3) CodeGeneratorError("Cannot use continue without a scope to continue!");
                Bytecode code;
                code.type = Bytecode::Continue;
                context.codes.push_back(code);
            }
            break;
            case Instruction::Switch:
                CodeGeneratorError("Switch not implemented!");
            case Instruction::Case:
                CodeGeneratorError("Case not implemented!");
            case Instruction::For:
            {
                wasLastConditional = true;
                wasPushAdded = true;

                Bytecode code;

                // for is split into 3 codes so that they can be easily identified
                // initial statement after the scope was pushed
                PushScope(context);
                // this one does not need a bytecode
                GetTypes(context, n.valueArray, code.opType, code.opTypeMeta, n.expression1Index);
                GenerateExpressionBytecode(context, n.valueArray, code.opType, {}, n.expression1Index);

                // now the after statement that needs a jump label, not sure if it will be before or after condition yet
                context.addLoopLabel();
                code.type = Bytecode::ForStatement;
                GetTypes(context, n.valueArray, code.opType, code.opTypeMeta, n.expression2Index);
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, {}, n.expression3Index));
                context.codes.push_back(code);

                // conditional statement needs bool type and another jump label for returning here after the after statement
                context.addLoopLabel();
                code.type = Bytecode::ForCondition;
                code.opType = &types["bool"];
                code.opTypeMeta = {};
                code.op1(GenerateExpressionBytecode(context, n.valueArray, code.opType, {}, n.expression2Index));
                context.codes.push_back(code);
            }
            break;
            default:
                CodeGeneratorError("Unhandled instruction type!");
                break;
        }
    }

    // checking if all added brackets were terminated correctly just to be sure
    if (context.activeLevels.size() != 1) CodeGeneratorError("Function/method scope level invalid!");

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
    op1Value       = op.value;
    op1Location    = op.location;
    op1LiteralType = op.literalType;
    op1LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::op2() const {
    return {op2Location, op2Value, op2LiteralType, op2LiteralSize};
}

BytecodeOperand Bytecode::op2(BytecodeOperand op) {
    op2Value       = op.value;
    op2Location    = op.location;
    op2LiteralType = op.literalType;
    op2LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::op3() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::op3(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.size;
    return op;
}

BytecodeOperand Bytecode::result() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::result(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.size;
    return op;
}

BytecodeOperand::BytecodeOperand(Location::Type location, OperandValue value, Type::TypeEnum literalType, uint8_t literalSize) {
    this->location = location;
    this->value = value;
    this->literalType = literalType;
    this->size = literalSize;
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

BytecodeContext BytecodeContext::current() const {
    BytecodeContext newContext;
    newContext.localVariables = localVariables;
    newContext.temporaries = temporaries;
    newContext.isConstExpr = isConstExpr;
    newContext.isMutable = isMutable;
    newContext.activeLevels = activeLevels;
    return newContext;
}

void BytecodeContext::merge(BytecodeContext& context) {
    localVariables = std::move(context.localVariables);
    temporaries = std::move(context.temporaries);
    // activeLevels = std::move(context.activeLevels)
    const auto start = codes.size();
    codes.resize(codes.size() + context.codes.size());
    for (size_t n = 0; n < context.codes.size(); n++) {
        codes[start + n] = context.codes[n];
    }
}

BytecodeOperand BytecodeContext::insertVariable(std::string* identifier, TypeObject* type, TypeMeta meta) {
    VariableLocation location;
    location.type = VariableLocation::Local;
    location.level = localVariables.size() - 1;
    location.number = localVariables[location.level].size();
    localVariables[location.level].emplace_back(type, identifier, meta);
    return {Location::Variable, {location}};
}

BytecodeOperand BytecodeContext::insertTemporary(TypeObject* type, TypeMeta meta) {
    VariableLocation location;
    location.type = VariableLocation::Temporary;
    location.number = temporaries.size();
    temporaries.emplace_back(type, nullptr, meta);
    return {Location::Variable, {location}};

}

BytecodeOperand BytecodeContext::getVariable(std::string* identifier, TypeObject* type, TypeMeta meta) {
    // first let's go through local variables from top and back
    for (int64_t n = activeLevels.size() - 1; n >= 0; n--) {
        for (int64_t m = localVariables[activeLevels[n]].size() - 1; m >= 0 and localVariables[activeLevels[n]].size() != 0; m--) {
            auto& current = localVariables[activeLevels[n]][m];
            if (current.identifier == identifier or *current.identifier == *identifier) {
                if (current.type != type) CodeGeneratorError("Type mismatch in last local variable match!");
                if (current.meta.pointerLevel != meta.pointerLevel) CodeGeneratorError("Pointer level mismatch in last local variable!");
                if (current.meta.isMutable < meta.isMutable) CodeGeneratorError("Cannot use non-mutable variables in mutable context! (might need different contitions)");
                if (not current.meta.isReference and meta.isReference) {
                    Bytecode code;
                    code.type = Bytecode::Address;
                    code.op1({Location::Variable, VariableLocation(VariableLocation::Local, n, m)});
                    code.result(insertTemporary(type, meta));
                    code.opTypeMeta = meta;
                    code.opType = type;
                    codes.push_back(code);
                    return code.result();
                }
                if (current.meta.isReference and not meta.isReference) {
                    // in that case the value of the referred is needed
                    Bytecode code;
                    code.type = Bytecode::Dereference;
                    code.op1({Location::Variable, VariableLocation(VariableLocation::Local, n, m)});
                    code.result(insertTemporary(type, meta));
                    code.opTypeMeta = meta;
                    code.opType = type;
                    codes.push_back(code);
                    return code.result();
                }
                return {Location::Variable, VariableLocation(VariableLocation::Local, n, m)};
            }
        }
    }

    // now global variables
    for (uint64_t n = 0; n < globalVariableObjects.size(); n++) {
        auto& current = globalVariableObjects[n];
        if (current.identifier == identifier or *current.identifier == *identifier) {
            if (current.type != type) CodeGeneratorError("Type mismatch in last global variable match!");
            if (current.meta.pointerLevel != meta.pointerLevel) CodeGeneratorError("Pointer level mismatch in last global variable!");
            if (current.meta.isMutable < meta.isMutable) CodeGeneratorError("Cannot use non-mutable variables in mutable context! (might need different contitions)");
            if (not current.meta.isReference and meta.isReference) {
                Bytecode code;
                code.type = Bytecode::Address;
                code.op1({Location::Variable, VariableLocation(VariableLocation::Global, 0, n)});
                code.result(insertTemporary(type, meta));
                code.opTypeMeta = meta;
                code.opType = type;
                codes.push_back(code);
                return code.result();
            }
            if (current.meta.isReference and not meta.isReference) {
                // in that case the value of the referred is needed
                Bytecode code;
                code.type = Bytecode::Dereference;
                code.op1({Location::Variable, VariableLocation(VariableLocation::Global, 0, n)});
                code.result(insertTemporary(type, meta));
                code.opTypeMeta = meta;
                code.opType = type;
                codes.push_back(code);
                return code.result();
            }
            return {Location::Variable, VariableLocation(VariableLocation::Global, 0, n)};
        }
    }

    CodeGeneratorError("Variable \"" + *identifier + "\" not found!");
    return {};
}


VariableObject& BytecodeContext::getVariableObject(const std::string* identifier) {
    // first let's go through local variables from top and back
    for (int64_t n = activeLevels.size() - 1; n >= 0; n--) {
        for (int64_t m = localVariables[n].size() - 1; m >= 0 and localVariables[n].size() != 0; m--) {
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

    CodeGeneratorError("Variable \"" + *identifier + "\" not found!");
    return globalVariableObjects[0];
}

VariableObject& BytecodeContext::getVariableObject(BytecodeOperand operand) {
    if (operand.location != Location::Variable) CodeGeneratorError("Cannot get a variable from non-variable location!");
    switch (operand.value.variable.type) {
        case VariableLocation::None:
            CodeGeneratorError("Invalid variable type in get!");
        case VariableLocation::Global:
            return globalVariableObjects[operand.value.variable.number];
        case VariableLocation::Local:
            return localVariables[operand.value.variable.level][operand.value.variable.number];
        case VariableLocation::Temporary:
            return temporaries[operand.value.variable.number];
        default:
            CodeGeneratorError("Somehow could not find a variable object!");
            return globalVariableObjects[0];
    }
}

void BytecodeContext::addLoopLabel() {
    Bytecode code;
    code.type = Bytecode::LoopLabel;
    codes.push_back(code);
}

uint64_t TypeObject::getMemberOffsetAndType(std::string* identifier, TypeObject*& typeToSet, TypeMeta& typeMetaToSet) {
    for (auto& n : members) {
        if (n.name() == *identifier) {
            typeToSet = n.typeObject;
            typeMetaToSet  = n.typeMeta();
            return n.offset;
        }
    }
    CodeGeneratorError("Member \"" + *identifier + "\" is not a member of type \"" + typeName + "\"!");
    return 0;
}

ParserFunctionMethod& TypeObject::findMethod(std::string* identifier) {
    for (auto& n : methods) {
        if (*n.name == *identifier) return n;
    }

    CodeGeneratorError("Could not find a method with identifier: " + *identifier + " in type: " + typeName + "!");
    return methods.front();
}