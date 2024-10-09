#include "Bytecode.hpp"
#include <iostream>
#include "GenerateCode.hpp"

Bytecode::Bytecode(uint64_t code) : code(code) {}

Bytecode::Bytecode(uint64_t code, std::string source) : code(code), source(source) {}

Bytecode::Bytecode(uint64_t code, std::string source, uint64_t number) : code(code), source(source), number(number) {}

Bytecode::Bytecode(uint64_t code, std::string source, VariableType type) : code(code), source(source), type(type) {}

Bytecode::Bytecode(uint64_t code, std::string source, uint64_t number, VariableType type) : code(code), source(source),
                                                                                            number(number),
                                                                                            type(type) {}

Bytecode::Bytecode(uint64_t code, std::string source, std::string target, VariableType type) : code(code),
                                                                                               source(source),
                                                                                               target(target),
                                                                                               type(type) {}

Bytecode::Bytecode(uint64_t code, std::string source, std::string target, uint64_t number, VariableType type) : code(
        code), source(source), target(target), number(number), type(type) {}

std::string EnumToVarType(uint8_t type) {
    switch (type) {
        case ParserType::Type::signedInteger:
            return "signed integer";
        case ParserType::Type::unsignedInteger:
            return "unsigned integer";
        case ParserType::Type::floatingPoint:
            return "floating point";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::string EnumToComparisonType(uint8_t type) {
    switch (type) {
        case Bytecode::Condition::notEquals:
            return "is not equal";
        case Bytecode::Condition::equals:
            return "is equal";
        case Bytecode::Condition::greaterEqual:
            return "is greater or equal";
        case Bytecode::Condition::greater:
            return "is greater";
        case Bytecode::Condition::lesserEqual:
            return "is lesser or equal";
        case Bytecode::Condition::lesser:
            return "is lesser";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::string EnumToComparisonTypeNegative(uint8_t type) {
    switch (type) {
        case Bytecode::Condition::notEquals:
            return "is equal";
        case Bytecode::Condition::equals:
            return "is not equal";
        case Bytecode::Condition::greaterEqual:
            return "is lesser";
        case Bytecode::Condition::greater:
            return "is lesser or equal";
        case Bytecode::Condition::lesserEqual:
            return "is greater";
        case Bytecode::Condition::lesser:
            return "is greater or equal";
        default:
            CodeGeneratorError("Invalid type in string conversion!");
    }
    return "";
}

std::ostream& operator<<(std::ostream& out, const Bytecode& code) {
    // none, add, subtract, multiply, divide, callFunction, returnValue, pushLevel, popLevel, jump,
    //        compare, declare, assign
    switch (code.code) {
        case Bytecode::none:
            out << "Invalid instruction\n";
            break;
        case Bytecode::add:
            out << "Add: " << code.source << " to: " << code.target << ", store result as: " << EXPRESSION_SIGN << "#"
                << code.number << " using type: " << code.type << "\n";
            break;
        case Bytecode::subtract:
            out << "Subtract: " << code.source << " from: " << code.target << ", store result as: " << EXPRESSION_SIGN
                << "#" << code.number << " using type: " << code.type << "\n";
            break;
        case Bytecode::multiply:
            out << "Multiply: " << code.target << " by: " << code.source << ", store result as: " << EXPRESSION_SIGN
                << "#" << code.number << " using type: " << code.type << "\n";
            break;
        case Bytecode::divide:
            out << "Divide: " << code.target << " by: " << code.source << ", store result as: " << EXPRESSION_SIGN
                << "#" << code.number << " using type: " << code.type << "\n";
            break;
        case Bytecode::callFunction:
            if (code.target.empty()) {
                out << "Call function: " << code.source << "( ... )\n";
            }
            else {
                out << "Call function: " << code.source << "( ... ) of type: " << code.type << " and store result in: "
                    << code.target << "\n";
            }
            break;
        case Bytecode::moveArgument:
            out << "Move: " << code.source << " as function argument number: " << code.number << " using type: "
                << code.type << "\n";
            break;
        case Bytecode::prepareArguments:
            out << "Prepare arguments for function: " << code.source << "\n";
            break;
        case Bytecode::returnValue:
            out << "Return value of: " << code.source << " using type: " << code.type << "\n";
            break;
        case Bytecode::pushLevel:
            out << "Push variable level\n";
            break;
        case Bytecode::popLevel:
            out << "Pop variable level\n";
            break;
        case Bytecode::jumpConditionalFalse:
            out << "Jump to: " << code.source << " if left is " << EnumToComparisonTypeNegative(code.number)
                << " than right\n";
            break;
        case Bytecode::jumpConditionalTrue:
            out << "Jump to: " << code.source << " if left is " << EnumToComparisonType(code.number) << " than right\n";
            break;
        case Bytecode::jump:
            out << "Jump to: " << code.source << "\n";
            break;
        case Bytecode::compare:
            out << "Compare: " << code.source << " with: " << code.target << " using type: " << code.type << "\n";
            break;
        case Bytecode::declare:
            out << "Declare variable: " << code.target << " of type: " << code.type << " with assigned value of: "
                << code.source << "\n";
            break;
        case Bytecode::assign:
            out << "Assign value of: " << code.source << " to: " << code.target << " using type: " << code.type << "\n";
            break;
        case Bytecode::addLabel:
            out << "Add label: " << code.source << "\n";
            break;
        case Bytecode::moveValue:
            out << "Move value of: " << code.source << " to: " << code.target << " using type: " << code.type << "\n";
            break;
        case Bytecode::addFromArgument:
            out << "Add variable named: " << code.target << " from argument located at: " << code.source << "\n";
            break;
        default:
            CodeGeneratorError("Invalid bytecode code!");
            break;
    }
    return out;
}