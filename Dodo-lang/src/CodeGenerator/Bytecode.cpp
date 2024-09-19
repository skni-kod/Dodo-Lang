#include <Bytecode.hpp>
#include <iostream>
#include "GenerateCode.hpp"

Bytecode::Bytecode(uint64_t code, std::string source) : code(code), source(source) {}

Bytecode::Bytecode(uint64_t code, std::string source, std::string target) : code(code), source(source), target(target) {}

Bytecode::Bytecode(uint64_t code, std::string source, std::string target, uint64_t number) : code(code), source(source), target(target), number(number) {}

Bytecode::Bytecode(uint64_t code, std::string source, uint64_t number) : code(code), source(source), number(number) {}

Bytecode::Bytecode(uint64_t code, std::string source, uint16_t sourceType, uint16_t sourceSize, uint16_t targetType, uint16_t targetSize)  :
    code(code), source(source), sourceType(sourceType), sourceSize(sourceSize), targetType(targetType), targetSize(targetSize) {}

Bytecode::Bytecode(uint64_t code, std::string source, uint16_t sourceType, uint16_t sourceSize, VariableType targetType)  :
        code(code), source(source), sourceType(sourceType), sourceSize(sourceSize), targetType(targetType.type), targetSize(targetType.size) {}

Bytecode::Bytecode(uint64_t code, std::string source, VariableType sourceType, VariableType targetType) :
        code(code), source(source), sourceType(sourceType.type), sourceSize(sourceType.size), targetType(targetType.type), targetSize(targetType.size) {}

Bytecode::Bytecode(uint64_t code, std::string source, std::string target, VariableType sourceType) :
        code(code), source(source), target(target), sourceType(sourceType.type), sourceSize(sourceType.size) {}

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

std::ostream& operator<<(std::ostream& out, const Bytecode& code) {
    // none, add, subtract, multiply, divide, callFunction, returnValue, pushLevel, popLevel, jump,
    //        compare, declare, assign
    switch (code.code) {
        case Bytecode::none:
            out << "Invalid instruction\n";
            break;
        case Bytecode::add:
            out << "Add: " << code.source << " to: " << code.target << ", store result as: "<< EXPRESSION_SIGN << code.number << "\n";
            break;
        case Bytecode::subtract:
            out << "Subtract: " << code.source << " from: " << code.target <<  ", store result as: "<< EXPRESSION_SIGN << code.number << "\n";
            break;
        case Bytecode::multiply:
            out << "Multiply: " << code.target << " by: " << code.source <<  ", store result as: "<< EXPRESSION_SIGN << code.number << "\n";
            break;
        case Bytecode::divide:
            out << "Divide: " << code.target << " by: " << code.source <<  ", store result as: "<< EXPRESSION_SIGN << code.number << "\n";
            break;
        case Bytecode::callFunction:
            out << "Call function: " << code.source << "\n";
            break;
        case Bytecode::moveArgument:
            out << "Move: " << code.source << " as function argument number: " << code.number << "\n";
            break;
        case Bytecode::prepareArguments:
            out << "Prepare arguments for function: " << code.source << "\n";
            break;
        case Bytecode::returnValue:
            out << "Return value of: " << code.source << "\n";
            break;
        case Bytecode::pushLevel:
            out << "Push variable level\n";
            break;
        case Bytecode::popLevel:
            out << "Pop variable level\n";
            break;
        case Bytecode::jumpConditional:
            out << "Jump to: " << code.source << " if <not implemented>\n";
            break;
        case Bytecode::jump:
            out << "Jump to: " << code.source << "\n";
            break;
        case Bytecode::compare:
            out << "Compare: " << code.source << " with: " << code.target << "\n";
            break;
        case Bytecode::declare:
            out << "Declare variable: " << code.target << " with assigned value: " << code.source << "\n";
            break;
        case Bytecode::assign:
            out << "Assign value of: " << code.source << " to: " << code.target << "\n";
            break;
        case Bytecode::convert:
            out << "Convert value of: " << code.source << " from size: " << code.sourceSize << " to: " << code.targetSize
                << " and type: " << EnumToVarType(code.sourceType) << " to: " << EnumToVarType(code.targetType) << "\n";
            break;
        default:
            CodeGeneratorError("Invalid bytecode code!");
            break;
    }
    return out;
}