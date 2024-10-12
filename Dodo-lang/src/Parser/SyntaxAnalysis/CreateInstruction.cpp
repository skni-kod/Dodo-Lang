#include "AnalysisInternal.hpp"


bool IsComparisonOperand(const std::string& value) {
    if (value == "==" or value == "!=" or value == ">" or value == "<" or value == ">=" or value == "<=") {
        return true;
    }
    return false;
}

uint8_t GetComparisonOperandType(const std::string& value) {
    if (value == "==") {
        return ParserCondition::Condition::equals;
    }
    if (value == "!=") {
        return ParserCondition::Condition::notEquals;
    }
    if (value == ">") {
        return ParserCondition::Condition::greater;
    }
    if (value == ">=") {
        return ParserCondition::Condition::greaterEqual;
    }
    if (value == "<") {
        return ParserCondition::Condition::lesser;
    }
    if (value == "<=") {
        return ParserCondition::Condition::lesserEqual;
    }
    ParserError("Unknown comparison operand!");
    return 1;
}

FunctionInstruction CreateInstruction(Generator<const LexicalToken*>& generator, const LexicalToken* firstToken) {
    FunctionInstruction instruction;
    instruction.sourceFile = GetCurrentFile();
    instruction.sourceLine = GetCurrentLine();

    // return statement
    if (firstToken->type == LexicalToken::Type::keyword and firstToken->value == "return") {
        instruction.variant.returnInstruction = new ReturnInstruction();
        instruction.type = FunctionInstruction::Type::returnValue;
        instruction.variant.returnInstruction->expression = ParseMath(generator);
        return instruction;
    }

    // declaration
    if (firstToken->value == "let" or firstToken->value == "mut") {
        auto result = CreateVariable(generator, firstToken->value, false);
        instruction.variant.declarationInstruction = new DeclarationInstruction();
        instruction.type = FunctionInstruction::Type::declaration;
        instruction.variant.declarationInstruction->name = result.first;
        instruction.variant.declarationInstruction->content = std::move(result.second);
        return instruction;
    }

    // variable modification
    if (firstToken->type == LexicalToken::Type::identifier) {
        // get the = or the + before += and such
        const LexicalToken* current = generator();
        if (current->type != LexicalToken::Type::operand) {
            ParserError("Expected an operand after identifier!");
        }

        if (current->value == "(") {
            instruction.variant.functionCallInstruction = new FunctionCallInstruction();
            instruction.type = FunctionInstruction::Type::functionCall;
            instruction.variant.functionCallInstruction->functionName = firstToken->value;
            instruction.variant.functionCallInstruction->arguments = ParseMath(generator,
                                                                               std::vector<const LexicalToken*> {
                                                                                       firstToken, current}, false, 1);
            return instruction;
        }

        instruction.variant.valueChangeInstruction = new ValueChangeInstruction();
        instruction.type = FunctionInstruction::Type::valueChange;
        instruction.variant.valueChangeInstruction->name = firstToken->value;

        if (current->value == "=") {
            instruction.variant.valueChangeInstruction->expression = ParseMath(generator);
            return instruction;
        }

        if (current->value == "+=") {
            LexicalToken temp = {LexicalToken::Type::operand, "+"};
            instruction.variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "-=") {
            LexicalToken temp = {LexicalToken::Type::operand, "-"};
            instruction.variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "*=") {
            LexicalToken temp = {LexicalToken::Type::operand, "*"};
            instruction.variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
            return instruction;
        }
        if (current->value == "/=") {
            LexicalToken temp = {LexicalToken::Type::operand, "/"};
            instruction.variant.valueChangeInstruction->expression = ParseMath(generator, {firstToken, &temp});
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
            std::vector<const LexicalToken*> tokens;
            while (not IsComparisonOperand(current->value)) {
                tokens.push_back(current);
                current = generator();
            }
            instruction.type = FunctionInstruction::Type::ifStatement;
            instruction.variant.ifInstruction = new IfInstruction();
            instruction.variant.ifInstruction->condition.left = ParseMath(tokens);
            instruction.variant.ifInstruction->condition.type = GetComparisonOperandType(current->value);

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
            instruction.variant.ifInstruction->condition.right = ParseMath(tokens);
            return instruction;
        }

        // while statement
        if (firstToken->value == "while") {
            const LexicalToken* current = generator();
            if (current->type != LexicalToken::Type::operand or current->value != "(") {
                ParserError("Expected a bracket opening after keyword while!");
            }
            // left side
            current = generator();
            std::vector<const LexicalToken*> tokens;
            while (not IsComparisonOperand(current->value)) {
                tokens.push_back(current);
                current = generator();
            }
            instruction.type = FunctionInstruction::Type::whileStatement;
            instruction.variant.whileInstruction = new WhileInstruction();
            instruction.variant.whileInstruction->condition.left = ParseMath(tokens);
            instruction.variant.whileInstruction->condition.type = GetComparisonOperandType(current->value);

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
            instruction.variant.whileInstruction->condition.right = ParseMath(tokens);
            return instruction;
        }

        // for statement
        if (firstToken->value == "for") {
            const LexicalToken* current = generator();
            if (current->type != LexicalToken::Type::operand or current->value != "(") {
                ParserError("Expected a bracket opening after keyword for!");
            }

            instruction.type = FunctionInstruction::Type::forStatement;
            instruction.variant.forInstruction = new ForInstruction();


            // if there is a variable at all
            while (lastToken->type != LexicalToken::Type::expressionEnd) {
                current = generator();
                if (current->type != LexicalToken::Type::identifier) {
                    if (current->type == LexicalToken::Type::keyword and current->value == "mut") {
                        current = generator();
                    }
                    else {
                        ParserError("Expected a type identifier in first segment of for loop");
                    }
                }
                ForLoopVariable var;
                var.typeName = current->value;


                current = generator();
                if (current->type != LexicalToken::Type::identifier) {
                    ParserError("Expected a name after type in first segment of for loop");
                }
                var.identifier = current->value;

                current = generator();
                if (current->type == LexicalToken::Type::operand and current->value == "=") {
                    var.value = ParseMath(generator);
                }
                else if (current->type == LexicalToken::Type::expressionEnd) {
                    var.value.operationType = ParserValue::Node::constant;
                    var.value.value = std::make_unique<std::string>("0");
                }
                else {
                    ParserError("Expected an expression or an end of expression after loop variable declaration!");
                }

                instruction.variant.forInstruction->variables.emplace_back(std::move(var));
            }

            // now get the condition
            current = generator();
            std::vector<const LexicalToken*> tokens;
            while (not IsComparisonOperand(current->value)) {
                tokens.push_back(current);
                current = generator();
            }

            instruction.variant.forInstruction->condition.left = ParseMath(tokens);
            instruction.variant.forInstruction->condition.type = GetComparisonOperandType(current->value);

            // right side
            current = generator();
            tokens.clear();
            uint64_t bracketLevel = 0;
            while (bracketLevel != 0 or current->type != LexicalToken::Type::expressionEnd) {
                if (current->value == "(") {
                    bracketLevel++;
                }
                else if (current->value == ")") {
                    bracketLevel--;
                }
                tokens.push_back(current);
                current = generator();
            }
            instruction.variant.forInstruction->condition.right = ParseMath(tokens);

            // and now add the operations after this
            while (lastToken->value != ")") {
                instruction.variant.forInstruction->instructions.push_back(CreateInstruction(generator, generator()));
            }

            return instruction;
        }

        ParserError("Other keyword starting instructions net yet introduced!");
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