#include "AnalysisInternal.hpp"

FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken) {
    FunctionInstruction instruction;

    // return statement
    if (firstToken->type == LexicalToken::Type::keyword and firstToken->value == "return") {
        instruction.Variant.returnInstruction = new ReturnInstruction();
        instruction.type = FunctionInstruction::Type::returnValue;
        instruction.Variant.returnInstruction->expression = ParseMath(generator);
        return std::move(instruction);
    }

    // declaration, assignment or function call
    if (firstToken->type == LexicalToken::Type::identifier) {
        const LexicalToken* current = generator();
        // declaration
        if (current->type == LexicalToken::identifier) {
            instruction.Variant.declarationInstruction = new DeclarationInstruction();
            instruction.type = FunctionInstruction::Type::declaration;
            instruction.Variant.declarationInstruction->typeName = firstToken->value;
            instruction.Variant.declarationInstruction->name = current->value;
            current = generator();
            // with value
            if (current->type == LexicalToken::Type::operand and current->value == "=") {
                instruction.Variant.declarationInstruction->expression = ParseMath(generator);
            }
            else if (current->type == LexicalToken::Type::expressionEnd) {
                instruction.Variant.declarationInstruction->expression.nodeType = ParserValue::Node::empty;
            }
            return std::move(instruction);
        }
    }


    return std::move(instruction);
}