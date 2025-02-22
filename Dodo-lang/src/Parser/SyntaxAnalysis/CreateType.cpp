#include "AnalysisInternal.hpp"
#include "TypeObject.hpp"
#include "../ParserVariables.hpp"

void CreateMember(TypeObject& type, Generator<const LexicalToken*>& generator, const std::string& typeName) {
    // TODO: add methods and non-default operators

    const auto* current = generator();





    TypeObjectMember member;

    member.memberTypeName = std::make_unique<std::string>(typeName);
    if (not generator) {
        ParserError("Expected a member name!");
    }
    member.memberName = generator()->value;
    // TODO: add default values
    if (not generator or generator()->value != ";") {
        ParserError("Expected a block end after member declaration!");
    }

    type.members.emplace_back(std::move(member));
}

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
        type.typeAlignment = type.typeSize = stoull(current->value);
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

    if (not generator) {
        ParserError("Expected an opening bracket or block end after type name!");
    }
    current = generator();
    if (current->value == ";") {
        types.map.emplace(type.typeName, std::move(type));
        return;
    }
    if (current->value != "{") {
        ParserError("Expected an opening bracket or block end after type name!");
    }

    if (not generator) {
        ParserError("Expected a member type or closing bracket!");
    }
    current = generator();

    while (current->value != "}") {
        CreateMember(type, generator, current->value);
        current = generator();
    }

    if (not type.isPrimitive and type.members.size() == 0) {
        ParserError("Member-less complex types are prohibited!");
    }

    types.map.emplace(type.typeName, std::move(type));
}

std::pair<uint64_t, uint64_t> CalculateTypeSize(TypeObject& type) {
    // was already calculated
    if (type.typeSize != 0) {
        uint64_t size = type.typeSize;
        uint64_t alignment = type.typeAlignment;
        return {size, alignment};
    }
    uint64_t sum = 0, alignment = 0;
    for (auto& n : type.members) {
        auto result = CalculateTypeSize(types[*n.memberTypeName]);
        if (result.first == 0) {
            return {0, 0};
        }
        if (sum % result.second != 0) {
            sum = sum / result.second + result.second;
        }
        n.memberOffset = sum;
        sum += result.first;
        if (result.second > alignment) {
            alignment = result.second;
        }
        n.memberType = &types[*n.memberTypeName];
        n.memberTypeName = nullptr;
    }
    if (sum % alignment != 0) {
        sum = sum / alignment;
    }
    type.typeSize = sum;
    type.typeAlignment = alignment;
    return {sum, alignment};
}

void CalculateTypeSizes() {
    // count refers to values left not calculated in this round
    uint64_t lastCount = 0xFFFFFFFFFFFFFFFF;
    while (true) {
        uint64_t currentCount = 0;
        for (auto& n : types.map) {
            currentCount += (CalculateTypeSize(n.second).first == 0);
        }

        if (currentCount == 0) {
            break;
        }
        if (currentCount == lastCount) {
            ParserError("Type size calculation deadlock!");
        }
        lastCount = currentCount;
    }
}