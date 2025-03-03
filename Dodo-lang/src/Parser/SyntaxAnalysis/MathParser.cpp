#include "AnalysisInternal.hpp"

bool IsLValue(std::vector <ParserTreeValue>& valueArray, const uint32_t start) {
    for (auto current = &valueArray[start];;) {
        if (current->isLValued) {
            if (current->next != 0) {
                current = &valueArray[current->next];
                continue;
            }
            return true;
        }
        return false;
    }
}

bool IsRightToLeftOrdered(const LexerToken* token) {
    if (token->type == Token::Operator) {
        switch (token->op) {
            case Operator::Increment:
            case Operator::Decrement:
            case Operator::Not:
            case Operator::BinNot:
            case Operator::Assign:
            case Operator::Address:
            case Operator::Dereference:
                return true;
            default:
                break;
        }
    }
    return false;
}

// pass operators only, cheers
bool IsSplittableOperator(const LexerToken* token) {
    switch (token->op) {
        case Operator::Brace:
        case Operator::BraceOpen:
        case Operator::BraceClose:
        case Operator::Bracket:
        case Operator::BracketOpen:
        case Operator::BracketClose:
        case Operator::Index:
        case Operator::IndexOpen:
        case Operator::IndexClose:
        case Operator::Macro:
        case Operator::Address:
        case Operator::Dereference:
            return false;
        default:
            return true;
    }
}

// TODO: DODODODODODODODODODODDODODODODODODODODODODODO change address and dereference to be operator types

ParserTreeValue ParseExpressionStep(std::vector <ParserTreeValue>& valueArray, std::pair<uint32_t, uint32_t> range, std::vector <LexerToken*>& tokens) {
    auto [start, end] = range;
    LexerToken* mostImportant = nullptr;
    uint32_t firstChosen = 0, lastChosen = 0;

    // searching through the array to find the element with the highest split priority
    for (uint32_t n = start; n < end; n++) {
        if (tokens[n]->type == Token::Operator and IsSplittableOperator(tokens[n])
            and (mostImportant == nullptr or mostImportant->op < tokens[n]->op)) {

            mostImportant = tokens[n];
            firstChosen = lastChosen = n;
        }
        else if (mostImportant != nullptr and *tokens[n] == *mostImportant) {
            lastChosen = n;
        }
    }

    ParserTreeValue out;

    // now we have found (or not) the most important splitting operand and can construct something out of it
    if (mostImportant != nullptr) {
        ParserError("Expression splitting not implemented!");
    }
    else {
        // that one is more complicated
        uint32_t length = end - start;

        if (length == 1) {
            // that should be the simplest case, right?
            if (tokens[start]->type == Token::Identifier) {
                // a variable
                out.operation = ParserOperation::Variable;
                out.identifier = tokens[start]->text;
                out.isLValued = true;
            }
            else if (tokens[start]->type == Token::String) {
                // a string literal
                out.operation = ParserOperation::String;
                out.identifier = tokens[start]->text;
            }
            else if (tokens[start]->type == Token::Number) {
                // a constant number
                out.operation = ParserOperation::Constant;
                out.literal = tokens[start];
            }
            else if (tokens[start]->type == Token::Keyword) {
                ParserError("Are keywords valid in expressions? Null maybe?");
            }
            else {
                ParserError("Invalid single token remained in expression!");
            }

        }
    }
    
    return std::move(out);
}

bool IsExpressionEndToken(const LexerToken* token) {
    switch (token->type) {
        case Token::Keyword:
            switch (token->kw) {
                case Keyword::End:
                case Keyword::Comma:
                    return true;
                default:
                    return false;
            }
        case Token::Operator:
            switch (token->op) {
                case Operator::BraceClose:
                case Operator::BracketClose:
                case Operator::IndexClose:
                case Operator::Assign:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

LexerToken* ParseExpression(Generator <LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <LexerToken*> tokens) {
    // first of all let's
    LexerToken* last = nullptr;
    uint16_t bracketLevel = 0, braceLevel = 0, indexLevel = 0;
    for (uint32_t n = 0; n < tokens.size(); n++) {
        if (bracketLevel == 0 and braceLevel == 0 and indexLevel == 0 and IsExpressionEndToken(tokens[n])) {
            if (n != tokens.size() - 1) {
                ParserError("Invalid expression!");
            }
            last = tokens.back();
            tokens.pop_back();
        }

        if (tokens[n]->type == Token::Operator) {
            switch (tokens[n]->op) {
                case Operator::BraceOpen:
                    braceLevel++;
                break;
                case Operator::BracketOpen:
                    bracketLevel++;
                break;
                case Operator::IndexOpen:
                    indexLevel++;
                break;
                case Operator::BraceClose:
                    if (braceLevel == 0) {
                        ParserError("Invalid braces!");
                    }
                    braceLevel--;
                    break;
                case Operator::BracketClose:
                    if (bracketLevel == 0) {
                        ParserError("Invalid brackets!");
                    }
                    bracketLevel--;
                break;
                case Operator::IndexClose:
                    if (indexLevel == 0) {
                        ParserError("Invalid index brackets!");
                    }
                    indexLevel--;
                break;
                default:
                    break;
            }
        }

        if (n == tokens.size() - 1) {
            tokens.push_back(generator());
        }
    }

    if (tokens.size() == 0) {
        ParserError("Empty expression!");
    }

    // finding dereference operators
    // TODO: make this more reliable, cases for ++, -- and such
    for (int32_t n = 0; n < tokens.size(); n++) {
        if (tokens[n]->MatchOperator(Operator::Multiply)) {
            if (n >= 2 and tokens[n - 1]->type == Token::Operator and tokens.size() > n - 2) {
                tokens[n]->op = Operator::Dereference;
            }
        }
    }

    // now the entire expression bounds are done, and it can be parsed into a tree and compressed to size
    valueArray.reserve(valueArray.size() + tokens.size());
    valueArray.emplace_back();
    uint32_t backIndex = valueArray.size() - 1;
    valueArray[backIndex] = ParseExpressionStep(valueArray, {0, tokens.size()}, tokens);
    valueArray.shrink_to_fit();

    return last;
}