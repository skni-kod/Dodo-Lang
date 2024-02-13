#include "TypeParser.hpp"
#include "../ParserVariables.hpp"
#include "../Parser.hpp"

void ParseTypes(Generator<const LexicalToken*>& generator) {
    while (generator) {
        // get the next token
        const LexicalToken* current = generator();

        if (current->type == tokenType::keyword and current->value == "type") {

            // get name
            current = generator();
            if (current->type != tokenType::identifier) {
                ParserError("expected an identifier after type keyword!");
            }
            const std::string& name = current->value;

            // get the : operand
            current = generator();
            if (current->type != tokenType::operand or current->value != ":") {
                // TODO: find out how to call ":"
                ParserError("expected a proper operand after type name!");
            }
            // TODO: add custom behaviour types

            current = generator();
            if (current->type != tokenType::identifier) {
                ParserError("expected an identifier after definition operand!");
            }
            uint8_t type = 3;
            if (current->value == "SIGNED_INTEGER") {
                type = ParserType::SIGNED_INTEGER;
            }
            else if (current->value == "UNSIGNED_INTEGER") {
                type = ParserType::UNSIGNED_INTEGER;
            }
            else if (current->value == "FLOATING_POINT") {
                type = ParserType::FLOATING_POINT;
            }
            else {
                ParserError("incorrect data type!");
            }

            current = generator();
            if (current->type != tokenType::operand or current->value != "(") {
                ParserError("expected a bracket operand after data type!");
            }

            // size
            current = generator();
            if (current->type != tokenType::literal or current->literal != literalType::numeric) {
                ParserError("unexpected size type!");
            }
            int64_t size = std::stoll(current->value);
            if (size < 0 or size > 8) {
                ParserError("unexpected size!");
            }

            if (IsType(name)) {
                ParserError("type redefinition!");
            }
            parserTypes.insert(std::pair<std::string, ParserType>(name, ParserType(type, size)));
            if (flags::informationLevel == flags::informationLevel::full) {
                std::cout << "INFO L3: Added type: " << name;
                switch (type) {
                    case ParserType::SIGNED_INTEGER:
                        std::cout << " : signed integer : ";
                        break;
                    case ParserType::UNSIGNED_INTEGER:
                        std::cout << " : unsigned integer : ";
                        break;
                    case ParserType::FLOATING_POINT:
                        std::cout << " : floating point : ";
                        break;
                }
                std::cout << size << " bytes in size\n";
            }

            current = generator();
            if (current->type != tokenType::operand or current->value != ")") {
                ParserError("expected a bracket operand after data size!");
            }

            current = generator();
            if (current->type != tokenType::endline) {
                ParserError("expected an end line operand at the end of definition!");
            }
        }
    }
}