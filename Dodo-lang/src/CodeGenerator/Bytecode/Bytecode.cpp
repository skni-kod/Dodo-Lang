#include "BytecodeInternal.hpp"
#include <iostream>

// checks literal types, matches then with sizes and issues errors and warnings
void CheckLiteralMatch(LexerToken* literal, TypeObject* type, TypeMeta typeMeta) {
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

// this function takes a vector of existing bytecode, a tree value table, expression type and starting index
// it recursively creates bytecode instructions needed to make the expression work
// it is the most important function of the entire bytecode generator, probably
// TODO: make this return some type to assign an operand
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
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Syscall:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Literal:
            CheckLiteralMatch(current.literal, type, typeMeta);
        return {Location::Literal, {current.literal->_unsigned}, type->primitiveType, static_cast<uint8_t>(type->typeSize)};
        case ParserOperation::Variable: {
            if (isGlobal and not current.isBeingDefined) CodeGeneratorError("Cannot initialize global variables with other variables!");
            auto var = context.getVariableObject(current.identifier);
            if (not current.isBeingDefined and current.next == 0 and (type != var.type or typeMeta != var.meta)) CodeGeneratorError("Variable type mismatch!");
            type = var.type;
            if (typeMeta.isReference != var.meta.isReference) var.meta.isReference = typeMeta.isReference;
            typeMeta = var.meta;
            if (current.next) {
                if (not typeMeta.isReference) {
                    // getting the address of the structure for the member if it's not already a reference
                    typeMeta.isReference = true;
                    code.type = Bytecode::Address;
                    code.op1(context.getVariable(current.identifier, type, typeMeta));
                    code.result(context.insertTemporary(type, typeMeta));
                    context.codes.push_back(code);

                    auto val = GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, code.result());
                    context.isGeneratingReference = false;

                    return val;
                }
                auto val = GenerateExpressionBytecode(context, values, type, typeMeta, current.next, isGlobal, context.getVariable(current.identifier, type, typeMeta));
                context.isGeneratingReference = false;
                return val;
            }
            return context.getVariable(current.identifier, type, typeMeta);
        }
        case ParserOperation::String:
            // checking if the type is valid for a string
            if (not type->isPrimitive or type->typeSize != 1
                or type->primitiveType != Type::unsignedInteger or typeMeta.pointerLevel != 1)
                    CodeGeneratorError("String literals can only be assigned to 1 byte unsigned integer pointers!");

            Strings.emplace_back(current.identifier, static_cast <bool>(typeMeta.isMutable));
            return {Location::String, {Strings.size() - 1}};
        case ParserOperation::Definition:
            code.type = Bytecode::Define;
            if (not types.map.contains(*current.identifier)) CodeGeneratorError("Type " + *current.identifier + " does not exist!");
            type = &types.map[*current.identifier];
            if (isGlobal) {
                // if it's global give it a number and push back a pointer
                code.op1({Location::Variable, {{VariableLocation::Global, 0, converterGlobals.size()}}});
                converterGlobals.emplace_back(type, values[current.next].identifier, typeMeta);
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

void GetTypes(std::vector<ParserTreeValue>& values, TypeObject*& type, TypeMeta& typeMeta, uint16_t index) {
    auto& current = values[index];

    if (current.operation == ParserOperation::Variable) {
        if (current.next) GetTypes(values, type, typeMeta, current.next);
        return;
    }

    if (current.operation == ParserOperation::Member) {
        type->getMemberOffsetAndType(current.identifier, type, typeMeta);
        typeMeta.isReference = true;
        if (current.next) GetTypes(values, type, typeMeta, current.next);
        return;
    }

    CodeGeneratorError("Invalid operation in type negotiation!");
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

    for (auto& n : function.instructions) {
        switch (n.type) {
            case Instruction::Expression:
                switch (n.valueArray[0].operation) {
                    case ParserOperation::Definition:
                        GenerateExpressionBytecode(context, n.valueArray, n.valueArray[0].definitionType, n.valueArray[0].typeMeta);
                        break;
                    case ParserOperation::Operator: {
                        {
                            if (not n.valueArray[0].left or n.valueArray[n.valueArray[0].left].operation != ParserOperation::Variable) CodeGeneratorError("Invalid expression!");
                            // now we need to find the type it seems
                            auto variable = context.getVariableObject(n.valueArray[n.valueArray[0].left].identifier);

                            TypeObject* type = variable.type;
                            TypeMeta typeMeta = variable.meta;

                            // get the type of the last member of the lvalue used
                            GetTypes(n.valueArray, type, typeMeta, n.valueArray[0].left);

                            GenerateExpressionBytecode(context, n.valueArray, type, typeMeta);
                            break;
                        }

                    }
                    default:
                        CodeGeneratorError("Unhandled expression type!");
                }
                break;

            default:
                CodeGeneratorError("Unhandled instruction type!");
                break;
        }
    }

    return context;
}

// methods

BytecodeValue::BytecodeValue(uint64_t val) {
    ui = val;
}
BytecodeValue::BytecodeValue(VariableLocation val) {
    variable = val;
}

BytecodeOperand Bytecode::op1() const {
    return {op1Location, op1Value, op1LiteralType, op1LiteralSize};
}

BytecodeOperand Bytecode::op1(BytecodeOperand op) {
    op1Value       = op.value;
    op1Location    = op.location;
    op1LiteralType = op.literalType;
    op1LiteralSize = op.literalSize;
    return op;
}

BytecodeOperand Bytecode::op2() const {
    return {op2Location, op2Value, op2LiteralType, op2LiteralSize};
}

BytecodeOperand Bytecode::op2(BytecodeOperand op) {
    op2Value       = op.value;
    op2Location    = op.location;
    op2LiteralType = op.literalType;
    op2LiteralSize = op.literalSize;
    return op;
}

BytecodeOperand Bytecode::op3() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::op3(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.literalSize;
    return op;
}

BytecodeOperand Bytecode::result() const {
    return {op3Location, op3Value, op3LiteralType, op3LiteralSize};
}

BytecodeOperand Bytecode::result(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    op3LiteralSize = op.literalSize;
    return op;
}

BytecodeOperand::BytecodeOperand(Location::Type location, BytecodeValue value, Type::TypeEnum literalType, uint8_t literalSize) {
    this->location = location;
    this->value = value;
    this->literalType = literalType;
    this->literalSize = literalSize;
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
    for (uint64_t n = 0; n < converterGlobals.size(); n++) {
        auto& current = converterGlobals[n];
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

    CodeGeneratorError("Variable\"" + *identifier + "\" not found!");
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
    for (auto& current : converterGlobals) {
        if (current.identifier == identifier or *current.identifier == *identifier) {
            return current;
        }
    }

    CodeGeneratorError("Variable\"" + *identifier + "\" not found!");
    return converterGlobals[0];
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// old code

BytecodeOld::BytecodeOld(uint64_t code) : code(code) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source) : code(code), source(source) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source, uint64_t number) : code(code), source(source), number(number) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source, VariableType type) : code(code), source(source), type(type) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source, uint64_t number, VariableType type) : code(code), source(source),
                                                                                            number(number),
                                                                                            type(type) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source, std::string target, VariableType type) : code(code),
                                                                                               source(source),
                                                                                               target(target),
                                                                                               type(type) {}

BytecodeOld::BytecodeOld(uint64_t code, std::string source, std::string target, uint64_t number, VariableType type) : code(
        code), source(source), target(target), number(number), type(type) {}

std::string EnumToVarType(uint8_t type) {
    switch (type) {
        case Type::signedInteger:
            return "signed integer";
        case Type::unsignedInteger:
            return "unsigned integer";
        case Type::floatingPoint:
            return "floating point";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::string EnumToComparisonType(uint8_t type) {
    switch (type) {
        case BytecodeOld::Condition::notEquals:
            return "is not equal";
        case BytecodeOld::Condition::equals:
            return "is equal";
        case BytecodeOld::Condition::greaterEqual:
            return "is greater or equal";
        case BytecodeOld::Condition::greater:
            return "is greater";
        case BytecodeOld::Condition::lesserEqual:
            return "is lesser or equal";
        case BytecodeOld::Condition::lesser:
            return "is lesser";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::string EnumToComparisonTypeNegative(uint8_t type) {
    switch (type) {
        case BytecodeOld::Condition::notEquals:
            return "is equal";
        case BytecodeOld::Condition::equals:
            return "is not equal";
        case BytecodeOld::Condition::greaterEqual:
            return "is lesser";
        case BytecodeOld::Condition::greater:
            return "is lesser or equal";
        case BytecodeOld::Condition::lesserEqual:
            return "is greater";
        case BytecodeOld::Condition::lesser:
            return "is greater or equal";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::ostream& operator<<(std::ostream& out, const BytecodeOld& code) {
    // none, add, subtract, multiply, divide, callFunction, returnValue, pushLevel, popLevel, jump,
    //        compare, declare, assign
    switch (code.code) {
        case BytecodeOld::none:
            out << "Invalid instruction\n";
            break;
        case BytecodeOld::add:
            out << "Add: " << code.source << " to: " << code.target << ", store result as: " << code.type.getPrefix() << EXPRESSION_SIGN << "#"
                << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::subtract:
            out << "Subtract: " << code.source << " from: " << code.target << ", store result as: " << code.type.getPrefix() << EXPRESSION_SIGN
                << "#" << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::multiply:
            out << "Multiply: " << code.target << " by: " << code.source << ", store result as: " << code.type.getPrefix() << EXPRESSION_SIGN
                << "#" << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::divide:
            out << "Divide: " << code.target << " by: " << code.source << ", store result as: " << code.type.getPrefix() << EXPRESSION_SIGN
                << "#" << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::callFunction:
            if (code.target.empty()) {
                out << "Call function: " << code.source << "( ... )\n";
            }
            else {
                out << "Call function: " << code.source << "( ... ) of type: " << code.type << " and store result in: "
                    << code.target << "\n";
            }
            break;
        case BytecodeOld::getAddress:
            out << "Extract address of: " << code.source  << " and store result as: " << VariableType(code.type.size, code.type.type, code.type.subtype + 1).getPrefix() << EXPRESSION_SIGN
                << "#" << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::getValue:
            out << "Extract value at: " << code.source  << " and store result as: " << VariableType(code.type.size, code.type.type, code.type.subtype - 1).getPrefix() << EXPRESSION_SIGN
                << "#" << code.number << "-0 using type: " << code.type << "\n";
            break;
        case BytecodeOld::moveArgument:
            out << "Move: " << code.source << " as a function argument, using type: " << code.type << "\n";
            break;
        case BytecodeOld::returnValue:
            out << "Return value of: " << code.source << " using type: " << code.type << "\n";
            break;
        case BytecodeOld::pushLevel:
            out << "Push variable level\n";
            break;
        case BytecodeOld::popLevel:
            out << "Pop variable level\n";
            break;
        case BytecodeOld::jumpConditionalFalse:
            out << "Jump to: " << code.source << " if left is " << EnumToComparisonTypeNegative(code.number)
                << " than right\n";
            break;
        case BytecodeOld::jumpConditionalTrue:
            out << "Jump to: " << code.source << " if left is " << EnumToComparisonType(code.number) << " than right\n";
            break;
        case BytecodeOld::jump:
            out << "Jump to: " << code.source << "\n";
            break;
        case BytecodeOld::compare:
            out << "Compare: " << code.source << " with: " << code.target << " using type: " << code.type << "\n";
            break;
        case BytecodeOld::declare:
            out << "Declare variable: " << code.target << " of type: " << code.type << " with assigned value of: "
                << code.source << "\n";
            break;
        case BytecodeOld::assign:
            if (code.number) {
                out << "Assign value of: " << code.source << " to the value pointed at by: " << code.target << " using type: " << code.type << "\n";
            }
            else {
                out << "Assign value of: " << code.source << " to: " << code.target << " using type: " << code.type << "\n";
            }
            break;
        case BytecodeOld::addLabel:
            out << "Add label: " << code.source << "\n";
            break;
        case BytecodeOld::moveValue:
            out << "Move value of: " << code.source << " to: " << code.target << " using type: " << code.type << "\n";
            break;
        case BytecodeOld::addFromArgument:
            out << "Add variable named: " << code.target << " from argument located at: " << code.source << "\n";
            break;
        case BytecodeOld::syscall:
            out << "Call syscall number: " << code.number << "\n";
        break;
        default:
            CodeGeneratorError("Invalid bytecode code!");
            break;
    }
    return out;
}