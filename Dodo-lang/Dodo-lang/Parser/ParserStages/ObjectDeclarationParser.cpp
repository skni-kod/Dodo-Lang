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
                std::cout << "INFO L3: Added object declaration: " << current->value << "\n";d
            }

            // TODO: add check for object definitions inside objects - maybe treating is as a normal object in another place?
        }
    }
}