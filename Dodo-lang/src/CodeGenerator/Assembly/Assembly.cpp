#include "Assembly.hpp"

#include <utility>

void Processor::clear() {
    for (auto& n : registers) {
        n.content = {};
    }
    stack.clear();
}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, AsmOperand operand, OperandValue value) : isFixedLocation(true), isInput(isInput), fixedLocation(operand), result(value) {}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, uint8_t operandNumber, OperandValue value) : isFixedLocation(false), isInput(isInput), operandNumber(operandNumber), result(value) {}

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