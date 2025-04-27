#include <Bytecode.hpp>

#include "AnalysisInternal.hpp"

// instruction format
// None     -
// Assign   - <lvalue> = <rvalue> ;
// Declare  - (let / mut) <type> <name> {; / = <rvalue> ;}
// Return   - return {; / <rvalue> ;}
// Call     - <lvalue> (<rvalue, rvalue, ...>) ;
// Syscall  - syscall (<number>, <rvalue, rvalue, ...>) ;
// If       - if (<rvalue>) {
// Else     - else {
// ElseIf   - else if (<rvalue>)
// Switch   - switch (lvalue) {
// While    - while (rvalue) {
// For      - for (<declare>; <rvalue>; <assign>) {
// Do       - do {
// Break    - break;
// Continue - continue;

// TODO: change declarations to be parsed in expressions
ParserInstructionObject ParseInstruction(Generator<LexerToken*>& generator, LexerToken* first, uint32_t* braceCounter) {
    ParserInstructionObject output;

    uint8_t keyword1 = Keyword::None;
    uint8_t keyword2 = Keyword::None;

    auto* current = first;
    LexerToken* aux = current;
    if (current->type == Token::Keyword) {
        keyword1 = current->kw;
        if (keyword1 != Keyword::Do and keyword1 != Keyword::Let and keyword1 != Keyword::Mut) {
            current = generator();
            if (current->type == Token::Keyword) {
                keyword2 = current->kw;
                if (keyword2 != Keyword::End) {
                    current = generator();
                }
            }
        }
    }

    switch (keyword1) {
        case Keyword::Return: {
            // valueless return
            output.type = Instruction::Return;
            if (keyword2 == Keyword::End) {
                return std::move(output);
            }
            if (keyword2) {
                ParserError("Unexpected keyword after return!");
            }
            // return with value
            if (const auto result = ParseExpression(generator, output.valueArray, {current}); not result->MatchKeyword(Keyword::End)) {
                ParserError("Expected a ';' after expression!");
            }
            return std::move(output);
        }

        case Keyword::If:{
            if (keyword2) {
                ParserError("Expected an opening bracket, not a keyword!");
            }
            output.type = Instruction::If;
            // return with value
            auto result = ParseExpression(generator, output.valueArray, {});
            if (not result->MatchOperator(Operator::BracketClose)) {
                ParserError("Expected a ')' after if expression!");
            }
            return std::move(output);
        }

        case Keyword::Else:
            if (keyword2 == Keyword::If) {
                if (not current->MatchOperator(Operator::BracketOpen)) {
                    ParserError("Expected an opening bracket!");
                }
                output.type = Instruction::ElseIf;
                if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchOperator(Operator::BracketClose)) {
                    ParserError("Expected a closing bracket after expression!");
                }
                return std::move(output);
            }
            if (not current->MatchOperator(Operator::BraceOpen)) {
                ParserError("Expected an opening bracket!");
            }
            output.type = Instruction::Else;
            return std::move(output);

        case Keyword::Switch:
            ParserError("Due to their mind boggling mathematics switches are not implemented yet!");
            if (keyword2 or not current->MatchOperator(Operator::BracketOpen)) {
                ParserError("Expected an opening bracket!");
            }
            output.type = Instruction::Switch;
            if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchOperator(Operator::BracketClose)) {
                ParserError("Expected a closing bracket after expression!");
            }
            return std::move(output);

        case Keyword::While:
            if (keyword2 or not current->MatchOperator(Operator::BracketOpen)) {
                ParserError("Expected an opening bracket!");
            }
            output.type = Instruction::While;
            if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchOperator(Operator::BracketClose)) {
                ParserError("Expected a closing bracket after expression!");
            }
            return std::move(output);

        case Keyword::For:
            if (keyword2 or not current->MatchOperator(Operator::BracketOpen)) {
                ParserError("Expected an opening bracket!");
            }
            output.type = Instruction::For;
            if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchKeyword(Keyword::End)) {
                ParserError("Expected a ';' after expression!");
            }
            output.expression2Index = output.valueArray.size();
            if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchKeyword(Keyword::End)) {
                ParserError("Expected a ';' after expression!");
            }
            output.expression3Index = output.valueArray.size();
            if (const auto result = ParseExpression(generator, output.valueArray, {}); not result->MatchOperator(Operator::BracketClose)) {
                ParserError("Expected a closing bracket after expression!");
            }
            return std::move(output);

        case Keyword::Do:
            output.type = Instruction::Do;
            return std::move(output);

        case Keyword::Break:
            if (keyword2 != Keyword::End) {
                ParserError("Expected a ';' after break!");
            }
            output.type = Instruction::Break;
            return std::move(output);

        case Keyword::Continue:
            if (keyword2 != Keyword::End) {
                ParserError("Expected a ';' after continue!");
            }
            output.type = Instruction::Continue;
            return std::move(output);

        case Keyword::Syscall:
        case Keyword::None:
        case Keyword::Let:
        case Keyword::Mut:
        if (current->MatchOperator(Operator::BraceClose)) {
            output.type = Instruction::EndScope;
            (*braceCounter)--;
            return std::move(output);
        }
        if (current->MatchOperator(Operator::BraceOpen)) {
            output.type = Instruction::BeginScope;
            (*braceCounter)++;
            return std::move(output);
        }
        // expression
        output.type = Instruction::Expression;
        if (keyword1 == Keyword::Syscall) {
            if (const auto result = ParseExpression(generator, output.valueArray, {aux, current}); not result->MatchKeyword(Keyword::End)) {
                ParserError("Expected a ';' after expression!");
            }
        }
        else {
            if (const auto result = ParseExpression(generator, output.valueArray, {current}); not result->MatchKeyword(Keyword::End)) {
                ParserError("Expected a ';' after expression!");
            }
        }

        return std::move(output);
        default:
            ParserError("Malformed instruction!");
    }
    return std::move(output);
}