#include "AnalysisInternal.hpp"

ParserFunctionMethodObject CreateMethodOrFunction(Generator<const LexerToken*>& generator,
    const ParserValueTypeObject& type, const LexerToken* identifier, uint32_t operatorType) {

    ParserFunctionMethodObject output;

    const auto* current = generator();
    while (not current->MatchOperator(Operator::BracketClose)) {
        // TODO: add let and mut here

        if (current->type != Token::Identifier) {
            ParserError("Expected parameter type identifier!");
        }

        auto [thingType, thingIdentifier] = ParseValueType(generator, current);
        output.arguments.emplace_back(*thingIdentifier->text, thingType, TypeObjectValue());

        current = generator();
        if (not current->MatchKeyword(Keyword::Comma) and not current->MatchOperator(Operator::BracketClose)) {
            ParserError("Expected a comma or bracket close after parameter!");
        }
    }



    while (not generator()->MatchOperator(Operator::BraceClose)) {}

    return std::move(output);
}