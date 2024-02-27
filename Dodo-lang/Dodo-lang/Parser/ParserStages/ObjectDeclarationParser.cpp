#include "ObjectDeclarationParser.hpp"
#include "../ParserVariables.hpp"

void ParseObjectDeclarations(Generator<const LexicalToken*>& generator) {
    while (generator) {
        const LexicalToken* current = generator();
        if (current->type == tokenType::keyword and (current->value == "struct" or current->value == "class")) {

            // object name
            current = generator();
            if (current->type != tokenType::identifier) {
                ParserError("expected an identifier after object declaration!");
            }
            parserObjects.insert(current->value, ParserObject());
            if (flags::informationLevel == flags::informationLevel::full){
                std::cout << "INFO L3: Added object declaration: " << current->value << "\n";
            }

            // TODO: add check for object definitions inside objects - maybe treating is as a normal object in another place?
        }
    }
}

void ParseObjectMethodDefinitions(Generator<const LexicalToken*>& generator) {
    while (generator) {
        const LexicalToken* current = generator();
        if (current->type == tokenType::keyword and (current->value == "struct" or current->value == "class")) {

            // object name
            const std::string& currentObject = generator()->value;

            current = generator();
            if (current->type == tokenType::operand and current->value == ":") {
                // inheritance
                current = generator();
                if (current->type == tokenType::identifier) {

                }
                else if (current->type == tokenType::keyword) {
                    uint8_t accessType = 0;
                    if (current->value == "public") {
                        accessType = ParserObject::Access::publicAccess;
                    }
                    else if (current->value == "private") {
                        accessType = ParserObject::Access::privateAccess;
                    }
                    else if (current->value == "protected") {
                        accessType = ParserObject::Access::protectedAccess;
                    }
                    else {
                        ParserError("invalid access specifier!");
                    }

                    current = generator();
                    if (current->type == tokenType::identifier) {

                    }
                    else {
                        ParserError("expected identifier after access specifier!");
                    }

                }
                else {
                    ParserError("expected identifier of access specifier!");
                }
            }
            else if (current->type == tokenType::blockBegin) {
                // no inheritance
            }
            else {
                ParserError("unexpected token after object definition!");
            }

        }
    }
}
