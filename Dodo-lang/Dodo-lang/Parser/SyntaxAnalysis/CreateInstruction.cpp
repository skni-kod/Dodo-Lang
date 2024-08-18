#include "AnalysisInternal.hpp"

FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken) {
    FunctionInstruction instruction;

    if (firstToken->type == LexicalToken::Type::keyword and firstToken->value == "return") {
        instruction.Variant.returnInstruction = new ReturnInstruction(generator);
        instruction.type = FunctionInstruction::Type::returnValue;
    }

    return instruction;
}