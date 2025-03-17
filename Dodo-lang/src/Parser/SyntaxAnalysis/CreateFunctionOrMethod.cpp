#include "AnalysisInternal.hpp"

ParserFunctionMethod CreateMethodOrFunction(Generator<LexerToken*>& generator,
    const ParserValueTypeObject& type, LexerToken* identifier, uint32_t operatorType) {

    ParserFunctionMethod output;

    auto* current = generator();
    if (current->MatchOperator(Operator::BracketOpen)) {
        current = generator();
    }
    while (not current->MatchOperator(Operator::BracketClose)) {
        if (not current->MatchKeyword(Keyword::Let)) {
            ParserError("Expected a parameter!");
        }

        output.parameters.emplace_back();
        const auto result = ParseExpression(generator, output.parameters.back().definition, {current});

        current = generator();
        if (not result->MatchKeyword(Keyword::Comma) and not result->MatchOperator(Operator::BracketClose)) {
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