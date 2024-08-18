#include "AnalysisInternal.hpp"
#include "../ParserVariables.hpp"

void CreateFunction(Generator<const LexicalToken*>& generator, const std::string& returnTypeName) {
    const LexicalToken* current = generator();

    // <type> <<type operand>>
    ParserFunction function;
    if (returnTypeName == "void") {
        function.returnValueType = ParserFunction::Subtype::none;
    }
    else {
        function.returnType = returnTypeName;
        if (current->type == LexicalToken::Type::operand) {
            if (current->value == "*") {
                function.returnValueType = ParserFunction::Subtype::pointer;
            }
            else if (current->value == "&") {
                function.returnValueType = ParserFunction::Subtype::reference;
            }
            else {
                ParserError("Unexpected operand after function return type!");
            }
            current = generator();
        }
        else {
            function.returnValueType = ParserFunction::Subtype::value;
        }
    }

    // <type> <<type operand>> <name>
    if (current->type != LexicalToken::Type::identifier) {
        ParserError("Expected an identifier after function return type declaration!");
    }
    if (parserFunctions.isKey(current->value)) {
        ParserError("Function redefinition!");
    }
    function.name = current->value;

    // <type> <<type operand>> <name> (
    current = generator();
    if (current->type != LexicalToken::Type::operand or current->value != "(") {
        ParserError("Expected an opening bracket after function name!");
    }

    // <type> <<type operand>> <name> ( <<argument>> / )
    current = generator();
    while (current->type != LexicalToken::Type::operand or current->value != ")") {
        // ... ( <type name>
        if (current->type != LexicalToken::Type::identifier) {
            ParserError("Expected an identifier at beginning of argument definition!");
        }
        FunctionArgument argument;
        argument.typeName = current->value;

        // ... ( <type name> <<operand>> <name>
        current = generator();
        if (current->type == LexicalToken::Type::operand) {
            if (current->value == "*") {
                argument.type = FunctionArgument::Subtype::pointer;
            }
            else if (current->value == "&") {
                argument.type = FunctionArgument::Subtype::reference;
            }
            else {
                ParserError("Unexpected operand after function argument type name!");
            }
            current = generator();
        }
        else {
            argument.type = FunctionArgument::Subtype::value;
        }
        if (current->type != LexicalToken::Type::identifier) {
            ParserError("Expected an identifier after function argument type!");
        }
        argument.name = current->value;

        // ... ( <type name> <<operand>> <name> <<=>> / , / )
        current = generator();
        // TODO: add support for default values here
        if (current->type == LexicalToken::Type::comma) {
            current = generator();
        }
        function.arguments.emplace_back(std::move(argument));
    }

    // ... ( ... ) {
    current = generator();
    if (current->type != LexicalToken::Type::blockBegin) {
        ParserError("Expected a block begin operand after function arguments!");
    }

    // ... ( ... ) { ... }
    current = generator();
    while (current->type != LexicalToken::Type::blockEnd) {

        // FUNCTION INSTRUCTIONS
        function.instructions.push_back(CreateInstruction(generator, current));

        current = generator();
    }
    parserFunctions.insert(function.name, std::move(function));
}