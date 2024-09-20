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


uint64_t expressionCounter = 0;

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
            return "";
        }
        else {
            std::string left =  CalculateBytecodeExpression(*expression.left,  type);
            std::string right = CalculateBytecodeExpression(*expression.right, type);
            bytecodes.emplace_back(expression.operationType, right, left, expressionCounter);
            return EXPRESSION_SIGN + std::to_string(expressionCounter++);;
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

void BytecodeFunctionCall(const FunctionCallInstruction& instruction) {
    BytecodePushLevel();
    bytecodes.emplace_back(Bytecode::prepareArguments, instruction.functionName);
    bytecodes.emplace_back(Bytecode::callFunction, instruction.functionName);
    BytecodePopLevel();
}


void GenerateFunctionStepOne(const ParserFunction& function) {
    expressionCounter = 0;
    earlyVariables.clear();
    for (uint64_t n = 0; n < function.instructions.size(); n++) {
        const auto& current = function.instructions[n];
        switch (current.type) {
            case FunctionInstruction::Type::returnValue:
                BytecodeReturn(*current.variant.returnInstruction, function);
                break;
            case FunctionInstruction::Type::declaration:
                BytecodeDeclare(*current.variant.declarationInstruction);
                break;
            case FunctionInstruction::Type::valueChange:
                BytecodeAssign(*current.variant.valueChangeInstruction);
                break;
            case FunctionInstruction::Type::beginScope:
                BytecodePushLevel();
                break;
            case FunctionInstruction::Type::endScope:
                BytecodePopLevel();
                break;
            case FunctionInstruction::Type::functionCall:
                BytecodeFunctionCall(*current.variant.functionCallInstruction);
                break;
        }
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