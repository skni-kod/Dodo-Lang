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
BytecodeOperand GenerateExpressionBytecode(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal) {
    // first off let's do a switch to know what is going on
    auto& current = values[index];

    Bytecode code;
    code.opType = type;
    code.opTypeMeta = typeMeta;
    switch (current.operation) {
        case ParserOperation::Operator:
        case ParserOperation::SingleOperator:
        case ParserOperation::Group:
            return InsertOperatorExpression(context, values, type, typeMeta, index, isGlobal);
        case ParserOperation::Member:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Call:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Syscall:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Argument:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Literal:
            CheckLiteralMatch(current.literal, type, typeMeta);
            return {Location::Literal, {current.literal->_unsigned}, type->primitiveType, static_cast<uint8_t>(type->typeSize)};
        case ParserOperation::Variable:
            // TODO: make this work correctly
            if (current.isBeingDefined) return {Location::Variable, {globalVariables.size() - 1}};
            if (isGlobal) CodeGeneratorError("Cannot initialize global variables with other variables!");
            CodeGeneratorError("Variables not implemented!");
        case ParserOperation::String:
            // checking if the type is valid for a string
            if (not type->isPrimitive or type->typeSize != 1
                or type->primitiveType != Type::unsignedInteger or typeMeta.pointerLevel != 1)
                    CodeGeneratorError("String literals can only be assigned to 1 byte unsigned integer pointers!");

            // TODO: add support for string mutability
            Strings.emplace_back(current.identifier, false);
            return {Location::String, {Strings.size() - 1}};
        case ParserOperation::Definition:
            code.type = Bytecode::Define;
            if (isGlobal) {
                // if it's global give it a number and push back a pointer
                code.op1({Location::Variable, {converterGlobals.size()}});
                converterGlobals.push_back(&globalVariables[*values[current.next].identifier]);
            }
            else CodeGeneratorError("Local variables not implemented!");
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

std::vector<Bytecode> GenerateGlobalVariablesBytecode() {
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

    return context.codes;
}

std::vector<Bytecode> GenerateFunctionBytecode(ParserFunction& function) {

    return {};
}

// methods

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
    newContext.tempCounter = tempCounter;
    newContext.isConstExpr = isConstExpr;
    newContext.isMutable = isMutable;
    return newContext;
}

void BytecodeContext::merge(const BytecodeContext& context) {
    const auto start = codes.size();
    codes.resize(codes.size() + context.codes.size());
    for (size_t n = 0; n < context.codes.size(); n++) {
        codes[start + n] = context.codes[n];
    }
}

// old code

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