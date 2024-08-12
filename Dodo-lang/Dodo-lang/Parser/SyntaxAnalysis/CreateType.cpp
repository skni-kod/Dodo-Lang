#include "AnalysisInternal.hpp"
#include "../ParserVariables.hpp"

void CreateType(Generator<const LexicalToken*>& generator) {
    // type <name>
    if (!generator) {
        ParserError("Expected more tokens after base type declaration!");
    }

    const LexicalToken* current = generator();
    if (current->type != LexicalToken::type::identifier) {
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
        type = ParserType::type::SIGNED_INTEGER;
    }
    else if (current->value == "UNSIGNED_INTEGER") {
        type = ParserType::type::UNSIGNED_INTEGER;
    }
    else if (current->value == "FLOATING_POINT") {
        type = ParserType::type::FLOATING_POINT;
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

    // type <name> : <type>(<size>
    if (!generator) {
        ParserError("Expected more tokens after base type type definition bracket opening!");
    }

    current = generator();
    if (current->type != LexicalToken::type::literal) {
        ParserError("Expected a number inside basic type size bracket!");
    }
    if (current->literalValue < 0 or current->literalValue > 8) {
        ParserError("Invalid value inside basic type size bracket!");
    }
    uint8_t size = current->literalValue;

    // type <name> : <type>(<size>)
    if (!generator) {
        ParserError("Expected more tokens after base type size definition!");
    }

    current = generator();
    if (current->value != ")") {
        ParserError("Expected a closing bracket after basic type size definition!");
    }

    // type <name> : <type>(<size>);
    if (!generator) {
        ParserError("Expected more tokens after base type type definition bracket!");
    }

    current = generator();
    if (current->type != LexicalToken::type::expressionEnd) {
        ParserError("Expected an end of expression token after closing of basic type type bracket!");
    }

    parserTypes.insert(name, ParserType(type, size));
}