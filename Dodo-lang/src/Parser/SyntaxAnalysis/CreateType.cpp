#include "AnalysisInternal.hpp"
#include "ErrorHandling.hpp"
#include "SyntaxAnalysis.hpp"
#include "TypeObject.hpp"

// TODO: add name syntax checks
void CreateType(Generator<LexerToken*>& generator, LexerToken*& firstToken) {
    TypeObject type;

    // check if it's primitive
    if (firstToken->Match(Keyword::Primitive)) {
       type.isPrimitive = true;
        // format:
        // primitive (SIGNED_INTEGER/UNSIGNED_INTEGER)<1/2/4/8>/FLOATING_POINT<2/4/8>

        if (not generator) {
            Error("Expected primitive type identifier!");
        }
        auto current = generator();

        if (current->Match(Keyword::TypeSI)) {
            type.primitiveType = Type::signedInteger;
        }
        else if (current->Match(Keyword::TypeUI)) {
            type.primitiveType = Type::unsignedInteger;
        }
        else if (current->Match(Keyword::TypeFP)) {
            type.primitiveType = Type::floatingPoint;
        }
        else {
            Error("Invalid primitive type identifier!");
        }

        current = generator();

        if (not generator or not current->Match(Operator::Lesser)) {
            Error("Expected an opening triangle bracket after primitive type identifier!");
        }

        current = generator();
        if (not current->Match(Type::unsignedInteger)) {
            Error("Expected primitive size after bracket opening!");
        }
        type.typeAlignment = type.typeSize = current->_unsigned;
        if (type.primitiveType == Type::floatingPoint) {
            if (type.typeSize != 2 and type.typeSize != 4 and type.typeSize != 8) {
                Error("Only sizes of 2, 4 and 8 bytes are accepted for float primitives!");
            }
        }
        else {
            if (type.typeSize != 1 and type.typeSize != 2 and
                type.typeSize != 4 and type.typeSize != 8) {
                Error("Only sizes of 1, 2, 4 and 8 bytes are accepted for integer primitives!");
            }
        }

        if (not generator or not generator()->Match(Operator::Greater)) {
            Error("Expected a closing bracket after primitive type identifier!");
        }

        if (not generator or not generator()->Match(Keyword::Type)) {
            Error("Expected \"type\" keyword after primitive type identifier!");
        }
    }

    // now actual type name, etc.
    if (not generator) {
        Error("Expected type name after type keyword!");
    }
    auto current = generator();
    if (not current->Match(Token::Identifier)) {
        Error("Expected type name identifier after type keyword!");

    }
    if (types.contains(*current->text)) {
        Error("Type name duplication: \"" + *current->text + "\"!");
    }
    type.typeName = *current->text;

    if (not generator) {
        Error("Expected an opening bracket or block end after type name!");
    }
    current = generator();

    // type attributes
    if (current->Match(Operator::BracketOpen)) {

#define AssignTypeAttributeError(condition, error) if (condition) Error(error);

        /// <summary>
        /// Checks if current identifier matches the attribute names,
        /// varadic arguments are the condition and text for an additional constraint error
        /// </summary>
        /// <param name="attribute">Attribute to assign</param>
#define AssignTypeAttribute(attribute, ...) if (*current->text == MACRO_STRING(attribute)) { \
        PlaceIf2Arg(__VA_ARGS__, AssignTypeAttributeError(__VA_ARGS__),) \
        type.attributes.attribute = true; continue; }

        do {
            current = generator();
            if (not current->Match(Token::Identifier))
                Error("Expected an attribute identifier!");

            AssignTypeAttribute(TypeAttribute_primitiveAssignFromLiteral,
                not type.isPrimitive, std::string("Cannot assign attribute: ") +
                MACRO_STRING(TypeAttribute_primitiveAssignFromLiteral) + " to a compound type!")

            AssignTypeAttribute(TypeAttribute_primitiveSimplyConvert,
                not type.isPrimitive, std::string("Cannot assign attribute: ") +
                MACRO_STRING(TypeAttribute_primitiveSimplyConvert) + " to a compound type!")

            Error("Unknown type attribute: " + *current->text);
        }
        while ((current = generator())->Match(Keyword::Comma));
#undef  AssignTypeAttribute
#undef  AssignTypeAttributeError
        if (not current->Match(Operator::BracketClose))
            Error("Expected an attribute identifier!");
        current = generator();
    }

    if (current->Match(Keyword::End)) {
        types.emplace(type.typeName, std::move(type));
        return;
    }
    if (not current->Match(Operator::BraceOpen)) {
        Error("Expected an opening bracket or block end after type name!");
    }

    // since members and methods are very similar to variables and functions they can use the same functions
    RunSyntaxAnalysis(generator, true, &type);

    if (not type.isPrimitive and type.members.empty()) {
        Error("Member-less complex types are prohibited!");
    }

    types.emplace(type.typeName, std::move(type));
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
        uint64_t memberSize, memberAlignment;
        if (n.typeMeta().pointerLevel > 0 or n.typeMeta().isReference) {
            memberSize = memberAlignment = Options::addressSize;
        }
        else {
            auto [size, alignment] = CalculateTypeSize(types[n.typeName()]);
            memberSize = size;
            memberAlignment = alignment;
        }

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
        for (auto& n : types) {
            currentCount += (CalculateTypeSize(n.second).first == 0);
        }

        if (currentCount == 0) {
            break;
        }
        if (currentCount == lastCount) {
            Error("Type size calculation deadlock!");
        }
        lastCount = currentCount;
    }
}

void ResolveCallParameterTypes(ParserFunctionMethod& called) {
    for (auto& n : called.parameters) {
        if (not types.contains(n.typeName()))
            Error("Type: " + n.typeName() + " used in parameter not found!");
        n.typeObject = &types[n.typeName()];
    }
}

void ResolveParameterTypes() {
    for (auto& n : types)
        for (auto& m : n.second.methods)
            ResolveCallParameterTypes(m);

    for (auto& n : functions)
        for (auto& m : n.second)
            ResolveCallParameterTypes(m);
}