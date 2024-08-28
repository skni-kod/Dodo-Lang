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

    // declaration
    if (firstToken->value == "let" or firstToken->value == "mut") {
        const LexicalToken* current = generator();
        if (current->type != LexicalToken::identifier) {
            ParserError("Expected a typeName identifier after variable declaration!");
        }
        const std::string& typeName = current->value;
        current = generator();
        if (current->type == LexicalToken::identifier) {
            instruction.Variant.declarationInstruction = new DeclarationInstruction();
            instruction.type = FunctionInstruction::Type::declaration;
            instruction.Variant.declarationInstruction->typeName = typeName;
            instruction.Variant.declarationInstruction->name = current->value;
            current = generator();
            if (firstToken->value == "mut") {
                instruction.Variant.declarationInstruction->isMutable = true;
            }
            // with value
            if (current->type == LexicalToken::Type::operand and current->value == "=") {
                instruction.Variant.declarationInstruction->expression = ParseMath(generator);
            }
            else if (current->type == LexicalToken::Type::expressionEnd) {
                if (firstToken->value == "let") {
                    ParserError("Cannot create an immutable variable without specifying the value!");
                }
                instruction.Variant.declarationInstruction->expression.nodeType = ParserValue::Node::empty;
            }
            return std::move(instruction);
        }
        else {
            ParserError("Expected an identifier after variable type name!");
        }
    }

    // variable modification
    if (firstToken->type == LexicalToken::Type::identifier) {
        // get the = or the + before += and such
        const LexicalToken* current = generator();
        if (current->type != LexicalToken::Type::operand) {
            ParserError("Expected an operand after identifier!");
        }

        if (current->value == "(") {
            instruction.Variant.functionCallInstruction = new FunctionCallInstruction();
            instruction.type = FunctionInstruction::Type::functionCall;
            instruction.Variant.functionCallInstruction->functionName = firstToken->value;
            instruction.Variant.functionCallInstruction->arguments = ParseMath(generator, std::vector<const LexicalToken*> {firstToken, current}, false);
            return std::move(instruction);
        }

        instruction.Variant.valueChangeInstruction = new ValueChangeInstruction();
        instruction.type = FunctionInstruction::Type::valueChange;
        instruction.Variant.valueChangeInstruction->name = firstToken->value;

        if (current->value == "=") {
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator);
            return std::move(instruction);
        }

        if (current->value == "+=") {
            LexicalToken temp = {LexicalToken::Type::operand, "+"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return std::move(instruction);
        }
        if (current->value == "-=") {
            LexicalToken temp = {LexicalToken::Type::operand, "-"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return std::move(instruction);
        }
        if (current->value == "*=") {
            LexicalToken temp = {LexicalToken::Type::operand, "*"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return std::move(instruction);
        }
        if (current->value == "/=") {
            LexicalToken temp = {LexicalToken::Type::operand, "/"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return std::move(instruction);
        }


        ParserError("Unexpected operator after identifier!");
    }


    return std::move(instruction);
}