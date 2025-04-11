#include "Assembly.hpp"

#include <utility>

void Processor::clear() {
    for (auto& n : registers) {
        n.content = {};
    }
    stack.clear();
}

Place Processor::get(AsmOperand& op) {
    if (op.op == Location::reg) {
        return {&registers[op.value.ui], Location::reg};
    }
    if (op.op == Location::sta) {
        // finding stuff on the stack is way harder
        if (op.value.offset == 0) return {static_cast <StackEntry*>(nullptr), Location::sta};
        for (auto& n : stack) {
            if (n.offset == op.value.offset) return {{&n}, Location::sta};
        }
        return {static_cast <StackEntry*>(nullptr), Location::sta};
    }
    CodeGeneratorError("Internal: invalid processor location 1!");
    return {};
}

AsmOperand Processor::getContent(AsmOperand& op, BytecodeContext& context) {
    if (op.op == Location::reg) {
        return registers[op.value.ui].content;
    }
    if (op.op == Location::sta) {
        // finding stuff on the stack is way harder
        if (op.value.offset == 0) return {};
        for (auto& n : stack) {
            if (n.offset == op.value.offset) return n.content;
        }
        return {};
    }
    CodeGeneratorError("Internal: invalid processor location 2!");
    return {};
}

Place::Place(Register* reg, const Location::Type where) : reg(reg), where(where) {}
Place::Place(StackEntry* sta, const Location::Type where) : sta(sta), where(where){}

AsmOperand::AsmOperand(BytecodeOperand op, BytecodeContext& context) {
    if (op.location == Location::Variable) {
        auto var = context.getVariableObject(op);
        this->op = op.location;
        if (var.type->isPrimitive and not var.meta.isReference and not var.meta.pointerLevel) {
            type = var.type->primitiveType;
            size = var.type->typeSize;
        }
        else {
            size = Options::addressSize;
            type = Type::address;
        }
        value = op.value;
    }
    else if (op.location == Location::Literal) {
        this->op = op.location;
        size = op.size;
        type = op.literalType;
        value = op.value;
    }
    else CodeGeneratorError("Unsupported operand type!");
}

AsmOperand::AsmOperand(BytecodeOperand op, BytecodeContext& context, Location::Type location, OperandValue value) {
    if (op.location == Location::Variable) {
        auto var = context.getVariableObject(op);
        this->op = location;
        if (var.type->isPrimitive and not var.meta.isReference and not var.meta.pointerLevel) {
            type = var.type->primitiveType;
            size = var.type->typeSize;
        }
        else {
            size = Options::addressSize;
            type = Type::address;
        }
        this->value = value;
    }
    else if (op.location == Location::Literal) {
        this->op = location;
        size = op.size;
        type = op.literalType;
        this->value = value;
    }
    else CodeGeneratorError("Unsupported operand type!");
}

AsmOperand::AsmOperand(ParserFunctionMethod* functionMethod) {
    op = Location::Label;
    labelType = function;
    value.function = functionMethod;
}

AsmOperand AsmOperand::CopyTo(Location::Type location, OperandValue value) const {
    AsmOperand op(*this);
    op.op = location;
    op.value = value;
    return op;
}

VariableObject& AsmOperand::object(BytecodeContext& context) const {
    if (op != Location::Variable) CodeGeneratorError("Internal: non variable object get!");
    switch (value.variable.type) {
        case VariableLocation::None:
            CodeGeneratorError("Internal: invalid variable type in object get!");
        case VariableLocation::Global:
            return globalVariableObjects[value.variable.number];
        case VariableLocation::Local:
            return context.localVariables[value.variable.level][value.variable.number];
        case VariableLocation::Temporary:
            return context.temporaries[value.variable.number];
        default:
            CodeGeneratorError("Internal: could not find a variable object!");
        return globalVariableObjects[0];
    }
}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, AsmOperand location, AsmOperand value) : isFixedLocation(true), isInput(isInput), fixedLocation(location), value(value) {}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, uint8_t operandNumber, AsmOperand value) : isFixedLocation(false), isInput(isInput), operandNumber(operandNumber), value(value) {}

AsmInstructionVariant::AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        std::vector <AsmInstructionResultInput> resultsAndInputs)  :
        code(code), minimumVersion(minimumVersion), resultsAndInputs(std::move(resultsAndInputs)) {}

AsmInstructionVariant::AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, std::vector<RegisterRange> allowedRegisters,
        std::vector<AsmInstructionResultInput> resultsAndInputs) :
        code(code), minimumVersion(minimumVersion), op1(op1), allowedRegisters(std::move(allowedRegisters)), resultsAndInputs(std::move(resultsAndInputs)) {}


AsmInstructionVariant::AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2, std::vector<RegisterRange> allowedRegisters,
        std::vector<AsmInstructionResultInput> resultsAndInputs) :
        code(code), minimumVersion(minimumVersion), op1(op1), op2(op2), allowedRegisters(std::move(allowedRegisters)), resultsAndInputs(std::move(resultsAndInputs)) {}

AsmInstructionVariant::AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2, AsmOpDefinition op3, std::vector<RegisterRange> allowedRegisters,
        std::vector<AsmInstructionResultInput> resultsAndInputs) :
        code(code), minimumVersion(minimumVersion), op1(op1), op2(op2), op3(op3), allowedRegisters(std::move(allowedRegisters)), resultsAndInputs(std::move(resultsAndInputs)) {}

AsmInstructionVariant::AsmInstructionVariant(uint16_t code, Options::ArchitectureVersion minimumVersion,
        AsmOpDefinition op1, AsmOpDefinition op2, AsmOpDefinition op3, AsmOpDefinition op4,
        std::vector<RegisterRange> allowedRegisters, std::vector<AsmInstructionResultInput> resultsAndInputs) :
        code(code), minimumVersion(minimumVersion), op1(op1), op2(op2), op3(op3), op4(op4), allowedRegisters(std::move(allowedRegisters)), resultsAndInputs(std::move(resultsAndInputs)) {}

bool Register::canBeLongStored(const VariableObject& variable) const {
    if (isReservedRegister) return false;
    if (variable.meta.pointerLevel + variable.meta.isReference and canStoreAddresses and canOperateOnAddresses) return true;
    if (not variable.type->isPrimitive) return false;
    switch (variable.type->primitiveType) {
        case Type::floatingPoint:
            if (not canStoreFloatingPointValues or not canOperateOnFloatingPointValues) return false;
            break;
        case Type::signedInteger:
            if (not canStoreSignedIntegers or not canOperateOnSignedIntegers) return false;
            break;
        case Type::unsignedInteger:
            if (not canStoreUnsignedIntegers or not canOperateOnUnsignedIntegers) return false;
            break;
        default: return false;
    }
    switch (variable.type->typeSize) {
        case 1:
            return operandSize8;
        case 2:
            return operandSize16;
        case 4:
            return operandSize32;
        case 8:
            return operandSize64;
        default: return false;
    }
}

bool Register::canBeStored(const VariableObject& variable) const {
    if (variable.meta.pointerLevel + variable.meta.isReference and canStoreAddresses) return true;
    if (not variable.type->isPrimitive) return false;
    switch (variable.type->primitiveType) {
        case Type::floatingPoint:
            if (not canStoreFloatingPointValues) return false;
        break;
        case Type::signedInteger:
            if (not canStoreSignedIntegers) return false;
        break;
        case Type::unsignedInteger:
            if (not canStoreUnsignedIntegers) return false;
        break;
        default: return false;
    }
    switch (variable.type->typeSize) {
        case 1:
            return operandSize8;
        case 2:
            return operandSize16;
        case 4:
            return operandSize32;
        case 8:
            return operandSize64;
        default: return false;
    }
}