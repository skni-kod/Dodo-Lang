#include "AnalysisInternal.hpp"

ParserFunctionMethod CreateMethodOrFunction(Generator<LexerToken*>& generator,
    const ParserValueTypeObject& type, LexerToken* identifier, bool isMethod, Operator::Type operatorType) {

    ParserFunctionMethod output;
    output.returnType = type;
    output.isMethod = isMethod;
    output.isConstructor = operatorType == Operator::Constructor;
    output.isDestructor = operatorType == Operator::Destructor;
    output.isOperator = operatorType != Operator::None and not output.isConstructor and not output.isDestructor;

    auto* current = generator();

    if (current->Match(Operator::BracketOpen)) {
        current = generator();
    }
    while (not current->Match(Operator::BracketClose)) {
        if (current->Match(Keyword::Comma)) current = generator();

        if (not current->Match(Keyword::Let)) {
            Error("Expected a parameter!");
        }

        output.parameters.emplace_back();
        output.parameters.back().isMethod = isMethod;
        output.parameters.back().isOperator = operatorType != Operator::None;

        const auto result = ParseExpression(generator, output.parameters.back().definition, {current});

        current = result;
        if (not result->Match(Keyword::Comma) and not result->Match(Operator::BracketClose)) {
            Error("Expected a comma or bracket close after parameter!");
        }
    }

    if (not (current = generator())->Match(Operator::BraceOpen)) {
        if (current->Match(Keyword::Const))
            output.isConst = false;
        else
            Error("Expected an opening brace after function prototype!");
    }
    uint32_t braceLevel = 0;
    while (not (current = generator())->Match(Operator::BraceClose) or braceLevel != 0) {
        output.instructions.emplace_back(ParseInstruction(generator, current, &braceLevel));
        if (output.instructions.back().type == Instruction::Else) {
            output.instructions.emplace_back(Instruction::BeginScope);
            braceLevel++;
        }
    }

    if (identifier != nullptr and identifier->type == Token::Identifier) output.name = identifier->string;

    return std::move(output);
}