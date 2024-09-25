#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"
#include "ParserVariables.hpp"

// the base runner for the analysis
// it finds the base keywords that define the start of the next structure
bool RunSyntaxAnalysis(Generator<const LexicalToken*>& generator) {
    // the compiler skips the given structure if it can in case of issues
    bool didFail = false;

    while (generator) {
        const LexicalToken* current = generator();

        // BASIC TYPES
        if (current->value == "type") {
            try {
                CreateType(generator);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (current->type != LexicalToken::Type::expressionEnd) {
                    current = generator();
                }
                continue;
            }

        }
        // FUNCTIONS
        if (current->type == LexicalToken::Type::identifier and
            (parserTypes.isKey(current->value) or current->value == "void")) {
            // TODO: modify this for global variables and any function return type
            try {
                CreateFunction(generator, current->value);
                continue;
            }
            catch (__ParserException& e) {
                didFail = true;
                while (current->type != LexicalToken::Type::expressionEnd) {
                    current = generator();
                }
                continue;
            }
        }
    }
    return didFail;
}