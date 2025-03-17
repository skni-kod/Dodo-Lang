#include "AnalysisInternal.hpp"
#include "SyntaxAnalysis.hpp"
#include "TypeObject.hpp"
#include "../ParserVariables.hpp"

// TODO: add name syntax checks
void CreateType(Generator<LexerToken*>& generator, LexerToken*& firstToken) {
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

        current = generator();

        if (not generator or not current->MatchOperator(Operator::Lesser)) {
            ParserError("Expected an opening triangle bracket after primitive type identifier!");
        }

        current = generator();
        if (not current->MatchNumber(Type::unsignedInteger)) {
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
    if (not current->MatchOperator(Operator::BraceOpen)) {
        ParserError("Expected an opening bracket or block end after type name!");
    }

    // since members and methods are very similar to variables and functions they can use the same functions
    RunSyntaxAnalysis(generator, true, &type);

    if (not type.isPrimitive and type.members.empty()) {
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
        auto [memberSize, memberAlignment] = CalculateTypeSize(types[n.typeName()]);
        if (memberSize == 0) {
            return {0, 0};
        }
        if (sum % memberAlignment != 0) {
            sum = sum / memberAlignment + memberAlignment;
        }
        n.offset = sum;
        sum += memberSize;
        if (memberAlignment > alignment) {
            alignment = memberAlignment;
        }
        n.typeObject = &types[n.typeName()];
    }
    if (sum % alignment != 0) {
        sum = (sum / alignment + 1) * alignment;
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