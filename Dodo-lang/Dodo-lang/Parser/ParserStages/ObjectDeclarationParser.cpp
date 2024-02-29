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
                std::cout << "INFO L3: Added object declaration : " << current->value << "\n";
            }
        }
    }
}

inline ObjectMethod ParseFunctionPrototype(Generator<const LexicalToken*>& generator, const std::string& name, const std::string& returnType, const uint8_t& publicness, const uint8_t& typeType) {
    uint64_t bracketLevel = 1;
    ObjectMethod method;
    method.access = publicness;
    method.returnType.type = typeType;
    if (returnType == "void") {
        method.returnType.isVoid = true;
    }
    else if (IsType(returnType)) {
        method.returnType.dataPointer = &parserTypes[returnType];
    }
    else {
        method.returnType.isObject = true;
        method.returnType.objectPointer = &parserObjects[returnType];
    }
    while (true) {
        const LexicalToken* current = generator();
        if (current->type == tokenType::operand and current->value == ")") {
            return std::move(method);
        }

    }
}

void ParseObjectMethodMemberDefinitions(Generator<const LexicalToken*>& generator) {
    while (generator) {
        const LexicalToken* current = generator();
        if (current->type == tokenType::keyword and (current->value == "struct" or current->value == "class")) {
            uint8_t objectAccess;
            if (current->value == "struct") {
                objectAccess = ParserObject::Access::publicAccess;
            }
            else {
                objectAccess = ParserObject::Access::privateAccess;
            }
            // object name
            const std::string& currentObject = generator()->value;

            current = generator();
            if (current->type == tokenType::operand and current->value == ":") {
                // inheritance
                current = generator();
                uint8_t accessType = ParserObject::Access::publicAccess;
                if (current->type == tokenType::keyword) {
                    // inheritance access
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
                }
                if (current->type == tokenType::identifier) {
                    parserObjects[currentObject].parent = &parserObjects[current->value];
                    parserObjects[currentObject].parentAccess = accessType;
                }
                else {
                    ParserError("expected access specifier or identifier after specifier!");
                }
                current = generator();
            }
            if (current->type == tokenType::blockBegin) {
                // actual content of the object
                uint64_t bracketLevel = 1;
                while (bracketLevel > 0) {
                    current = generator();
                    if (current->type == blockBegin) {
                        bracketLevel++;
                    }
                    else if (current->type == blockEnd) {
                        if (bracketLevel == 0) {
                            ParserError("too many closing curly braces!");
                        }
                        bracketLevel--;
                    }
                    else if (bracketLevel == 1) {
                        // looking for variables and methods
                        if (current->type == tokenType::identifier or
                        (current->type == tokenType::keyword and current->value == "void")) {
                            // found a method or variable
                            const std::string& type = current->value;

                            if (!IsDeclarable(type) and current->value != "void") {
                                ParserError("cannot declare with non existent type!");
                            }

                            current = generator();
                            if (current->type != tokenType::identifier) {
                                ParserError("expected name identifier after type specifier");
                            }

                            const std::string& name = current->value;
                            current = generator();

                            if (current->type == tokenType::endline) {
                                // a variable
                                ObjectMember temp;
                                temp.access = objectAccess;
                                temp.type = ObjectMember::Type::value;
                                temp.defaultValue = false;
                                if (IsObject(type)) {
                                    temp.isObject = true;
                                    temp.objectPointer = &parserObjects[type];
                                }
                                else if (IsType(type)) {
                                    temp.dataPointer = &parserTypes[type];
                                }
                                parserObjects[currentObject].members.emplace_back(temp);
                                if (flags::informationLevel >= flags::informationLevel::full) {
                                    std::cout << "INFO L3: Added member to object : " <<
                                    currentObject << " : " << type << " " << name << "\n";
                                }

                            }
                            else if (current->type == tokenType::operand) {
                                if (current->value == "(") {
                                    // a function
                                    parserObjects[currentObject].methods.emplace_back
                                    (ParseFunctionPrototype(generator, name, type,objectAccess, 0));
                                    if (flags::informationLevel >= flags::informationLevel::full) {
                                        std::cout << "INFO L3: Added method to object : " << currentObject << " : " << type << " " << name << "(";
                                        for (const auto& n : parserObjects[currentObject].methods.back().arguments) {
                                            // TODO: add names of types and objects into their objects
                                        }
                                        std::cout << ")\n";
                                    }
                                }
                                else if (current->value == "=") {
                                    ParserError("initial values have not yet been introduced!");
                                }
                                else {
                                    ParserError("unexpected operand!");
                                }
                            }

                        }
                    }
                }
            }
            else {
                ParserError("unexpected token after object definition!");
            }

        }
    }
}
