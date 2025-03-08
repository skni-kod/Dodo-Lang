#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"
#include "ParserVariables.hpp"

std::pair<ParserValueTypeObject, LexerToken*> ParseValueType(Generator<LexerToken*>& generator, LexerToken* first) {
    ParserValueTypeObject output;

    if (not first->MatchKeyword(Keyword::Void)) {
        output.typeName = *first->text;
    }
    auto current = generator();
    if (current->type == Token::Operator) {
        switch (current->op) {
            case Operator::Multiply:
                output.type.subType = Subtype::pointer;
            output.type.pointerLevel = 1;
            while ((current = generator())->MatchOperator(Operator::Multiply)) {
                output.type.pointerLevel++;
            }
            break;
            // TODO: what to do with references
            default:
                ParserError("Invalid type modifier!");
            break;
        }
    }
    else {
        if (not first->MatchKeyword(Keyword::Void)) {
            output.type.subType = Subtype::value;
        }

    }
    return {std::move(output), current};
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

        bool isFunction = false;
        bool isMutable = false;
        bool isOperator = false;
        uint32_t operatorId = 0;

        if (current->MatchKeyword(Keyword::Mut)) {
            isMutable = true;
            current = generator();
        }
        else if (current->MatchKeyword(Keyword::Let)) {
            current == generator();
        }

        if (current->type != Token::Identifier and not current->MatchKeyword(Keyword::Void)) {
            ParserError("Expected a type identifier!");
        }

        auto [thingType, thingIdentifier] = ParseValueType(generator, current);
        thingType.type.isMutable = isMutable;

        if (thingIdentifier->type != Token::Identifier) {
            if (isInType and thingIdentifier->MatchKeyword(Keyword::Operator)) {
                isOperator = true;
                current = generator();
                if (current->type != Token::Operator) {
                    ParserError("Expected an operator after operator overload keyword!");
                }
                if (not IsOperatorOverloadAllowed(current->op)) {
                    ParserError("This operator cannot be overloaded!");
                }
                operatorId = current->type;
            }
            else {
                ParserError("Expected an identifier after value type!");
            }
        }

        current = generator();

        if (current->MatchOperator(Operator::BracketOpen)) {
            // function or method
            if (isInType) {
                type->methods.emplace_back(std::move(CreateMethodOrFunction(generator, thingType, thingIdentifier, operatorId)));
            }
            else {
                unpreparedFunctions.emplace(*thingIdentifier->text, std::move(CreateMethodOrFunction(generator, thingType, thingIdentifier, operatorId)));
            }
        }
        else {
            if (isOperator) {
                ParserError("Cannot create a variable of operator overload!");
            }
            if (thingType.typeName.empty() and thingType.type.pointerLevel == 0) {
                ParserError("Cannot create a void type variable!");
            }
            // variable
            if (current->MatchOperator(Operator::Assign)) {
                ParserError("Default values for variables not implemented!");
            }

            if (not current->MatchKeyword(Keyword::End)) {
                ParserError("Unexpected token after variable name!");
            }

            if (isInType) {
                ParserTypeObjectMember member;
                member.type = thingType.type;
                member.memberName = std::move(*thingIdentifier->text);
                member.memberTypeName = std::make_unique<std::string>(thingType.typeName);
                type->members.push_back(std::move(member));
            }
            else {
                ParserError("Global variables not implemented!");
            }
        }
    }
    return didFail;
}