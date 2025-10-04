#include "Assembly.hpp"

#include <GenerateCode.hpp>
#include <iostream>
#include <Parser.hpp>
#include <utility>
#include "X86_64Enums.hpp"

void Context::clearProcessor() {
    for (auto& n : registers) {
        n.content = {};
    }
    stack.clear();
}

Place Context::get(AsmOperand& op) {
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
    Error("Internal: invalid context location 1!");
    return {};
}

AsmOperand emptyContent = {};

AsmOperand& Context::getContentRef(AsmOperand& op) {
    if (op.op == Location::reg) {
        return registers[op.value.ui].content;
    }
    if (op.op == Location::off) {
        emptyContent = {};
        return emptyContent;
    }
    if (op.op == Location::sta) {
        // finding stuff on the stack is way harder
        for (auto& n : stack) {
            if (n.offset == op.value.offset) return n.content;
        }
        // if it is here, then we need to create the stack location again, and that will be a pain
        // first let's find the index for the thing
        uint32_t index;
        if (stack.empty()) index = 0;
        else if (stack.front().offset < op.value.offset) index = 0;
        else if (stack.back().offset > op.value.offset) index = stack.size() - 1;
        else for (index = 0; index < stack.size() - 1; index++) {
            if (stack[index].offset > op.value.offset and stack[index + 1].offset < op.value.offset) break;
        }
        // now we have the index and just need to check if it fits here
        std::cout << "Warning: using unchecked register location for now!\n";
        // TODO: add the check

        stack.emplace(stack.begin() + index, StackEntry({}, op.value.offset, op.size));
        return stack[index].content;
        //Error("Internal: invalid context location for reference!");
    }
    Error("Internal: invalid context location 3!");
    return emptyContent;
}

AsmOperand Context::getContent(AsmOperand& op) {
    if (op.op == Location::reg) {
        return registers[op.value.ui].content;
    }
    if (op.op == Location::off) {
        return {};
    }
    if (op.op == Location::sta) {
        // finding stuff on the stack is way harder
        if (op.value.offset == 0) return {};
        for (auto& n : stack) {
            if (n.offset == op.value.offset) return n.content;
        }
        return {};
    }
    Error("Internal: invalid context location 2!");
    return {};
}

AsmOperand Context::getContentAtOffset(int32_t offset) {
    for (const auto& n : stack) if (n.offset == offset) return n.content;
    return {};
}

AsmOperand& Context::getContentRefAtOffset(int32_t offset) {
    for (auto& n : stack) if (n.offset == offset) return n.content;
    // in that case it does not exist
    Error("Internal: cannot get content in stack!");
    return stack[0].content;
}

AsmOperand Context::getLocation(AsmOperand& op) {
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
        Error("Internal: variable not found in memory!");
    }
    return op;
}

AsmOperand Context::getLocationIfExists(AsmOperand& op) {
    if (op.op == Location::Variable) {
        for (uint16_t n = 0; n < registers.size(); n++) {
            if (registers[n].content == op) {
                return registers[n].content.copyTo(Location::reg, n);
            }
        }
        for (auto& n : stack) {
            if (n.content == op) {
                OperandValue temp;
                temp.offset = n.offset;
                return n.content.copyTo(Location::sta, temp);
            }
        }
    }
    return {};
}

AsmOperand Context::getLocationStackBias(AsmOperand& op) {
    if (op.op == Location::Variable) {
        for (auto& n : stack) {
            if (n.content == op) {
                OperandValue temp;
                temp.offset = n.offset;
                return n.content.copyTo(Location::sta, temp);
            }
        }
        for (auto n = 0; n < registers.size(); n++) {
            if (registers[n].content == op) {
                return op.copyTo(Location::reg, n);
            }
        }
        Error("Internal: variable not found in memory!");
    }
    return op;
}

AsmOperand Context::getLocationRegisterBias(AsmOperand& op) {
    return getLocation(op);
}

AsmOperand Context::pushStack(BytecodeOperand value, int32_t amount) {
    if (value.location == Location::Variable) {
        auto& var = getVariableObject(value);
        int32_t offset;
        if (var.meta.isReference or var.meta.pointerLevel) {
            int32_t size = amount * Options::addressSize;
            if (stack.empty()) offset = size;
            else offset = -stack.back().offset + size;
            if (offset % Options::addressSize) offset = (offset / Options::addressSize + 1) * Options::addressSize;
            stack.emplace_back(AsmOperand(value, *this), -offset, size);
        }
        else {
            int32_t size = var.type->typeSize * amount;
            if (stack.empty()) offset = size;
            else offset = -stack.back().offset + size;
            if (offset % var.type->typeAlignment) offset = (offset / var.type->typeAlignment + 1) * var.type->typeAlignment;
            stack.emplace_back(AsmOperand(value, *this), -offset, size);
        }
        return {stack.back().offset};
    }
    Error("Internal: unsupported stack push!");
    return {};
}

AsmOperand Context::pushStack(AsmOperand value, int32_t amount) {
    if (value.op == Location::Variable) {
        auto& var = value.object(*this);
        int32_t offset;
        if (var.meta.isReference or var.meta.pointerLevel) {
            int32_t size = amount * Options::addressSize;
            if (stack.empty()) offset = size;
            else offset = -stack.back().offset + size;
            if (offset % Options::addressSize) offset = (offset / Options::addressSize + 1) * Options::addressSize;
            stack.emplace_back(value, -offset, size);
        }
        else {
            int32_t size = var.type->typeSize * amount;
            if (stack.empty()) offset = size;
            else offset = -stack.back().offset + size;
            if (offset % var.type->typeAlignment) offset = (offset / var.type->typeAlignment + 1) * var.type->typeAlignment;
            stack.emplace_back(value, -offset, size);
        }
        OperandValue off;
        off.offset = stack.back().offset;
        return {value.copyTo(Location::sta, off)};
    }
    Error("Internal: unsupported stack push!");
    return {};
}

AsmOperand Context::pushStackTemp(uint32_t size, uint32_t alignment) {
    int32_t offset;
    if (stack.empty()) offset = size;
    else offset = -stack.back().offset + size;
    if (offset % alignment) offset = (offset / alignment + 1) * alignment;
    stack.emplace_back(AsmOperand(), -offset, size);
    return -offset;
}

AsmOperand Context::tempStack(uint8_t size, uint8_t alignment) {
    if (not alignment) alignment = size;
    AsmOperand location;
    location.op = Location::sta;
    if (stack.empty()) {
        if (size <= alignment) location.value.offset = -alignment;
        else if (size % alignment != 0) location.value.offset = (-(size / alignment) - 1) * alignment;
        else location.value.offset = -size;
    }
    else Error("Internal: somehow can't find a stack location!");
    return location;
}

bool Context::isRegisterCalleeSaved(uint8_t registerNumber) const {
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        if (Options::targetSystem == TARGET_SYSTEM_LINUX) {
            switch (registerNumber) {
            case x86_64::RBX:
            case x86_64::RSP:
            case x86_64::RBP:
            case x86_64::R12:
            case x86_64::R13:
            case x86_64::R14:
            case x86_64::R15:
                return true;
            default:
                return false;
            }
        }
    }
    Unimplemented();
}

AsmOperand Context::getFreeRegister(Type::TypeEnum valueType, uint16_t size, bool useCalleeSaved) const {
    for (auto& n : registers) {
        if (n.content.op != Location::None) continue;
        if (not useCalleeSaved and isRegisterCalleeSaved(n.number))
            continue;;
        bool valid;
        switch (valueType) {
            case Type::address:
            case Type::unsignedInteger:
                valid = n.canOperateOnUnsignedIntegers;
                break;
            case Type::signedInteger:
                valid = n.canOperateOnSignedIntegers;
                break;
            case Type::floatingPoint:
                valid = n.canOperateOnFloatingPointValues;
                break;
            default: valid = false;
        }
        switch (size) {
            case 1: valid *= n.operandSize8; break;
            case 2: valid *= n.operandSize16; break;
            case 4: valid *= n.operandSize32; break;
            case 8: valid *= n.operandSize64; break;
            case 16: valid *= n.operandSize128; break;
            case 32: valid *= n.operandSize256; break;
            case 64: valid *= n.operandSize512; break;
            default: valid = false;
        }
        if (valid) return {Location::reg, valueType, false, size, n.number};
    }
    if (not useCalleeSaved)
        return getFreeRegister(valueType, size, true);
    Error("Internal: could not find a valid register in first pass!");
}


Place::Place(Register* reg, const Location::Type where) : reg(reg), where(where) {}
Place::Place(StackEntry* sta, const Location::Type where) : sta(sta), where(where){}

AsmOperand::AsmOperand(Location::Type op, Type::TypeEnum type, bool isArgumentMove, uint8_t size, OperandValue value)
    : op(op), type(type), isArgumentMove(isArgumentMove), size(size), value(value) {}

AsmOperand::AsmOperand(Location::Type op, Type::TypeEnum type, bool isArgumentMove, LabelType label, OperandValue value)
    : op(op), type(type), isArgumentMove(isArgumentMove), labelType(label), value(value) {}

AsmOperand::AsmOperand(const int32_t stackOffset) {
    op = Location::sta;
    value.offset = stackOffset;
}

AsmOperand::AsmOperand(BytecodeOperand op, Context& context) {
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
    else if (op.location == Location::String) {
        this->op = op.location;
        size = 8;
        type = Type::address;
        value = op.value;
    }
    else if (op.location == Location::None) {
        return;
    }
    else Error("Unsupported operand type!");
}

bool AsmOperand::operator==(const AsmOperand& target) const {
    return target.op == op and target.type == type and target.useAddress == useAddress and target.value.ui == value.ui and target.size == size;
}

bool AsmOperand::is(Location::Type location) const {
    return op == location;
}

AsmOperand::AsmOperand(BytecodeOperand op, Context& context, Location::Type location, OperandValue value) {
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
    else Error("Unsupported operand type!");
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

VariableObject& AsmOperand::object(Context& context) const {
    AsmOperand location = *this;
    if (op != Location::Variable) {
        if (op == Location::reg or op == Location::sta or op == Location::mem) {
            location = context.getContentRef(location);
            if (location.op != Location::Variable) Error("Internal: non variable object get!");
        }
        else Error("Internal: non variable object get!");
    }
    switch (value.variable.type) {
        case VariableLocation::None:
            Error("Internal: invalid variable type in object get!");
        case VariableLocation::Global:
            return globalVariableObjects[value.variable.number];
        case VariableLocation::Local:
            return context.localVariables[value.variable.level][value.variable.number];
        case VariableLocation::Temporary:
            return context.temporaries[value.variable.number];
        default:
            Error("Internal: could not find a variable object!");
        return globalVariableObjects[0];
    }
}

void AsmOperand::print(std::ostream& out, Context& context) {
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
            out << "Literal: ";
            switch (type) {
                case Type::address:
                    switch (Options::addressSize) {
                        case 1:
                            out << "fixed address: " << value.u8 << " ";
                            break;
                        case 2:
                            out << "fixed address: " << value.u16 << " ";
                            break;
                        case 4:
                            out << "fixed address: " << value.u32 << " ";
                            break;
                        case 8:
                            out << "fixed address: " << value.u64 << " ";
                            break;
                        default:
                            break;
                    }
                case Type::floatingPoint:
                    switch (size) {
                        case 2:
                            Error("16 bit floats not supported in printing!");
                            break;
                        case 4:
                            out << "floating point literal: " << value.f32 << " ";
                            break;
                        case 8:
                            out << "floating point literal: " << value.f64 << " ";
                            break;
                        default:
                            break;
                    }
                    break;
                case Type::signedInteger:
                    switch (size) {
                        case 1:
                            out << "signed integer literal: " << value.i8 << " ";
                            break;
                        case 2:
                            out << "signed integer literal: " << value.i16 << " ";
                            break;
                        case 4:
                            out << "signed integer literal: " << value.i32 << " ";
                            break;
                        case 8:
                            out << "signed integer literal: " << value.i64 << " ";
                            break;
                        default:
                            break;
                    }
                break;
                case Type::unsignedInteger:
                    switch (size) {
                        case 1:
                            out << "unsigned integer literal: " << static_cast <uint64_t>(value.u8) << " ";
                        case 2:
                            out << "unsigned integer literal: " << value.u16 << " ";
                        case 4:
                            out << "unsigned integer literal: " << value.u32 << " ";
                        case 8:
                            out << "unsigned integer literal: " << value.u64 << " ";
                        default:
                            break;
                    }
                    break;
            }
            break;
        case Location::String:
            out << "String: " << "<placeholder>" << " ";
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
        case Location::Stack:
            out << "Stack: " << std::to_string(value.offset) << " ";
        break;
        case Location::Register:
            out << "Register: " << std::to_string(value.reg) << " ";
        break;
        default:
            Error("Internal: unhandled operand print case!");
    }
}

AsmOperand AsmOperand::moveAwayOrGetNewLocation(Context& context, std::vector<AsmInstruction>& instructions, uint32_t index, std::vector <AsmOperand>* forbiddenLocations, bool stackOnly) {
    if (op != Location::reg and op != Location::sta) Error("Internal: cannot move away a non-location!");
    // rewritten code since the old one is buggy

    // first of all let's see what we have here
    auto content = context.getContent(*this);
    // if it's not a variable then we can return
    if (content.op != Location::Var) return *this;
    auto& obj = content.object(context);
    // if the variable's last use is in the past then we can also return
    if (obj.lastUse < index) return *this;

    // now let's get the places where this thing actually is
    auto locations = content.getAllLocations(context);
    uint16_t otherPlaces = 0;
    for (auto& location : locations) {
        if (location == *this or not location.is(Location::Stack))
            continue;

        bool isValid = true;
        if (forbiddenLocations != nullptr)
            for (auto& m : *forbiddenLocations)
                if (m == location) {isValid = false; break;}
        if (not isValid)
            continue;

        ++otherPlaces;
    }

    if (otherPlaces == 0) {
        auto move = MoveInfo(*this, {});

        if (is(Location::reg) and not stackOnly)
            move.target = context.getFreeRegister(content.type, content.size);
        else
            move.target = context.pushStack(content);

        move.source.size = content.size;
        move.source.type = content.type;

        AddConversionsToMove(move, context, instructions, content, forbiddenLocations);
        if (is(Location::Sta))
            return move.target;
    }

    return *this;

    /*
    // TODO: there was something to improve with counting here
    auto& content = op == Location::reg ? context.registers[value.reg].content : context.getContentRefAtOffset(value.offset);
    if (content.op != Location::Var) return *this;
    auto& obj = content.object(context);
    if (obj.lastUse < index) return *this;
    auto locations = content.getAllLocations(context);
    uint16_t validPlaces = 0;
    for (auto& n : locations) {
        if (n == *this) continue;
        bool isValid = true;
        if (forbiddenLocations != nullptr)
            for (auto& m : *forbiddenLocations) if (m == n) {isValid = false; break;}
        if (stackOnly) validPlaces += n.op == Location::sta;
        else validPlaces += isValid and n != *this;
    }

    if (validPlaces <= 1 - stackOnly) {
        // there is no other place that will survive where this value is stored
        auto move = MoveInfo(*this, context.pushStack(content));
        //move.target = context.pushStack(content);
        AddConversionsToMove(move, context, instructions, content, forbiddenLocations);
    }
    return *this;
    */
}

std::vector<AsmOperand> AsmOperand::getAllLocations(Context& context) {
    std::vector<AsmOperand> result;
    for (auto& n : context.registers) {
        if (n.content == *this) result.push_back(n.content.copyTo(Location::reg, n.number));
    }
    for (auto& n : context.stack) {
        if (n.content == *this) result.push_back(n.content.copyTo(Location::sta, n.offset));
    }
    return std::move(result);
}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, AsmOperand location, AsmOperand value) : isFixedLocation(true), isInput(isInput), fixedLocation(location), value(value) {}

AsmInstructionResultInput::AsmInstructionResultInput(bool isInput, uint8_t operandNumber, AsmOperand value) : isInput(isInput), operandNumber(operandNumber), value(value) {}

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

void Context::assignVariable(AsmOperand variable, AsmOperand source, std::vector<AsmInstruction>& instructions) {
    auto locations = variable.getAllLocations(*this);
    //if (locations.empty()) pushStack(variable, context);
    if (source.op == Location::imm or source.op == Location::String) {
        auto location = getLocationStackBias(variable);
        MoveInfo move = {source, location};
        for (auto& n : registers) if (n.content == variable) n.content = {};
        for (auto& n : stack) if (n.content == variable) n.content = {};
        AddConversionsToMove(move, *this, instructions, variable, nullptr);
    }
    else if (source.op == Location::reg or source.op == Location::sta) {
        AsmOperand location = {};
        if (getContent(source).op == Location::Variable) {
            auto content = getContent(source);
            auto& obj = content.object(*this);
            if (obj.lastUse <= index) location = source;
            else location = source.moveAwayOrGetNewLocation(*this, instructions, index, nullptr);
        }
        else location = source;
        MoveInfo move = {source, location};
        for (auto& n : registers) if (n.content == variable) n.content = {};
        for (auto& n : stack) if (n.content == variable) n.content = {};
        AddConversionsToMove(move, *this, instructions, variable, nullptr);
    }
    else Error("Internal: unimplemented assignment source!");
}

void Context::cleanUnusedVariables() {
    for (auto& n : registers) {
        if (n.content.op == Location::Variable) {
            auto& obj = n.content.object(*this);
            if (obj.lastUse <= index) n.content = {};
        }
    }
    for (auto& n : stack) {
        if (n.content.op == Location::Variable) {
            auto& obj = n.content.object(*this);
            if (obj.lastUse <= index) n.content = {};
        }
    }
}

std::vector<MemorySnapshotEntry> Context::createSnapshot() {
    std::vector<MemorySnapshotEntry> output;
    for (auto& n : registers) {
        if (n.content.op == Location::Variable)
            if (n.content.object(*this).lastUse > index)
                output.emplace_back(n.content.copyTo(Location::reg, n.number), n.content);
    }
    for (auto& n : stack) {
        if (n.content.op == Location::Variable)
            if (n.content.object(*this).lastUse > index)
                output.emplace_back(n.content.copyTo(Location::sta, n.offset), n.content);
    }
    return std::move(output);
}

void Context::restoreSnapshot(std::vector<MemorySnapshotEntry>& snapshot, std::vector <AsmInstruction>& instructions) {
    // registers and stack are always in order
    // first let's move things
    for (auto& n : snapshot) {
        if (n.where.op == Location::reg) {
            if (registers[n.where.value.reg].content != n.what) {
                MoveInfo move = {getLocation(n.what), n.where.moveAwayOrGetNewLocation(*this, instructions, index, nullptr)};
                // TODO: add registers to set as forbidden
                AddConversionsToMove(move, *this, instructions, n.what, nullptr);
            }
        }
        else {
            if (getContent(n.where) != n.what) {
                MoveInfo move = {getLocation(n.what), n.where.moveAwayOrGetNewLocation(*this, instructions, index, nullptr)};
                // TODO: add registers to set as forbidden
                AddConversionsToMove(move, *this, instructions, n.what, nullptr);
            }
        }
    }

    // we need to go through the registers and stack. If something wasn't there during the snapshot then it needs to go
    // also there needs to be an exception for the stuff that needs to stay alive
    // sadly it is rather slow compared to the last implementation but it allows this thing to actually work
    for (auto& n : registers) {
        if (n.content.is(Location::Variable)) {
            bool isInSnapshot = false;
            bool notHere = false;
            for (auto& snap : snapshot) {
                if (snap.where.is(Location::reg) and snap.where.value.reg == n.number and snap.what == n.content) {
                    isInSnapshot = true;
                    break;
                }
                if (snap.what == n.content) {
                    notHere = true;
                    break;
                }
            }

            if (not notHere) {
                // now we have a variable that needs to go if it does not live later (mostly for loops producing temps)
                auto& obj = getVariableObject(n.content);
                if (obj.lastUse > index)
                    continue;
            }

        }
        n.content = {};
    }

    // the same but for stack
    for (int64_t k = stack.size() - 1; k >= 0; k--) {
        auto& n = stack[k];
        if (n.content.is(Location::Variable)) {
            bool isInSnapshot = false;
            bool notHere = false;
            for (auto& snap : snapshot) {
                if (snap.where.is(Location::sta) and snap.where.value.offset == n.offset and snap.what == n.content) {
                    isInSnapshot = true;
                    break;
                }
                if (snap.what == n.content) {
                    notHere = true;
                    break;
                }
            }

            if (isInSnapshot)
                continue;

            if (not notHere) {
                // now we have a variable that needs to go if it does not live later (mostly for loops producing temps)
                auto& obj = getVariableObject(n.content);
                if (obj.lastUse > index)
                    continue;
            }

        }
        stack.erase(stack.begin() + k);
    }
}
