#include "AnalysisInternal.hpp"
#include "TypeObject.hpp"
#include "../ParserVariables.hpp"

// TODO: add name syntax checks
void CreateType(Generator<const LexicalToken*>& generator, const std::string& starterWord) {
    TypeObject type;

    // check if it's primitive
    if (starterWord == "primitive") {
       type.isPrimitive = true;
        // format:
        // primitive (SIGNED_INTEGER/UNSIGNED_INTEGER)<1/2/4/8>/FLOATING_POINT<2/4/8>

        if (not generator) {
            ParserError("Expected primitive type identifier!");
        }
        auto current = generator();

        if (current->value == "SIGNED_INTEGER") {
            type.primitiveType = Primitive::signedInteger;
        }
        else if (current->value == "UNSIGNED_INTEGER") {
            type.primitiveType = Primitive::unsignedInteger;
        }
        else if (current->value == "FLOATING_POINT") {
            type.primitiveType = Primitive::floatingPoint;
        }
        else {
            ParserError("Invalid primitive type identifier!");
        }

        if (not generator or generator()->value != "<") {
            ParserError("Expected an opening bracket after primitive type identifier!");
        }

        if (not generator) {
            ParserError("Expected primitive size after bracket opening!");
        }
        current = generator();
        if (current->type != LexicalToken::Type::literal) {
            ParserError("Expected primitive size after bracket opening!");
        }
        type.typeSize = stoull(current->value);
        if (type.primitiveType == Primitive::floatingPoint) {
            if (type.typeSize != 2 and type.typeSize != 4 and type.typeSize != 8) {
                ParserError("Only sizes of 2, 4 and 8 bytes are accepted for float primitives!");
            }
        }
        else {
            if (type.typeSize != 1 and type.typeSize != 2 and
                type.typeSize != 4 and type.typeSize != 8) {
                ParserError("Only sizes of 1, 2, 4 and 8 bytes are accepted for integer primitives!");
            }
        }

        if (not generator or generator()->value != ">") {
            ParserError("Expected a closing bracket after primitive type identifier!");
        }

        if (not generator or generator()->value != "type") {
            ParserError("Expected \"type\" keyword after primitive type identifier!");
        }
    }

    // now actual type name, etc.
    if (not generator) {
        ParserError("Expected type name after keyword!");
    }
    auto current = generator();
    if (types.isKey(current->value)) {
        ParserError("Type name duplication: \"" + current->value + "\"!");
    }
    type.typeName = current->value;

    // in that case nothing after is acceptable
    if (type.isPrimitive) {
        if (not generator) {
            ParserError("Expected an opening bracket or block end after primitive name!");
        }
        current = generator();
        if (current->value == ";") {
            types.map.emplace(type.typeName, std::move(type));
            return;
        }
        if (current->value != "{") {
            ParserError("Expected an opening bracket or block end after primitive name!");
        }
    }
    else if (not generator or generator()->value != "{") {
        ParserError("Expected an opening bracket after type name!");
    }

    if (not generator) {
        ParserError("Expected a member type or closing bracket!");
    }
    current = generator();
    while (current->value != "}") {
        TypeObjectMember member;

        member.memberTypeName = std::make_unique<std::string>(current->value);
        if (not generator) {
            ParserError("Expected a member name!");
        }
        member.memberName = generator()->value;
        // TODO: add default values
        if (not generator or generator()->value != ";") {
            ParserError("Expected a block end after member declaration!");
        }

        type.members.emplace_back(std::move(member));

        current = generator();
    }

    types.map.emplace(type.typeName, std::move(type));
}

/*
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
*/