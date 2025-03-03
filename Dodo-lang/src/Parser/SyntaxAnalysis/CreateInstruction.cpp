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

ParserFunctionMethodInstructionObject ParseInstruction(Generator<LexerToken*>& generator, LexerToken* first) {
    ParserFunctionMethodInstructionObject output;

    uint8_t keyword1 = Keyword::None;
    uint8_t keyword2 = Keyword::None;

    auto* current = first;

    if (current->type == Token::Keyword) {
        keyword1 = current->kw;
        current = generator();
    }
    if (current->type == Token::Keyword) {
        keyword2 = current->kw;
        if (current->kw != Keyword::End) {
            current = generator();
        }
    }

    switch (keyword1) {
        case Keyword::Return:
            // valueless return
            if (keyword2 == Keyword::End) {
                output.type = Instruction::Return;
                return std::move(output);
            }
            if (keyword2) {
                ParserError("Unexpected keyword after return!");
            }
            // return with value
            ParserError("Unimplemented!");
            break;
        case Keyword::If:
            if (keyword2) {
                ParserError("Expected an opening bracket, not a keyword!");
            }
            // if
            ParserError("Unimplemented!");
            break;
        case Keyword::Else:
            if (keyword2 == Keyword::If) {
                // else if
                ParserError("Unimplemented!");
            }
            else {
                if (keyword2) {
                    ParserError("Expected an opening brace, not a keyword!");
                }
                // else
                ParserError("Unimplemented!");
            }

            break;
        case Keyword::Syscall:
            if (keyword2) {
                ParserError("Expected an opening bracket, not a keyword!");
            }
            ParserError("Unimplemented!");
        break;
        case Keyword::Switch:
            ParserError("Due to their mind boggling mathematics switches are not implemented yet!");
            break;
        case Keyword::While:
            // while loop
            if (keyword2) {
                ParserError("Expected an opening bracket, not a keyword!");
            }
            ParserError("Unimplemented!");
            break;
        case Keyword::For:
            if (keyword2) {
                ParserError("Expected an opening bracket, not a keyword!");
            }
            // for loop
            ParserError("Unimplemented!");
            break;
        case Keyword::Do:
            if (keyword2) {
                ParserError("Expected an opening brace, not a keyword!");
            }
            // do
            ParserError("Unimplemented!");
            break;
        case Keyword::Break:
            if (keyword2 != Keyword::End) {
                ParserError("Expected a ';' after break!");
            }
            // break
            output.type = Instruction::Break;
            return std::move(output);
        case Keyword::Continue:
            if (keyword2 != Keyword::End) {
                ParserError("Expected a ';' after continue!");
            }
            // break
            output.type = Instruction::Continue;
            return std::move(output);
        case Keyword::None:
        case Keyword::Let:
        case Keyword::Mut:
            if (keyword2) {
                ParserError("Unexpected keyword after variable declaration!");
            }{
            // instruction declare, assign or call
            if (keyword1 != Keyword::None) {
                output.type = Instruction::Declare;
                ParserTreeValue temp;
                temp.operation = ParserOperation::Declaration;
                temp.next = 1;
                // TODO: finish implementing
                ParserError("Declaration not fully implemented!");
                //temp.type
            }
            else {
                output.type = Instruction::Assign;
            }

            // TODO: check if function
            auto result = ParseExpression(generator, output.valueArray, {current});
            if (result->MatchOperator(Operator::Assign)) {
                if (not IsLValue(output.valueArray, 0)) {
                    ParserError("Cannot assign to non-lvalue!");
                }

                output.rValueIndex = output.valueArray.size();
                result = ParseExpression(generator, output.valueArray, {current});

            }
            else if (result->MatchKeyword(Keyword::End)) {
                output.type = Instruction::Call;
            }
            else {
                ParserError("Unexpected token after expression!");
            }
            break;
        }
        default:
            ParserError("Malformed instruction!");
    }
    return std::move(output);
}