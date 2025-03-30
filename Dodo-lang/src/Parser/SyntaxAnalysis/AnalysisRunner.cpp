#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"
#include "ParserVariables.hpp"

std::pair<ParserValueTypeObject, LexerToken*> ParseValueType(Generator<LexerToken*>& generator, LexerToken* first) {
    ParserValueTypeObject output;
    LexerToken* current;

    if (not first->MatchKeyword(Keyword::Void)) {
        if (first->MatchKeyword(Keyword::Mut)) {
            first = generator();
            output.type.isMutable = true;
        }
        if (first->type != Token::Identifier) {
            ParserError("Expected an identifier!");
        }
        output.typeName = first->text;

        while ((current = generator())->MatchOperator(Operator::Multiply, Operator::Dereference)) {
            output.type.pointerLevel++;
        }
    }
    return {output, current};
}

bool IsOperatorOverloadAllowed(uint32_t type) {
    switch (type) {
        case Operator::Assign:
        case Operator::Add:
        case Operator::Subtract:
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Power:
        case Operator::Modulo:
        case Operator::NOr:
        case Operator::BinNOr:
        case Operator::NAnd:
        case Operator::BinNAnd:
        case Operator::Macro:
        case Operator::Not:
        case Operator::BinNot:
        case Operator::Or:
        case Operator::BinOr:
        case Operator::And:
        case Operator::BinAnd:
        case Operator::XOr:
        case Operator::BinXOr:
        case Operator::Imply:
        case Operator::NImply:
        case Operator::BinImply:
        case Operator::BinNImply:
        case Operator::Lesser:
        case Operator::Greater:
        case Operator::Equals:
        case Operator::Index:
        case Operator::Increment:
        case Operator::Decrement:
        case Operator::ShiftRight:
        case Operator::ShiftLeft:
        case Operator::Brace:
        case Operator::Bracket:
            return true;
        default:
            return false;
    }
}

// the base runner for the analysis
// it finds the base keywords that define the start of the next structure
bool RunSyntaxAnalysis(Generator<LexerToken*>& generator, bool isInType, TypeObject* type) {
    // the compiler skips the given structure if it can in case of issues
    bool didFail = false;
    LexerToken* current = nullptr;
    while (generator or (isInType and not current->MatchOperator(Operator::BraceClose))) {
        current = generator();
        // TYPES
        if (current->MatchKeyword(Keyword::Type) or current->MatchKeyword(Keyword::Primitive)) {
            try {
                if (isInType) {
                    ParserError("Nested type declaration is prohibited!");
                }
                CreateType(generator, current);
                continue;
            }
            catch (ParserException& e) {
                didFail = true;
                while (not current->MatchOperator(Operator::BraceClose) and not current->MatchKeyword(Keyword::End)) {
                    current = generator();
                }
                continue;
            }
        }

        if (isInType and current->MatchOperator(Operator::BraceClose)) {
            return didFail;
        }

        // variable or member
        if (current->MatchKeyword(Keyword::Let)) {
            ParserMemberVariableParameter out;
            ParseExpression(generator, out.definition, {current});

            if (isInType) {
                type->members.push_back(std::move(out));
            }
            else {
                globalVariables.emplace(out.name(), std::move(out));
            }
        }
        // function or method
        else {
            // TODO: somehow unify type interpolation into 1 function with math parser
            auto [thingType, thingIdentifier] = ParseValueType(generator, current);

            if (isInType) {
                if (thingIdentifier->MatchKeyword(Keyword::Operator)) {
                    current = generator();
                    if (current->type != Token::Operator) {
                        ParserError("Expected an operator!");
                    }
                    if (not IsOperatorOverloadAllowed(current->op)) {
                        ParserError("This operator cannot be overloaded!");
                    }
                    type->methods.emplace_back(std::move(CreateMethodOrFunction(generator, thingType, thingIdentifier, true, current->op)));
                    type->methods.back().overloaded = current->op;
                }
                else {
                    type->methods.emplace_back(std::move(CreateMethodOrFunction(generator, thingType, thingIdentifier, true, Operator::None)));
                }
            }
            else {
                functions.emplace(*thingIdentifier->text, std::move(CreateMethodOrFunction(generator, thingType, thingIdentifier, false, Operator::None)));
            }
        }
    }
    return didFail;
}