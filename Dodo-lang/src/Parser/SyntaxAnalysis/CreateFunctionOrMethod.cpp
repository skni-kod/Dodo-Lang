#include "AnalysisInternal.hpp"

ParserFunctionMethod CreateMethodOrFunction(Generator<LexerToken*>& generator,
    const ParserValueTypeObject& type, LexerToken* identifier, bool isMethod, uint32_t operatorType) {

    ParserFunctionMethod output;
    output.returnType = type;
    output.isMethod = isMethod;
    output.isOperator = operatorType != Operator::None;

    auto* current = generator();

    if (current->MatchOperator(Operator::BracketOpen)) {
        current = generator();
    }
    while (not current->MatchOperator(Operator::BracketClose)) {
        if (current->MatchKeyword(Keyword::Comma)) current = generator();

        if (not current->MatchKeyword(Keyword::Let)) {
            ParserError("Expected a parameter!");
        }

        output.parameters.emplace_back();
        output.parameters.back().isMethod = isMethod;
        output.parameters.back().isOperator = operatorType != Operator::None;

        const auto result = ParseExpression(generator, output.parameters.back().definition, {current});

        current = result;
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
        if (output.instructions.back().type == Instruction::Else) {
            output.instructions.emplace_back(Instruction::BeginScope);
            braceLevel++;
        }
    }

    if (identifier != nullptr and identifier->type == Token::Identifier) output.name = identifier->string;

    return std::move(output);
}