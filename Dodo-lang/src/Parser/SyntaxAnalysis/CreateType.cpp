#include "AnalysisInternal.hpp"
#include "TypeObject.hpp"
#include "../ParserVariables.hpp"

// TODO: add name syntax checks
void CreateType(Generator<const LexerToken*>& generator, const LexerToken*& firstToken) {
    TypeObject type;

    // check if it's primitive
    if (firstToken->MatchKeyword(Keyword::Primitive)) {
       type.isPrimitive = true;
        // format:
        // primitive (SIGNED_INTEGER/UNSIGNED_INTEGER)<1/2/4/8>/FLOATING_POINT<2/4/8>

        if (not generator) {
            ParserError("Expected primitive type identifier!");
        }
        auto current = generator();

        if (current->MatchKeyword(Keyword::TypeSI)) {
            type.primitiveType = Type::signedInteger;
        }
        else if (current->MatchKeyword(Keyword::TypeUI)) {
            type.primitiveType = Type::unsignedInteger;
        }
        else if (current->MatchKeyword(Keyword::TypeFP)) {
            type.primitiveType = Type::floatingPoint;
        }
        else {
            ParserError("Invalid primitive type identifier!");
        }

        if (not generator or not current->MatchOperator(Operator::Lesser)) {
            ParserError("Expected an opening bracket after primitive type identifier!");
        }

        if (not generator) {
            ParserError("Expected primitive size after bracket opening!");
        }
        current = generator();
        if (current->MatchNumber(Type::unsignedInteger)) {
            ParserError("Expected primitive size after bracket opening!");
        }
        type.typeAlignment = type.typeSize = current->_unsigned;
        if (type.primitiveType == Type::floatingPoint) {
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

        if (not generator or not generator()->MatchOperator(Operator::Greater)) {
            ParserError("Expected a closing bracket after primitive type identifier!");
        }

        if (not generator or not generator()->MatchKeyword(Keyword::Type)) {
            ParserError("Expected \"type\" keyword after primitive type identifier!");
        }
    }

    // now actual type name, etc.
    if (not generator) {
        ParserError("Expected type name after type keyword!");
    }
    auto current = generator();
    if (current->type != Token::Identifier) {
        ParserError("Expected type name identifier after type keyword!");
    }
    if (types.isKey(*current->text)) {
        ParserError("Type name duplication: \"" + *current->text + "\"!");
    }
    type.typeName = *current->text;

    if (not generator) {
        ParserError("Expected an opening bracket or block end after type name!");
    }
    current = generator();
    if (current->MatchKeyword(Keyword::End)) {
        types.map.emplace(type.typeName, std::move(type));
        return;
    }
    if (current->MatchOperator(Operator::BraceOpen)) {
        ParserError("Expected an opening bracket or block end after type name!");
    }

    if (not generator) {
        ParserError("Expected a member type or closing bracket!");
    }
    current = generator();

    while (not current->MatchOperator(Operator::BraceClose)) {
        // TODO: do a common global, function and type parser here
        //CreateMember(type, generator, current->value);
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