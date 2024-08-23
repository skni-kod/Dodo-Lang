#include "AnalysisInternal.hpp"
#include "../ParserVariables.hpp"

void CreateType(Generator<const LexicalToken*>& generator) {
    // type <name>
    if (!generator) {
        ParserError("Expected more tokens after base type declaration!");
    }

    const LexicalToken* current = generator();
    if (current->type != LexicalToken::Type::identifier) {
        ParserError("Expected an identifier after base type declaration!");
    }

    const std::string& name = current->value;
    if (parserTypes.isKey(name)) {
        ParserError("Base type redefinition!");
    }
    // type <name> :
    if (!generator) {
        ParserError("Expected more tokens after base type name declaration!");
    }

    current = generator();
    if (current->value != ":") {
        ParserError("Expected a definition operand after base type name declaration!");
    }
    // type <name> : <type>
    if (!generator) {
        ParserError("Expected more tokens after base type definition!");
    }

    current = generator();
    uint8_t type = 0;
    if (current->value == "SIGNED_INTEGER") {
        type = ParserType::Type::signedInteger;
    }
    else if (current->value == "UNSIGNED_INTEGER") {
        type = ParserType::Type::unsignedInteger;
    }
    else if (current->value == "FLOATING_POINT") {
        type = ParserType::Type::floatingPoint;
    }
    else {
        ParserError("Invalid type of base type!");
    }

    // type <name> : <type>(
    if (!generator) {
        ParserError("Expected more tokens after base type type definition!");
    }

    current = generator();
    if (current->value != "(") {
        ParserError("Expected an opening bracket after basic type type definition!");
    }

    // type <name> : <type>(<singleSize>
    if (!generator) {
        ParserError("Expected more tokens after base type type definition bracket opening!");
    }

    current = generator();
    if (current->type != LexicalToken::Type::literal or current->literalValue != literalType::numeric) {
        ParserError("Expected a number inside basic type singleSize bracket!");
    }
    if (std::stoi(current->value) < 1 or std::stoi(current->value) > 8) {
        ParserError("Invalid value inside basic type singleSize bracket!");
    }
    uint8_t size = std::stoll(current->value);

    // type <name> : <type>(<singleSize>)
    if (!generator) {
        ParserError("Expected more tokens after base type singleSize definition!");
    }

    current = generator();
    if (current->value != ")") {
        ParserError("Expected a closing bracket after basic type singleSize definition!");
    }

    // type <name> : <type>(<singleSize>);
    if (!generator) {
        ParserError("Expected more tokens after base type type definition bracket!");
    }

    current = generator();
    if (current->type != LexicalToken::Type::expressionEnd) {
        ParserError("Expected an end of expression token after closing of basic type type bracket!");
    }

    parserTypes.insert(name, ParserType(type, size));
}