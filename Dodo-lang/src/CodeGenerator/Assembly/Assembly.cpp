#include "Assembly.hpp"

#include <GenerateCode.hpp>
#include <Parser.hpp>
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

AsmOperand& Processor::getContentRef(AsmOperand& op) {
    if (op.op == Location::reg) {
        return registers[op.value.ui].content;
    }
    if (op.op == Location::sta) {
        // finding stuff on the stack is way harder
        for (auto& n : stack) {
            if (n.offset == op.value.offset) return n.content;
        }
        CodeGeneratorError("Internal: invalid processor location for reference!");
    }
    CodeGeneratorError("Internal: invalid processor location 3!");
    return registers[0].content;
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

AsmOperand Processor::getLocation(AsmOperand& op) {
    if (op.op == Location::Variable) {
        for (uint16_t n = 0; n < registers.size(); n++) {
            if (registers[n].content == op) {
                return op.copyTo(Location::reg, n);
            }
        }
        for (auto& n : stack) {
            if (n.content == op) {
                OperandValue temp;
                temp.offset = n.offset;
                return n.content.copyTo(Location::sta, temp);
            }
        }
        CodeGeneratorError("Internal: variable not found in memory!");
    }
    return op;
}

AsmOperand Processor::pushStack(BytecodeOperand value, BytecodeContext& context) {
    if (value.location == Location::Variable) {
        auto var = context.getVariableObject(value);
        int32_t offset;
        if (var.meta.isReference or var.meta.pointerLevel) {
            if (stack.empty()) offset = Options::addressSize;
            else offset = stack.back().offset + Options::addressSize;
            if (offset % Options::addressSize) offset = (offset / Options::addressSize + 1) * Options::addressSize;
            stack.emplace_back(AsmOperand(value, context), -offset, Options::addressSize);
        }
        else {
            auto size = var.type->typeSize;
            if (stack.empty()) offset = size;
            else offset = stack.back().offset + size;
            if (offset % var.type->typeAlignment) offset = (offset / var.type->typeAlignment + 1) * var.type->typeAlignment;
            stack.emplace_back(AsmOperand(value, context), -offset, size);
        }
        return {stack.back().offset};
    }
    CodeGeneratorError("Internal: unsupported stack push!");
    return {};
}

AsmOperand Processor::tempStack(uint8_t size, uint8_t alignment) {
    if (not alignment) alignment = size;
    AsmOperand location;
    location.op = Location::sta;
    if (stack.empty()) {
        if (size <= alignment) location.value.offset = -alignment;
        else if (size % alignment != 0) location.value.offset = (-(size / alignment) - 1) * alignment;
        else location.value.offset = -size;
    }
    else CodeGeneratorError("Internal: somehow can't find a stack location!");
    return location;
}

Place::Place(Register* reg, const Location::Type where) : reg(reg), where(where) {}
Place::Place(StackEntry* sta, const Location::Type where) : sta(sta), where(where){}

AsmOperand::AsmOperand(Location::Type op, Type::TypeEnum type, bool useAddress, uint8_t size, OperandValue value)
    : op(op), type(type), useAddress(useAddress), size(size), value(value) {}

AsmOperand::AsmOperand(const int32_t stackOffset) {
    op = Location::sta;
    value.offset = stackOffset;
}

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

bool AsmOperand::operator==(const AsmOperand& target) const {
    return target.op == op and target.type == type and target.useAddress == useAddress and target.value.ui == value.ui and target.size == size;
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

AsmOperand AsmOperand::copyTo(Location::Type location, OperandValue value) const {
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

void AsmOperand::print(std::ostream& out, BytecodeContext& context) {
    switch (op) {
        case Location::Variable:
            out << "Variable: ";
        {
            auto& obj = object(context);
            if (value.variable.type != VariableLocation::Temporary) out << *obj.identifier;
            else out << "Temporary: " << std::to_string(value.variable.number);
        }
            break;
        case Location::Literal:
            out << "Literal:";
            switch (type) {
                case Type::address:
                    switch (Options::addressSize) {
                        case 1:
                            out << "fixed address: " << value.u8;
                        case 2:
                            out << "fixed address: " << value.u16;
                        case 4:
                            out << "fixed address: " << value.u32;
                        case 8:
                            out << "fixed address: " << value.u64;
                        default:
                            break;
                    }
                case Type::floatingPoint:
                    switch (size) {
                        case 2:
                            CodeGeneratorError("16 bit floats not supported in printing!");
                        case 4:
                            out << "floating point literal: " << value.f32;
                        case 8:
                            out << "floating point literal: " << value.f64;
                        default:
                            break;
                    }
                    break;
                case Type::signedInteger:
                    switch (size) {
                        case 1:
                            out << "signed integer literal: " << value.i8;
                        case 2:
                            out << "signed integer literal: " << value.i16;
                        case 4:
                            out << "signed integer literal: " << value.i32;
                        case 8:
                            out << "signed integer literal: " << value.i64;
                        default:
                            break;
                    }
                break;
                case Type::unsignedInteger:
                    switch (size) {
                        case 1:
                            out << "unsigned integer literal: " << static_cast <uint64_t>(value.u8);
                        case 2:
                            out << "unsigned integer literal: " << value.u16;
                        case 4:
                            out << "unsigned integer literal: " << value.u32;
                        case 8:
                            out << "unsigned integer literal: " << value.u64;
                        default:
                            break;
                    }
            }
            break;
        case Location::String:
            out << "String: " << *passedStrings[value.string];
            break;
        case Location::Label:
            out << "";
            break;
        case Location::Memory:
            out << "";
            break;
        case Location::Offset:
            out << "";
            break;
        case Location::Zeroed:
            out << "";
            break;
        default:
            CodeGeneratorError("Internal: unhandled operand print case!");
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

void Processor::assignVariable(const AsmOperand variable, const AsmOperand assignedLocation, const AsmOperand value) {
    for (auto& n : registers) {
        if (n.content == variable) n.content = {};
        if (assignedLocation.op == Location::reg and assignedLocation.value.reg == n.number) n.content = value;
    }
    for (auto& n : stack) {
        if (n.content == variable) n.content = {};
        if (assignedLocation.op == Location::sta and assignedLocation.value.offset == n.offset) n.content = value;
    }
}

void Processor::cleanUnusedVariables(BytecodeContext& context, uint32_t index) {
    for (auto& n : registers) {
        if (n.content.op == Location::Variable) {
            auto& obj = n.content.object(context);
            if (obj.lastUse <= index) n.content = {};
        }
    }
    for (auto& n : stack) {
        if (n.content.op == Location::Variable) {
            auto& obj = n.content.object(context);
            if (obj.lastUse <= index) n.content = {};
        }
    }
}