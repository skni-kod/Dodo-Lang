#include "BytecodeInternal.hpp"
#include <iostream>


// this function takes a vector of existing bytecode, a tree value table, expression type and starting index
// it recursively creates bytecode instructions needed to make the expression work
// it is the most important function of the entire bytecode generator, probably
// TODO: make this return some type to assign an operand
BytecodeOperand GenerateExpressionBytecode(BytecodeContext& context, std::vector<ParserTreeValue>& values, TypeObject* type, TypeMeta typeMeta, uint16_t index, bool isGlobal) {
    // first off let's do a switch to know what is going on
    auto current = values[index];

    Bytecode code;
    code.opType = type;
    code.opTypeMeta = typeMeta;
    switch (current.operation) {
        case ParserOperation::None:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Operator:
        case ParserOperation::SingleOperator:
            return InsertOperatorExpression(context, values, type, typeMeta, index, isGlobal);
        case ParserOperation::Group:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Member:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Call:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Syscall:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Argument:
            CodeGeneratorError("Not implemented!");
        case ParserOperation::Literal:
            return {Location::Literal, current.literal->literalType, {current.literal->_unsigned}};
        case ParserOperation::Variable:
            CodeGeneratorError("Variables not implemented!");
        case ParserOperation::String:
            CodeGeneratorError("String not implemented!");
        case ParserOperation::Definition:
            code.type = Bytecode::Define;
            if (isGlobal) {
                // if it's global give it a number and push back a pointer
                code.op1(Location::Variable, {converterGlobals.size()});
                converterGlobals.push_back(&globalVariables[*values[current.next].identifier]);
            }
            else CodeGeneratorError("Local variables not implemented!");

            if (current.value) {
                code.result(GenerateExpressionBytecode(context, values, type, typeMeta, current.value, isGlobal));
                context.codes.push_back(code);
                return code.result();
            }
            // TODO: add user definable default values for types!
            CodeGeneratorError("Default value support incomplete!");
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
    BytecodeOperand op;
    op.location    = op1Location;
    op.value       = op1Value;
    op.literalType = op1LiteralType;
    return op;
}

BytecodeOperand Bytecode::op1(BytecodeOperand op) {
    op1Value       = op.value;
    op1Location    = op.location;
    op1LiteralType = op.literalType;
    return op;
}

BytecodeOperand Bytecode::op1(Location::Type location, BytecodeValue value, Type::TypeEnum literalType) {
    op1Location    = location;
    op1Value       = value;
    op1LiteralType = literalType;
    return op1();
}

BytecodeOperand Bytecode::op2() const {
    BytecodeOperand op;
    op.location    = op2Location;
    op.value       = op2Value;
    op.literalType = op2LiteralType;
    return op;
}

BytecodeOperand Bytecode::op2(BytecodeOperand op) {
    op2Value       = op.value;
    op2Location    = op.location;
    op2LiteralType = op.literalType;
    return op;
}

BytecodeOperand Bytecode::op2(Location::Type location, BytecodeValue value, Type::TypeEnum literalType) {
    op2Location    = location;
    op2Value       = value;
    op2LiteralType = literalType;
    return op2();
}

BytecodeOperand Bytecode::op3() const {
    BytecodeOperand op;
    op.location    = op3Location;
    op.value       = op3Value;
    op.literalType = op3LiteralType;
    return op;
}

BytecodeOperand Bytecode::op3(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    return op;
}

BytecodeOperand Bytecode::op3(Location::Type location, BytecodeValue value, Type::TypeEnum literalType) {
    op3Location    = location;
    op3Value       = value;
    op3LiteralType = literalType;
    return op3();
}

BytecodeOperand Bytecode::result() const {
    BytecodeOperand op;
    op.location    = op3Location;
    op.value       = op3Value;
    op.literalType = op3LiteralType;
    return op;
}

BytecodeOperand Bytecode::result(BytecodeOperand op) {
    op3Value       = op.value;
    op3Location    = op.location;
    op3LiteralType = op.literalType;
    return op;
}

BytecodeOperand Bytecode::result(Location::Type location, BytecodeValue value, Type::TypeEnum literalType) {
    op3Location    = location;
    op3Value       = value;
    op3LiteralType = literalType;
    return result();
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