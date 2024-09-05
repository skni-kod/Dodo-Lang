#include "AnalysisInternal.hpp"


bool IsComparisonOperand(const std::string& value) {
    if (value == "==" or value == "!=" or value == ">" or value == "<" or value == ">=" or value == "<=") {
        return true;
    }
    return false;
}

uint8_t GetComparisonOperandType(const std::string& value) {
    if (value == "==") {
        return ParserCondition::Type::equals;
    }
    if (value == "!=") {
        return ParserCondition::Type::notEquals;
    }
    if (value == ">") {
        return ParserCondition::Type::greater;
    }
    if (value == ">=") {
        return ParserCondition::Type::greaterEqual;
    }
    if (value == "<") {
        return ParserCondition::Type::lesser;
    }
    if (value == "<=") {
        return ParserCondition::Type::lesserEqual;
    }
    ParserError("Unknown comparison operand!");
    return 1;
}

FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken) {
    FunctionInstruction instruction;

    // return statement
    if (firstToken->type == LexicalToken::Type::keyword and firstToken->value == "return") {
        instruction.Variant.returnInstruction = new ReturnInstruction();
        instruction.type = FunctionInstruction::Type::returnValue;
        instruction.Variant.returnInstruction->expression = ParseMath(generator);
        return instruction;
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
            return instruction;
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
            instruction.Variant.functionCallInstruction->arguments = ParseMath(generator, std::vector<const LexicalToken*> {firstToken, current}, false, 1);
            return instruction;
        }

        instruction.Variant.valueChangeInstruction = new ValueChangeInstruction();
        instruction.type = FunctionInstruction::Type::valueChange;
        instruction.Variant.valueChangeInstruction->name = firstToken->value;

        if (current->value == "=") {
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator);
            return instruction;
        }

        if (current->value == "+=") {
            LexicalToken temp = {LexicalToken::Type::operand, "+"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "-=") {
            LexicalToken temp = {LexicalToken::Type::operand, "-"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "*=") {
            LexicalToken temp = {LexicalToken::Type::operand, "*"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "/=") {
            LexicalToken temp = {LexicalToken::Type::operand, "/"};
            instruction.Variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }

        ParserError("Unexpected operator after identifier!");
    }

    // keyword starting things
    if (firstToken->type == LexicalToken::keyword) {
        if (firstToken->value == "else") {
            instruction.type = FunctionInstruction::Type::elseStatement;
            return instruction;
        }

        // if statement
        if (firstToken->value == "if") {
            const LexicalToken* current = generator();
            if (current->type != LexicalToken::Type::operand or current->value != "(") {
                ParserError("Expected a bracket opening after keyword if!");
            }
            // left side
            current = generator();
            std::vector <const LexicalToken*> tokens;
            while (not IsComparisonOperand(current->value)) {
                tokens.push_back(current);
                current = generator();
            }
            instruction.type = FunctionInstruction::Type::ifStatement;
            instruction.Variant.ifInstruction = new IfInstruction();
            instruction.Variant.ifInstruction->condition.left = ParseMath(tokens);
            instruction.Variant.ifInstruction->condition.type = GetComparisonOperandType(current->value);

            // right side
            current = generator();
            tokens.clear();
            uint64_t bracketLevel = 0;
            while (bracketLevel != 0 or current->value != ")") {
                if (current->value == "(") {
                    bracketLevel++;
                }
                else if (current->value == ")") {
                    bracketLevel--;
                }
                tokens.push_back(current);
                current = generator();
            }
            instruction.Variant.ifInstruction->condition.right = ParseMath(tokens);
            return instruction;
        }
        // if statement
        if (firstToken->value == "while") {
            const LexicalToken* current = generator();
            if (current->type != LexicalToken::Type::operand or current->value != "(") {
                ParserError("Expected a bracket opening after keyword while!");
            }
            // left side
            current = generator();
            std::vector <const LexicalToken*> tokens;
            while (not IsComparisonOperand(current->value)) {
                tokens.push_back(current);
                current = generator();
            }
            instruction.type = FunctionInstruction::Type::whileStatement;
            instruction.Variant.whileInstruction = new WhileInstruction();
            instruction.Variant.whileInstruction->condition.left = ParseMath(tokens);
            instruction.Variant.whileInstruction->condition.type = GetComparisonOperandType(current->value);

            // right side
            current = generator();
            tokens.clear();
            uint64_t bracketLevel = 0;
            while (bracketLevel != 0 or current->value != ")") {
                if (current->value == "(") {
                    bracketLevel++;
                }
                else if (current->value == ")") {
                    bracketLevel--;
                }
                tokens.push_back(current);
                current = generator();
            }
            instruction.Variant.whileInstruction->condition.right = ParseMath(tokens);
            return instruction;
        }
        else {
            ParserError("Other keyword starting instructions net yet introduced!");
        }
    }

    if (firstToken->type == LexicalToken::Type::blockBegin) {
        instruction.type = FunctionInstruction::Type::beginScope;
        return instruction;
    }

    if (firstToken->type == LexicalToken::Type::blockEnd) {
        instruction.type = FunctionInstruction::Type::endScope;
        return instruction;
    }

    ParserError("Invalid instruction!");
    return instruction;
}