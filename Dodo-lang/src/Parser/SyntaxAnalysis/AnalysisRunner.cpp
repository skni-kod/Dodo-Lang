#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"
#include "ParserVariables.hpp"

// the base runner for the analysis
// it finds the base keywords that define the start of the next structure
bool RunSyntaxAnalysis(Generator<const LexerToken*>& generator, bool isInType) {
    // the compiler skips the given structure if it can in case of issues
    bool didFail = false;

    while (generator) {
        const auto* current = generator();

        // BASIC TYPES
        if (current->MatchKeyword(Keyword::Type) or current->MatchKeyword(Keyword::Primitive)) {
            try {
                CreateType(generator, current);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not current->MatchOperator(Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }

        }
        // FUNCTIONS
        if (current->type == Token::Identifier or current->MatchKeyword(Keyword::Void)) {
            // TODO: modify this for global variables and any function return type
            try {
                //CreateFunction(generator, current->value);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not current->MatchOperator(Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }
        }
        // GLOBAL VARIABLES
        if (current->MatchKeyword(Keyword::Let) or current->MatchKeyword(Keyword::Mut)) {
            try {
                // TODO: figure out how to insert it via the function with checks
                //globalVariablesOLD.map.insert(CreateVariable(generator, current->value, true));
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (not current->MatchOperator( Operator::BraceClose)) {
                    current = generator();
                }
                continue;
            }
        } 
    }
    return didFail;
}