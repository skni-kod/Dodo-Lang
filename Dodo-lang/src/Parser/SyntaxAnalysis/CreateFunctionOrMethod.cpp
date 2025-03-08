#include "AnalysisInternal.hpp"

ParserFunctionMethodObject CreateMethodOrFunction(Generator<LexerToken*>& generator,
    const ParserValueTypeObject& type, LexerToken* identifier, uint32_t operatorType) {

    ParserFunctionMethodObject output;

    auto* current = generator();
    while (not current->MatchOperator(Operator::BracketClose)) {
        bool isMutable = false;
        if (current->type == Token::Keyword) {
            if (current->kw == Keyword::Let) {
                current = generator();
            }
            else if (current->kw == Keyword::Mut) {
                current = generator();
                isMutable = true;
            }
            else {
                ParserError("Unexpected mutability keyword in parameter!");
            }
        }

        if (current->type != Token::Identifier) {
            ParserError("Expected parameter type identifier!");
        }

        auto [thingType, thingIdentifier] = ParseValueType(generator, current);
        thingType.type.isMutable = isMutable;
        output.arguments.emplace_back(*thingIdentifier->text, thingType, TypeObjectValue());

        current = generator();
        if (not current->MatchKeyword(Keyword::Comma) and not current->MatchOperator(Operator::BracketClose)) {
            ParserError("Expected a comma or bracket close after parameter!");
        }
    }
    if (not generator()->MatchOperator(Operator::BraceOpen)) {
        ParserError("Expected an opening brace after function prototype!");
    }
    uint32_t braceLevel = 0;
    while (not (current = generator())->MatchOperator(Operator::BraceClose) or braceLevel != 0) {
        output.instructions.emplace_back(ParseInstruction(generator, current, &braceLevel));
    }

    return std::move(output);
}