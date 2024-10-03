#include "TheGenerator.hpp"
#include "GenerateCode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "LinearAnalysis.hpp"

uint8_t GetOperandType(const std::string& operand) {
    if (operand.front() == '%') {
        return Operand::reg;
    }
    if (operand.front() == '@') {
        return Operand::sta;
    }
    if (operand.front() == '$') {
        return Operand::imm;
    }
    if (operand.front() == 'i' or operand.front() == 'u' or operand.front() == 'f') {
        return Operand::var;
    }
    return Operand::none;
}

std::vector<uint8_t> GetOperandTypes(const InstructionRequirements& req) {
    std::vector<uint8_t> types;

    if (req.op1.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op1));
    if (req.op2.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op2));
    if (req.op3.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op3));
    if (req.op4.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op4));

    return types;

}
OpCombination opCombDummy;

OpCombination& ChooseOpCombination(InstructionRequirements& req, std::vector<uint8_t>& operands) {
    std::vector<OpCombination*> validOnes;

    uint16_t index = 0;
    uint16_t maxMatch = 0;
    for (uint16_t k = 0; k < req.combinations.size(); k++) {
        uint16_t match = 0;
        // check everything about the possibilities to only leave the ones usable in this instance
        auto& n = req.combinations[k];
        // first the amount of operands
        std::array<const uint8_t, 4> currentTypes = {n.type1, n.type2, n.type3, n.type4};
        uint8_t operandAmount = 0;
        for (; operandAmount < 4; operandAmount++) {
            if (currentTypes[operandAmount] == Operand::none) {
                break;
            }
        }

        if (operands.size() != operandAmount) {
            continue;
        }

        bool isValid = true;
        // when size is confirmed types need to be checked
        for (uint8_t m = 0; m < operands.size(); m++) {
            switch (operands[m]) {
                case Operand::reg:
                case Operand::sta:
                    // if the operand is a register or stack location then it can accept a register or stack if needed
                    if (currentTypes[m] != Operand::reg and currentTypes[m] != Operand::sta) {
                        isValid = false;
                    }
                    match++;
                    break;

                case Operand::imm:
                    // if the operand is a register or stack location then it can accept a register or stack if needed
                    if (currentTypes[m] != Operand::imm) {
                        isValid = false;
                    }
                    match++;
                    break;
            }
            if (not isValid) {
                break;
            }
        }

        if (isValid) {
            validOnes.push_back(&n);
            if (match > maxMatch) {
                maxMatch = match;
                index = k;
            }
        }
    }

    if (validOnes.size() == 1) {
        return *validOnes.front();
    }
    else if (validOnes.size() > 1) {
        return *validOnes[index];
    }
    return opCombDummy;
}

void MoveValueToStorage() {

}


void UpdateVariables() {
    for (auto& n: generatorMemory.registers) {
        uint8_t type = GetOperandType(n.content.value);
        if (type == Operand::var and variableLifetimes[n.content.value].lastUse < currentBytecodeIndex) {
            n.content.value = "!";
        }
    }
    for (uint64_t n = 0; n < generatorMemory.stack.size(); n++) {
        uint8_t type = GetOperandType(generatorMemory.stack[n].content.value);
        if (type == Operand::var and
            variableLifetimes[generatorMemory.stack[n].content.value].lastUse < currentBytecodeIndex) {
            generatorMemory.stack.erase(generatorMemory.stack.begin() + n);
        }
    }
}

internal::StackEntry* FindStackVariableByName(std::string name) {
    for (auto& n: generatorMemory.stack) {
        if (n.content.value == name) {
            return &n;
        }
    }
    return nullptr;
}

// pass a string in format "@<offset>"
internal::StackEntry* FindStackVariableByOffset(std::string offset) {
    uint64_t off = std::stoull(offset.substr(1, offset.size() - 1));
    for (auto& n: generatorMemory.stack) {
        if (n.offset == off) {
            return &n;
        }
    }
    return nullptr;
}

internal::StackEntry* FindStackVariableByOffset(std::uint64_t offset) {
    for (auto& n: generatorMemory.stack) {
        if (n.offset == offset) {
            return &n;
        }
    }
    return nullptr;
}

// adds a variable slot on the stack, pass a variable with prefix
internal::StackEntry* AddStackVariable(std::string name) {
    // type will be used in things probably
    uint8_t type;
    if (name.front() == 'u') {
        type == ParserType::Type::unsignedInteger;
    }
    else if (name.front() == 'i') {
        type == ParserType::Type::signedInteger;
    }
    else if (name.front() == 'f') {
        type == ParserType::Type::floatingPoint;
    }
    else {
        CodeGeneratorError("Invalid prefix type!");
    }

    int64_t size = std::stoll(name.substr(1, 1));

    // TODO: add amount later if necessary

    // searching for a valid space between vars
    for (uint64_t n = 1; n < generatorMemory.stack.size(); n++) {
        auto& b = generatorMemory.stack[n - 1];
        auto& s = generatorMemory.stack[n];
        int64_t space = b.offset - (s.offset + s.amount * s.size);
        if (space >= size) {
            // check alignment here
            int64_t aligned = s.offset + s.amount * s.size;
            if (aligned % size != 0) {
                aligned = (aligned / size - 1) * size;
            }
            if (b.offset - aligned >= size) {
                generatorMemory.stack.emplace(generatorMemory.stack.begin() + n,
                                              internal::StackEntry(aligned, 1, size, internal::ContentEntry(name)));
                return &generatorMemory.stack[n];
            }
        }
    }
    if (generatorMemory.stack.empty()) {
        generatorMemory.stack.emplace_back(internal::StackEntry(-size, 1, size, internal::ContentEntry(name)));
    }
    else {
        int64_t offset = generatorMemory.stack.back().offset - size;
        if (offset % size != 0) {
            offset = (offset / size - 1) * size;
        }
        generatorMemory.stack.emplace_back(internal::StackEntry(offset, 1, size, internal::ContentEntry(name)));
    }

    return &generatorMemory.stack.back();
}
void MoveValue(std::pair <std::string, std::string> pair) {
    MoveValue(pair.first, pair.second);
}
void MoveValue(std::string source, std::string target) {
    uint64_t targetNumber;
    internal::StackEntry* stackLocation = nullptr;
    // since this thing needs to move a value to given place first we need to know if the target is a register,
    uint8_t targetType = GetOperandType(target);
    uint8_t sourceType = GetOperandType(source);
    // if it is a register then ensure there is no value there and then perform the move
    if (targetType == Operand::reg) {
        targetNumber = std::stoull(target.substr(1, target.size() - 1));
        auto& reg = generatorMemory.registers[targetNumber];
        if (reg.content.value != "!") {
            // there is value inside
            if (reg.content.value == source) {
                // if the value is already there
                return;
            }
            if (GetOperandType(reg.content.value) == Operand::imm) {
                // immutables should be safe to overwrite since they are always set before the instruction
                return;
            }

            // in this case the value needs to be moved back to it's correct place if it's not already there
            stackLocation = FindStackVariableByName(reg.content.value);
            if (not stackLocation) {
                stackLocation = AddStackVariable(reg.content.value);
                if (options::targetArchitecture == "X86_64") {
                    Instruction ins;
                    ins.type == x86_64::mov;
                    ins.op1 = {Operand::sta, stackLocation->offset};
                    ins.op2 = {Operand::reg, targetNumber};
                    ins.sizeAfter = ins.sizeAfter = stackLocation->size;
                    ins.postfix1 = AddInstructionPostfix(stackLocation->size);
                    finalInstructions.push_back(ins);
                }
            }
            else if (stackLocation->content.value != reg.content.value) {
                if (options::targetArchitecture == "X86_64") {
                    Instruction ins;
                    ins.type == x86_64::mov;
                    ins.op1 = {Operand::sta, stackLocation->offset};
                    ins.op2 = {Operand::reg, targetNumber};
                    ins.sizeAfter = ins.sizeAfter = stackLocation->size;
                    ins.postfix1 = AddInstructionPostfix(stackLocation->size);
                    finalInstructions.push_back(ins);
                }
                stackLocation->content.value = reg.content.value;
            }
        }
    }
    // if it's on stack check if the location exists, if it does then just check if it's not there already
    // source is assumed to be a variable to even be worthy of passing to stack
    else if (targetType == Operand::sta) {
        stackLocation = FindStackVariableByOffset(target);
        if (not stackLocation) {
            stackLocation = AddStackVariable(source);
        }
        else if (stackLocation->content.value == source) {
            // if it's already there then end this charade
            return;
        }
        stackLocation->content.value = source;
    }

    // now the target is ready for input
    auto sourceContent = generatorMemory.findThing(source);

    // move the source to target
    if (options::targetArchitecture == "X86_64") {
        Instruction ins;
        ins.type = x86_64::mov;
        ins.op2 = sourceContent;
        // immutables are simple, just move them
        if (targetType == Operand::reg) {
            ins.op1 = {Operand::reg, targetNumber};
        }
        if (targetType == Operand::sta) {
            ins.sizeAfter = ins.sizeBefore = stackLocation->size;
            // it cannot be nullptr clion, it CAN NOT
            if (sourceContent.type == Operand::sta) {
                // stack to stack transfer is not allowed, need to do an intermediate move to a register
                uint64_t reg = 0;
                for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
                    if (generatorMemory.registers[n].content.value == "!") {
                        reg = n;
                        break;
                    }
                }
                MoveValue("@" + std::to_string(stackLocation->offset), "%" + std::to_string(reg));
                ins.op1 = {Operand::reg, reg};
            }
            else {
                ins.op1 = {Operand::sta, stackLocation->offset};
            }
        }
        else {
            if (sourceType == Operand::imm) {
                // TODO: figure out where to take the value from
                ins.sizeAfter = ins.sizeBefore = 4;
            }
            else if (sourceType == Operand::sta) {
                ins.sizeAfter = ins.sizeBefore = FindStackVariableByOffset(sourceContent.offset)->size;
            }
            else if (sourceType == Operand::var) {
                ins.sizeAfter = ins.sizeBefore = std::stoull(source.substr(1, 1));
            }
            generatorMemory.registers[targetNumber].content.value = source;
        }
        finalInstructions.push_back(ins);
    }
}

void GenerateInstruction(InstructionRequirements req) {

    // get the operand types passed by caller
    auto types = GetOperandTypes(req);
    // choose the best valid operand combination if there is one
    OpCombination& combination = ChooseOpCombination(req, types);

    Instruction ins;
    ins.type = req.instructionNumber;
    ins.sizeBefore = ins.sizeAfter = req.instructionSize;
    ins.postfix1 = AddInstructionPostfix(ins.sizeBefore);
    ins.postfix2 = AddInstructionPostfix(ins.sizeAfter);

    // move the first argument into place if it's there
    if (types.size() >= 1) {
        auto temp = combination.getOperand(1, req.op1);
        MoveValue(temp.first, temp.second);
        ins.op1 = temp.second;
    }

    if (types.size() >= 2) {
        auto temp = combination.getOperand(2, req.op2);
        MoveValue(temp.first, temp.second);
        ins.op2 = temp.second;
    }

    if (types.size() >= 3) {
        auto temp = combination.getOperand(3, req.op3);
        MoveValue(temp.first, temp.second);
        ins.op3 = temp.second;
    }

    if (types.size() == 4) {
        auto temp = combination.getOperand(4, req.op1);
        MoveValue(temp.first, temp.second);
        ins.op4 = temp.second;
    }

    if (req.op1.empty() and not req.combinations.empty()) {
        // there are no operands, just ensure any needed values are set and call
        for (auto& n : combination.registerValues) {
            MoveValue("$" + std::to_string(n.second), "%" + std::to_string(n.first));
        }
    }

    finalInstructions.push_back(ins);
}

std::pair<std::string, std::string> OpCombination::getOperand(uint16_t number, std::string source) {
    uint8_t type = 0;
    std::vector <uint16_t>* vec = nullptr;
    switch (number) {
        case 1:
            type = type1;
            vec = &allowed1;
            break;
        case 2:
            type = type2;
            vec = &allowed2;
            break;
        case 3:
            type = type3;
            vec = &allowed3;
            break;
        case 4:
            type = type4;
            vec = &allowed4;
            break;
        default:
            CodeGeneratorError("BUG: Invalid number in operand get!");
            return {};
    }

    // now type and allowed vec is ready
    DataLocation sourceType;
    sourceType.type = GetOperandType(source);
    // it it's a var convert it into a location
    if (sourceType.type == Operand::var) {
        // get new source
        sourceType = generatorMemory.findThing(source);
        if (sourceType.type == Operand::reg) {
            source = "%" + std::to_string(sourceType.value);
        }
        else {
            source = "@" + std::to_string(sourceType.offset);
        }
    }
    if (sourceType.type == Operand::sta) {
        if (type == sta) {
            return {source, source};
        }
        if (type == reg) {
            // in this case find a free and allowed register, start from back to be sure it's fine
            for (int64_t n = vec->size() - 1; n >= 0; n++) {
                if (generatorMemory.registers[vec->at(n)].content.value == "!") {
                    std::string target = "%" + std::to_string(vec->at(n));
                    return {source, target};
                }
            }
            // if not found, that would be annoying
            // try values, should be safe
            for (int64_t n = vec->size() - 1; n >= 0; n++) {
                if (generatorMemory.registers[vec->at(n)].content.value.starts_with("$")) {
                    std::string target = "%" + std::to_string(vec->at(n));
                    return {source, target};
                }
            }
            CodeGeneratorError("As of yet unhandled operand placing exception!");
        }
    }
    if (sourceType.type == Operand::reg) {
        // if it's on stack then it needs to ge there
        if (type == Operand::sta) {
            CodeGeneratorError("Unimplemented stack operation!");
        }
        else {
            for (auto& n : *vec) {
                if (n == sourceType.number) {
                    return {source, source};
                }
            }
            for (int64_t n = vec->size() - 1; n >= 0; n++) {
                if (generatorMemory.registers[vec->at(n)].content.value == "!") {
                    std::string target = "%" + std::to_string(vec->at(n));
                    return {source, target};
                }
            }
            // if not found, that would be annoying
            // try values, should be safe
            for (int64_t n = vec->size() - 1; n >= 0; n++) {
                if (generatorMemory.registers[vec->at(n)].content.value.starts_with("$")) {
                    std::string target = "%" + std::to_string(vec->at(n));
                    return {source, target};
                }
            }
        }
    }
    else if (sourceType.type == Operand::imm) {
        return {source, source};
    }
    CodeGeneratorError("Invalid operand get operation!");
    return {};
}