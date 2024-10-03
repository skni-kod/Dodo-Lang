#include "GenerateCodeInternal.hpp"
#include "Bytecode.hpp"
#include "GenerateCode.hpp"

#define MIN_16_BIT_FLOAT_PRECISE 0.000000059604644l
#define MIN_32_BIT_FLOAT_PRECISE 0.000000000000000000000000000000000000000000001401298l
#define MAX_16_BIT_FLOAT_PRECISE 65504.l
#define MAX_32_BIT_FLOAT_PRECISE 16777216.l

namespace {
    struct VariableContainer {
        VariableType type;
        std::string name;
        bool isMutable = true;

        VariableContainer(VariableType type, std::string name, bool isMutable = false) : type(type), name(name),
                                                                                         isMutable(isMutable) {}
    };

    struct VariableContainerVector {
        std::vector<std::vector<VariableContainer>> variables;

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
            for (auto& m: variables) {
                for (auto& n: m) {
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

// some variable names might repeat, they need to be counted
struct VarInstanceStruct {
    uint32_t instanceNumber = 0;
    uint32_t assignmentNumber = 0;

    uint32_t addInstance() {
        return ++instanceNumber;
        assignmentNumber = 0;
    }
};

MapWrapper<std::string, VarInstanceStruct> variableInstances;

std::string AddVariableInstance(std::string identifier) {
    if (variableInstances.isKey(identifier)) {
        auto& ins = variableInstances[identifier];
        ins.instanceNumber++;
        ins.assignmentNumber = 0;
        return identifier + "#" + std::to_string(ins.instanceNumber) + "-" + std::to_string(ins.assignmentNumber);
    }
    else {
        variableInstances.insert(identifier, {});
        auto& ins = variableInstances[identifier];
        return identifier + "#" + std::to_string(ins.instanceNumber) + "-" + std::to_string(ins.assignmentNumber);
    }
}

std::string GetVariableInstance(std::string identifier) {
    if (variableInstances.isKey(identifier)) {
        auto& ins = variableInstances[identifier];
        return identifier + "#" + std::to_string(ins.instanceNumber) + "-" + std::to_string(ins.assignmentNumber);
    }
    CodeGeneratorError("Invalid variable reference!");
    return "";
}

std::string ReassignVariableInstance(std::string identifier, const VariableType& type) {
    if (variableInstances.isKey(identifier)) {
        auto& ins = variableInstances[identifier];
        ins.assignmentNumber++;
        earlyVariables.add({type, GetVariableInstance(identifier), true});
        return identifier + "#" + std::to_string(ins.instanceNumber) + "-" + std::to_string(ins.assignmentNumber);
    }
    CodeGeneratorError("Invalid variable reference!");
    return "";
}


std::string BytecodeFunctionCall(const ParserValue& expression);

void HandleInstruction(const ParserFunction& function, const FunctionInstruction& instruction, uint64_t& counter);

uint64_t expressionCounter = 0;
uint64_t labelCounter = 0;

std::string CalculateBytecodeExpression(const ParserValue& expression, VariableType type) {
    if (expression.nodeType == ParserValue::Node::constant) {
        return "$" + *expression.value;
    }

    if (expression.nodeType == ParserValue::Node::variable) {
        const auto& var = earlyVariables.find(GetVariableInstance(*expression.value));
        return GetVariableInstance(*expression.value);
    }

    if (expression.nodeType == ParserValue::Node::operation) {
        if (expression.operationType == ParserValue::Operation::functionCall) {
            return BytecodeFunctionCall(expression);
        }
        else {
            std::string left = CalculateBytecodeExpression(*expression.left, type);
            std::string right = CalculateBytecodeExpression(*expression.right, type);
            bytecodes.emplace_back(expression.operationType, right, left, expressionCounter++, type);
            return AddVariableInstance(EXPRESSION_SIGN);
        }
    }

    if (expression.nodeType == ParserValue::Node::empty) {
        return "0";
    }

    CodeGeneratorError("Invalid node type!");
    return "";
}

VariableType NegotiateOperationType(const ParserValue& expression) {
    if (expression.nodeType == ParserValue::Node::variable) {
        return earlyVariables.find(*expression.value).type;
    }

    if (expression.nodeType == ParserValue::Node::constant) {
        bool isFloat = false;
        for (auto& n: *expression.value) {
            if (n != '-' and n != '.' and n < '0' and n > '9') {
                CodeGeneratorError("Non numeric constants no supported!");
            }
            if (n == '.') {
                if (isFloat) {
                    CodeGeneratorError("Multiple dots in floating point value!");
                }
                isFloat = true;
            }
        }

        if (isFloat) {
            long double number = std::stold(*expression.value);
            if ((number <= MAX_16_BIT_FLOAT_PRECISE and number > MIN_16_BIT_FLOAT_PRECISE) or
                (number >= -MAX_16_BIT_FLOAT_PRECISE and number < -MIN_16_BIT_FLOAT_PRECISE)) {
                return {2, ParserType::Type::floatingPoint};
            }

            if ((number <= MAX_32_BIT_FLOAT_PRECISE and number > MIN_32_BIT_FLOAT_PRECISE) or
                (number >= -MAX_32_BIT_FLOAT_PRECISE and number < -MIN_32_BIT_FLOAT_PRECISE)) {
                return {4, ParserType::Type::floatingPoint};
            }

            return {8, ParserType::Type::floatingPoint};
        }

        if (expression.value->front() == '-') {
            int64_t number = std::stoll(*expression.value);
            if (number <= 127 and number >= -128) {
                return {1, ParserType::Type::signedInteger};
            }

            if (number <= 32767 and number >= -32768) {
                return {2, ParserType::Type::signedInteger};
            }

            if (number <= 2147483647 and number >= -2147483648) {
                return {4, ParserType::Type::signedInteger};
            }

            return {8, ParserType::Type::signedInteger};
        }

        uint64_t number = std::stoull(*expression.value);
        if (number <= 255) {
            return {1, ParserType::Type::unsignedInteger};
        }

        if (number <= 65535) {
            return {2, ParserType::Type::unsignedInteger};
        }

        if (number <= 4294967295) {
            return {4, ParserType::Type::unsignedInteger};
        }

        return {8, ParserType::Type::unsignedInteger};
    }

    if (expression.nodeType == ParserValue::Node::operation) {
        if (expression.operationType == ParserValue::Operation::functionCall) {
            auto& fun = parserFunctions[*expression.value];
            return VariableType(fun.returnType);
        }

        auto left = NegotiateOperationType(*expression.left);
        auto right = NegotiateOperationType(*expression.right);

        if (left.subtype != right.subtype) {
            CodeGeneratorError("Subtype mismatch in operation!");
        }

        return VariableType((left.size > right.size ? left.size : right.size),
                            (left.type > right.type ? left.type : right.type));
    }

    CodeGeneratorError("Invalid node type in bytecode generator size negotiation!");
    return {};
}

VariableType NegotiateOperationType(const ParserValue& first, const ParserValue& second) {
    auto left = NegotiateOperationType(first);
    auto right = NegotiateOperationType(second);

    if (left.subtype != right.subtype) {
        CodeGeneratorError("Subtype mismatch in operation!");
    }

    return VariableType((left.size > right.size ? left.size : right.size),
                        (left.type > right.type ? left.type : right.type));
}

void BytecodeReturn(const ReturnInstruction& instruction, const ParserFunction& function) {
    const auto& type = parserTypes[function.returnType];
    bytecodes.emplace_back(Bytecode::returnValue, CalculateBytecodeExpression(instruction.expression, VariableType(type,
                                                                                                                   VariableType::Subtype::value)),
                           VariableType(type, VariableType::Subtype::value));
}

void BytecodeDeclare(const DeclarationInstruction& instruction) {
    auto& type = parserTypes[instruction.typeName];
    std::string name = AddVariableInstance(instruction.name);
    earlyVariables.add({{type.size, type.type, instruction.subtype}, name, instruction.isMutable});
    bytecodes.emplace_back(Bytecode::declare, CalculateBytecodeExpression(instruction.expression,
                                                                          {type.size, type.type, instruction.subtype}),
                           name, VariableType(type.size, type.type));
}

void BytecodeDeclare(const ForLoopVariable& var) {
    auto& type = parserTypes[var.typeName];
    std::string name = AddVariableInstance(var.identifier);
    earlyVariables.add({{type.size, type.type, var.subtype}, name, true});
    bytecodes.emplace_back(Bytecode::declare,
                           CalculateBytecodeExpression(var.value, {type.size, type.type, var.subtype}),
                           name, VariableType(type.size, type.type));
}

void BytecodeAssign(const ValueChangeInstruction& instruction) {
    auto& type = earlyVariables.find(GetVariableInstance(instruction.name)).type;
    bytecodes.emplace_back(Bytecode::assign, CalculateBytecodeExpression(instruction.expression, type), "", type);
    bytecodes.back().target = ReassignVariableInstance(instruction.name, type);
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
            bytecodes.emplace_back(Bytecode::moveArgument,
                                   CalculateBytecodeExpression(*argument->left, {type.size, type.type}), n,
                                   VariableType(type.size, type.type));

            // get the next argument
            argument = argument->left.get();
        }
    }

    bytecodes.emplace_back(Bytecode::callFunction, instruction.functionName, "", VariableType());
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
            bytecodes.emplace_back(Bytecode::moveArgument,
                                   CalculateBytecodeExpression(*argument->left, {type.size, type.type}), n);

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
    // get ingredients for comp
    auto type = NegotiateOperationType(instruction.condition.left, instruction.condition.right);
    std::string left = CalculateBytecodeExpression(instruction.condition.left, type);
    std::string right = CalculateBytecodeExpression(instruction.condition.right, type);
    // do the comp
    bytecodes.emplace_back(Bytecode::compare, left, right, type);
    std::string label = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::jumpConditionalFalse, label, instruction.condition.type);
    counter++;
    for (; counter < function.instructions.size() and
           function.instructions[counter].type != FunctionInstruction::Type::endScope; counter++) {
        HandleInstruction(function, function.instructions[counter], counter);
    }
    BytecodePopLevel();
    bytecodes.emplace_back(Bytecode::addLabel, label);
}

void BytecodeElse(const ParserFunction& function, uint64_t& counter) {
    // check if the if before actually exists
    // there must be at least 5: compare, conditional jump, pushLevel, popLevel and addLabel
    if (bytecodes.size() < 5) {
        CodeGeneratorError("There cannot be a valid statement before this else!");
    }
    if (bytecodes.back().code != Bytecode::addLabel) {
        CodeGeneratorError("Invalid expression before else statement!");
    }

    bool found = false;
    for (int64_t n = bytecodes.size() - 2; n >= 0; n--) {
        if (bytecodes[n].code == Bytecode::jumpConditionalFalse and bytecodes[n].source == bytecodes.back().source) {
            found = true;
            break;
        }
    }
    if (not found) {
        CodeGeneratorError("Invalid else statement!");
    }

    // adding the label to jump at the end
    std::string label = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace(bytecodes.end() - 1, Bytecode::jump, label);
    counter++;
    for (; counter < function.instructions.size() and
           function.instructions[counter].type != FunctionInstruction::Type::endScope; counter++) {
        HandleInstruction(function, function.instructions[counter], counter);
    }
    BytecodePopLevel();
    bytecodes.emplace_back(Bytecode::addLabel, label);
}

void BytecodeWhile(const ParserFunction& function, const WhileInstruction& instruction, uint64_t& counter) {
    // add label to return for before every comparison
    std::string labelBefore = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::addLabel, labelBefore);
    // get ingredients for comp
    auto type = NegotiateOperationType(instruction.condition.left, instruction.condition.right);
    std::string left = CalculateBytecodeExpression(instruction.condition.left, type);
    std::string right = CalculateBytecodeExpression(instruction.condition.right, type);
    // do the comp
    bytecodes.emplace_back(Bytecode::compare, left, right, type);
    std::string labelAfter = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::jumpConditionalFalse, labelAfter, instruction.condition.type);
    counter++;
    for (; counter < function.instructions.size() and
           function.instructions[counter].type != FunctionInstruction::Type::endScope; counter++) {
        HandleInstruction(function, function.instructions[counter], counter);
    }
    bytecodes.emplace_back(Bytecode::jump, labelBefore, instruction.condition.type);
    BytecodePopLevel();
    bytecodes.emplace_back(Bytecode::addLabel, labelAfter);
}

void BytecodeFor(const ParserFunction& function, const ForInstruction& instruction, uint64_t& counter) {
    // push level here so that these variables get deleted after the loop
    BytecodePushLevel();
    // now declare variables for the loop
    for (auto& n: instruction.variables) {
        BytecodeDeclare(n);
    }

    // add label to return for before every comparison
    std::string labelBefore = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::addLabel, labelBefore);
    // get ingredients for comp
    auto type = NegotiateOperationType(instruction.condition.left, instruction.condition.right);
    std::string left = CalculateBytecodeExpression(instruction.condition.left, type);
    std::string right = CalculateBytecodeExpression(instruction.condition.right, type);
    // do the comp
    bytecodes.emplace_back(Bytecode::compare, left, right, type);
    std::string labelAfter = options::jumpLabelPrefix + std::to_string(labelCounter++);
    bytecodes.emplace_back(Bytecode::jumpConditionalFalse, labelAfter, instruction.condition.type);
    counter++;
    for (; counter < function.instructions.size() and
           function.instructions[counter].type != FunctionInstruction::Type::endScope; counter++) {
        HandleInstruction(function, function.instructions[counter], counter);
    }
    // add after loop instructions here
    for (auto& n: instruction.instructions) {
        HandleInstruction(function, n, counter);
    }
    bytecodes.emplace_back(Bytecode::jump, labelBefore, instruction.condition.type);
    BytecodePopLevel();
    BytecodePopLevel();
    bytecodes.emplace_back(Bytecode::addLabel, labelAfter);
}

void HandleInstruction(const ParserFunction& function, const FunctionInstruction& instruction, uint64_t& counter) {
    currentlyGeneratedInstruction = &instruction;
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
        case FunctionInstruction::Type::elseStatement:
            BytecodeElse(function, counter);
            break;
        case FunctionInstruction::Type::whileStatement:
            BytecodeWhile(function, *instruction.variant.whileInstruction, counter);
            break;
        case FunctionInstruction::Type::forStatement:
            BytecodeFor(function, *instruction.variant.forInstruction, counter);
            break;
        default:
            CodeGeneratorError("Invalid instruction type in bytecode generator!");
            break;
    }
}

void GenerateFunctionStepOne(const ParserFunction& function) {
    expressionCounter = 0;
    earlyVariables.clear();
    variableInstances.map.clear();

    for (uint64_t n = 0; n < function.instructions.size(); n++) {
        HandleInstruction(function, function.instructions[n], n);
    }

    // adding size prefixes to things
    for (auto& n : bytecodes) {
        if (not n.source.empty()) {
            if (n.source.front() != '$') {
                n.source = n.type.GetPrefix() + n.source;
            }
        }
        if (not n.target.empty()) {
            if (n.target.front() != '$') {
                n.target = n.type.GetPrefix() + n.target;
            }
        }
    }

    if (options::informationLevel == options::InformationLevel::full) {
        std::cout << "INFO L3: Bytecodes for function: " << function.name << "(";
        // TODO: Add arguments here
        std::cout << ")\n";
        uint64_t k = 1;
        for (auto& n: bytecodes) {
            std::cout << "INFO L3: ";
            if (n.code == Bytecode::popLevel) {
                k--;
            }
            for (uint64_t m = 0; m < k; m++) {

                std::cout << "\t";
            }
            if (n.code == Bytecode::pushLevel) {
                k++;
            }
            std::cout << n;
        }
        std::cout << "INFO L3: Bytecode amount for this function: " << bytecodes.size() << "\n";
    }
}