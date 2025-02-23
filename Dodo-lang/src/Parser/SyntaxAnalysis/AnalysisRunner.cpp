#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"
#include "ParserVariables.hpp"

// the base runner for the analysis
// it finds the base keywords that define the start of the next structure
bool RunSyntaxAnalysis(Generator<const LexerToken*>& generator) {
    // the compiler skips the given structure if it can in case of issues
    bool didFail = false;

    while (generator) {
        auto* current = generator();

        // BASIC TYPES
        if (MatchKeyword(current , Keyword::Type) or MatchKeyword(current , Keyword::Primitive)) {
            try {
                //CreateType(generator, current->value);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not MatchOperator(current, Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }

        }
        // FUNCTIONS
        if (current->type == Token::Identifier or MatchKeyword(current, Keyword::Void)) {
            // TODO: modify this for global variables and any function return type
            try {
                //CreateFunction(generator, current->value);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not MatchOperator(current, Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }
        }
        // GLOBAL VARIABLES
        if (MatchKeyword(current, Keyword::Let) or MatchKeyword(current, Keyword::Mut)) {
            try {
                // TODO: figure out how to insert it via the function with checks
                //globalVariablesOLD.map.insert(CreateVariable(generator, current->value, true));
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not MatchOperator(current, Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }
        } 
    }
    return didFail;
}

inline bool MatchKeyword(const LexerToken*& token, const uint64_t type) {
    if (token->type != Token::Keyword) {
        return false;
    }
    if (token->op == type) {
        return true;
    }
    return false;
}
inline bool MatchOperator(const LexerToken*& token, const uint64_t type) {
    if (token->type != Token::Operator) {
        return false;
    }
    if (token->op == type) {
        return true;
    }
    return false;
}