#include "GenerateCodeInternal.hpp"
#include "Bytecode.hpp"
#include "GenerateCode.hpp"

namespace {
    struct VariableContainer {
        VariableType type;
        std::string name;
        bool isMutable = true;
        VariableContainer(VariableType type, std::string name, bool isMutable = false) : type(type), name(name), isMutable(isMutable) {}
    };

    struct VariableContainerVector {
        std::vector <std::vector<VariableContainer>> variables;
        void add(VariableContainer var) {
            variables.back().push_back(var);
        }
        void pushLevel() {
            variables.emplace_back();
        }
        void popLevel() {
            variables.pop_back();
        }
        void clear() {
            variables.clear();
            variables.resize(1);
        }
        VariableContainer& find(const std::string& name) {
            for (auto& m : variables) {
                for (auto& n : m) {
                    if (n.name == name) {
                        return n;
                    }
                }
            }
            CodeGeneratorError("Could not find variable: \"" + name + "\" during bytecode generation!");
            return variables.front().front();
        }
    };
}

VariableContainerVector earlyVariables;

std::string BytecodeFunctionCall(const ParserValue& expression);
void HandleInstruction(const ParserFunction& function, const FunctionInstruction& instruction, uint64_t& counter);

uint64_t expressionCounter = 0;
uint64_t labelCounter = 0;

std::string CalculateBytecodeExpression(const ParserValue& expression, VariableType type) {
    if (expression.nodeType == ParserValue::Node::constant) {
        return *expression.value;
    }

    if (expression.nodeType == ParserValue::Node::variable) {
        const auto& var = earlyVariables.find(*expression.value);
        if (var.type != type) {
            bytecodes.emplace_back(Bytecode::convert, *expression.value, var.type, type);
        }
        return *expression.value;
    }

    if (expression.nodeType == ParserValue::Node::operation) {
        if (expression.operationType == ParserValue::Operation::functionCall) {
            return BytecodeFunctionCall(expression);
        }
        else {
            std::string left =  CalculateBytecodeExpression(*expression.left,  type);
            std::string right = CalculateBytecodeExpression(*expression.right, type);
            bytecodes.emplace_back(expression.operationType, right, left, expressionCounter);
            return EXPRESSION_SIGN + std::to_string(expressionCounter++);
        }
    }

    if (expression.nodeType == ParserValue::Node::empty) {
        return "0";
    }

    CodeGeneratorError("Invalid node type!");
    return "";
}

void BytecodeReturn(const ReturnInstruction& instruction, const ParserFunction& function) {
    const auto& type = parserTypes[function.returnType];
    bytecodes.emplace_back(Bytecode::returnValue, CalculateBytecodeExpression(instruction.expression, {type.size, type.type}));
}

void BytecodeDeclare(const DeclarationInstruction& instruction) {
    auto& type = parserTypes[instruction.typeName];
    earlyVariables.add({{type.size, type.type, instruction.subtype}, instruction.name, instruction.isMutable});
    bytecodes.emplace_back(Bytecode::declare, CalculateBytecodeExpression(instruction.expression,{type.size, type.type, instruction.subtype}),
                           instruction.name, &type);
}

void BytecodeAssign(const ValueChangeInstruction& instruction) {
    bytecodes.emplace_back(Bytecode::assign, CalculateBytecodeExpression(instruction.expression, earlyVariables.find(instruction.name).type), instruction.name);
}

void BytecodePushLevel() {
    earlyVariables.pushLevel();
    bytecodes.emplace_back(Bytecode::pushLevel);
}

void BytecodePopLevel() {
    earlyVariables.popLevel();
    bytecodes.emplace_back(Bytecode::popLevel);
}

void BytecodeFunctionCallStandalone(const FunctionCallInstruction& instruction) {
    BytecodePushLevel();
    const ParserFunction& function = parserFunctions[instruction.functionName];
    if (not function.arguments.empty()) {
        bytecodes.emplace_back(Bytecode::prepareArguments, instruction.functionName);
        const auto* argument = &instruction.arguments;
        for (uint64_t n = 0; n < function.arguments.size(); n++) {
            if (argument == nullptr) {
                CodeGeneratorError("Function argument mismatch!");
                return;
            }
            if (argument->right == nullptr) {
                CodeGeneratorError("Invalid argument value!");
                return;
            }

            const auto& type = parserTypes[function.arguments[n].typeName];
            bytecodes.emplace_back(Bytecode::moveArgument, CalculateBytecodeExpression(*argument->left, {type.size, type.type}), n);

            // get the next argument
            argument = argument->left.get();
        }
    }

    bytecodes.emplace_back(Bytecode::callFunction, instruction.functionName, "");
    BytecodePopLevel();
}

std::string BytecodeFunctionCall(const ParserValue& expression) {
    BytecodePushLevel();
    const ParserFunction& function = parserFunctions[*expression.value];
    if (not function.arguments.empty()) {
        bytecodes.emplace_back(Bytecode::prepareArguments, *expression.value);
        const auto* argument = &expression;
        for (uint64_t n = 0; n < function.arguments.size(); n++) {
            if (argument == nullptr) {
                CodeGeneratorError("Function argument mismatch!");
                return "";
            }
            if (argument->right == nullptr) {
                CodeGeneratorError("Invalid argument value!");
                return "";
            }

            const auto& type = parserTypes[function.arguments[n].typeName];
            bytecodes.emplace_back(Bytecode::moveArgument, CalculateBytecodeExpression(*argument->left, {type.size, type.type}), n);

            // get the next argument
            argument = argument->left.get();
        }
    }

    if (function.returnType.empty()) {
        CodeGeneratorError("Function without return type in expression!");
    }

    const auto& type = parserTypes[function.returnType];
    std::string result = EXPRESSION_SIGN + std::to_string(expressionCounter++);
    bytecodes.emplace_back(Bytecode::callFunction, *expression.value, result, VariableType(type.size, type.type));
    BytecodePopLevel();
    return result;
}

void BytecodeIf(const ParserFunction& function, const IfInstruction& instruction, uint64_t& counter) {
    // TODO: add comparison type negotiation
    // get ingredients for comp
    std::string left  = CalculateBytecodeExpression(instruction.condition.left , {8, ParserType::Type::signedInteger});
    std::string right = CalculateBytecodeExpression(instruction.condition.right, {8, ParserType::Type::signedInteger});
    // do the comp
    bytecodes.emplace_back(Bytecode::compare, left, right);
    std::string label = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::jumpConditional, label, instruction.condition.type);
    BytecodePushLevel();
    counter++;
    for (;counter < function.instructions.size() and function.instructions[counter].type != FunctionInstruction::Type::endScope; counter++) {
        HandleInstruction(function, function.instructions[counter], counter);
    }
    BytecodePopLevel();
    bytecodes.emplace_back(Bytecode::addLabel, label);
}

void HandleInstruction(const ParserFunction& function, const FunctionInstruction& instruction, uint64_t& counter) {
    switch (instruction.type) {
        case FunctionInstruction::Type::returnValue:
            BytecodeReturn(*instruction.variant.returnInstruction, function);
            break;
        case FunctionInstruction::Type::declaration:
            BytecodeDeclare(*instruction.variant.declarationInstruction);
            break;
        case FunctionInstruction::Type::valueChange:
            BytecodeAssign(*instruction.variant.valueChangeInstruction);
            break;
        case FunctionInstruction::Type::beginScope:
            BytecodePushLevel();
            break;
        case FunctionInstruction::Type::endScope:
            BytecodePopLevel();
            break;
        case FunctionInstruction::Type::functionCall:
            BytecodeFunctionCallStandalone(*instruction.variant.functionCallInstruction);
            break;
        case FunctionInstruction::Type::ifStatement:
            BytecodeIf(function, *instruction.variant.ifInstruction, counter);
            break;
    }
}

void GenerateFunctionStepOne(const ParserFunction& function) {
    expressionCounter = 0;
    earlyVariables.clear();
    for (uint64_t n = 0; n < function.instructions.size(); n++) {
        HandleInstruction(function, function.instructions[n], n);
    }


    if (options::informationLevel == options::InformationLevel::full) {
        std::cout << "INFO L3: Bytecodes for function: " << function.name << "(";
        // TODO: Add arguments here
        std::cout << ")\n";
        for (auto& n : bytecodes) {
            std::cout << "INFO L3: " << n;
        }
    }
}