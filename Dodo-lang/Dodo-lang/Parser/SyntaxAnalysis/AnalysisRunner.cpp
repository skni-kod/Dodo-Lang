#include "SyntaxAnalysis.hpp"
#include "AnalysisInternal.hpp"

// the base runner for the analysis
// it finds the base keywords that define the start of the next structure
bool RunSyntaxAnalysis(Generator<const LexicalToken*>& generator) {
    // the compiler skips the given structure if it can in case of issues
    bool didFail = false;

    while (generator) {
        const LexicalToken* current = generator();
        if (current->value == "type") {
            try {
                CreateType(generator);
                continue;
            }
            catch (ParserException& e){
                didFail = true;
                while (current->type != LexicalToken::type::expressionEnd) {
                    current = generator();
                }
                continue;
            }

        }
    }
    return didFail;
}