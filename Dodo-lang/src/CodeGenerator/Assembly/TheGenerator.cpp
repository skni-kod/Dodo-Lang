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
    if (types.back() == Operand::var) {
        auto loc = generatorMemory.findThing(req.op1);
        if (loc.type == Operand::none) {
            auto& life = variableLifetimes[req.op1];
            if (life.assignStatus == VariableStatistics::AssignStatus::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == VariableStatistics::AssignStatus::reg) {
                loc.type = Operand::reg;
            }
            else {
                CodeGeneratorError("Unimplemented: Invalid location!");
            }
        }
        types.back() = loc.type;
    }
    if (req.op2.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op2));
    if (types.back() == Operand::var) {
        auto loc = generatorMemory.findThing(req.op2);
        if (loc.type == Operand::none) {
            auto& life = variableLifetimes[req.op2];
            if (life.assignStatus == VariableStatistics::AssignStatus::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == VariableStatistics::AssignStatus::reg) {
                loc.type = Operand::reg;
            }
            else {
                CodeGeneratorError("Unimplemented: Invalid location!");
            }
        }
        types.back() = loc.type;
    }
    if (req.op3.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op3));
    if (types.back() == Operand::var) {
        auto loc = generatorMemory.findThing(req.op3);
        if (loc.type == Operand::none) {
            auto& life = variableLifetimes[req.op3];
            if (life.assignStatus == VariableStatistics::AssignStatus::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == VariableStatistics::AssignStatus::reg) {
                loc.type = Operand::reg;
            }
            else {
                CodeGeneratorError("Unimplemented: Invalid location!");
            }
        }
        types.back() = loc.type;
    }
    if (req.op4.empty()) {
        return types;
    }
    types.push_back(GetOperandType(req.op4));
    if (types.back() == Operand::var) {
        auto loc = generatorMemory.findThing(req.op4);
        if (loc.type == Operand::none) {
            auto& life = variableLifetimes[req.op4];
            if (life.assignStatus == VariableStatistics::AssignStatus::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == VariableStatistics::AssignStatus::reg) {
                loc.type = Operand::reg;
            }
            else {
                CodeGeneratorError("Unimplemented: Invalid location!");
            }
        }
        types.back() = loc.type;
    }

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
                    match+=3;
                    break;

                case Operand::imm:
                    // if the operand is a register or stack location then it can accept a register or stack if needed
                    if (currentTypes[m] != Operand::imm) {
                        match+=1;
                    }
                    else {
                        match+=5;
                    }
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
        return req.combinations[index];
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

internal::StackEntry* FindStackVariableByName(const std::string& name) {
    for (auto& n: generatorMemory.stack) {
        if (n.content.value == name) {
            return &n;
        }
    }
    return nullptr;
}

// pass a string in format "@<offset>"
internal::StackEntry* FindStackVariableByOffset(const std::string& offset) {
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
        type = ParserType::Type::unsignedInteger;
    }
    else if (name.front() == 'i') {
        type = ParserType::Type::signedInteger;
    }
    else if (name.front() == 'f') {
        type = ParserType::Type::floatingPoint;
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

void SetContent(DataLocation location, const std::string& content) {
    switch (location.type) {
        case Operand::reg:
            generatorMemory.registers[location.value].content.value = content;
            return;
        case Operand::sta:
            FindStackVariableByOffset(location.offset)->content.value = content;
            return;
    }
    CodeGeneratorError("Unimplemented: Non stack/register content set!");
}

std::vector<std::pair<uint64_t, std::string>> storesToCheck;

void MoveValue(std::string source, std::string target, std::string contentToSet, uint16_t operationSize, uint64_t index) {
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
            if (reg.content.value == contentToSet) {
                // if the value is already there
                return;
            }
            // TODO: add better conditions for move, they are producing useless stores
            if (GetOperandType(reg.content.value) != Operand::imm and variableLifetimes[reg.content.value].lastUse >= index) {
                // in this case the value needs to be moved back to it's correct place if it's not already there
                auto& life = variableLifetimes[reg.content.value];
                if (life.assignStatus == VariableStatistics::reg and life.assigned != targetNumber) {
                    // the variable is in the wrong register, move it to it's correct one
                    // TODO: think about the case where something is in the assigned one
                    MoveValue(reg.content.value, "%" + std::to_string(life.assigned), reg.content.value, reg.content.value[1] - '0', index);
                    if (Optimizations::checkPotentialUselessStores and variableLifetimes[reg.content.value].lastUse == index) {
                        if (Options::targetArchitecture == "X86_64") {
                            for (int64_t n = finalInstructions.size(); n >= 0; n--) {
                                if (finalInstructions[n].op1.type == Operand::reg and finalInstructions[n].op1.number == life.assigned  and
                                    finalInstructions[n].op2.type == Operand::reg and finalInstructions[n].op2.number == targetNumber) {
                                    storesToCheck.emplace_back(n, generatorMemory.registers[life.assigned].content.value);
                                }
                            }
                        }
                        else {
                            CodeGeneratorError("Unimplemented: non x86-64 suspicious value find!");
                        }
                    }
                }
                else {
                    stackLocation = FindStackVariableByName(reg.content.value);
                    if (not stackLocation) {
                        stackLocation = AddStackVariable(reg.content.value);
                        if (Options::targetArchitecture == "X86_64") {
                            Instruction ins;
                            ins.type = x86_64::mov;
                            ins.op1 = {Operand::sta, stackLocation->offset};
                            ins.op2 = {Operand::reg, targetNumber};
                            ins.sizeAfter = ins.sizeAfter = stackLocation->size;
                            ins.postfix1 = AddInstructionPostfix(stackLocation->size);
                            if (Optimizations::checkPotentialUselessStores and variableLifetimes[reg.content.value].lastUse == index) {
                                storesToCheck.emplace_back(finalInstructions.size(), FindStackVariableByOffset(stackLocation->offset)->content.value);
                            }
                            finalInstructions.push_back(ins);
                        }
                    }
                    else if (stackLocation->content.value != reg.content.value) {
                        if (Options::targetArchitecture == "X86_64") {
                            Instruction ins;
                            ins.type = x86_64::mov;
                            ins.op1 = {Operand::sta, stackLocation->offset};
                            ins.op2 = {Operand::reg, targetNumber};
                            ins.sizeAfter = ins.sizeAfter = stackLocation->size;
                            ins.postfix1 = AddInstructionPostfix(stackLocation->size);
                            if (Optimizations::checkPotentialUselessStores and variableLifetimes[reg.content.value].lastUse == index) {
                                storesToCheck.emplace_back(finalInstructions.size(), FindStackVariableByOffset(stackLocation->offset)->content.value);
                            }
                            finalInstructions.push_back(ins);
                        }
                        stackLocation->content.value = contentToSet;
                    }
                }
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
        else if (stackLocation->content.value == contentToSet) {
            // if it's already there then end this charade
            return;
        }
        stackLocation->content.value = contentToSet;
    }

    // now the target is ready for input
    auto sourceContent = generatorMemory.findThing(source);

    // move the source to target
    if (Options::targetArchitecture == "X86_64") {
        // source was found
        if (sourceContent.type != Operand::none) {
            Instruction ins;
            ins.type = x86_64::mov;
            ins.op2 = sourceContent;
            // immutables are simple, just move them
            if (targetType == Operand::reg) {
                ins.sizeAfter = ins.sizeBefore = operationSize;
                ins.op1 = {Operand::reg, targetNumber};
                SetContent(ins.op1, contentToSet);
            }
            else if (targetType == Operand::sta) {
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
                    MoveValue("@" + std::to_string(stackLocation->offset), "%" + std::to_string(reg), stackLocation->content.value, operationSize, index);
                    stackLocation->content.value = contentToSet;
                    ins.op1 = {Operand::reg, reg};
                }
                else {
                    ins.op1 = {Operand::sta, stackLocation->offset};
                }
            }
            else {
                CodeGeneratorError("Assigning to invalid operand?");
            }
            // there is no sense in normal move to the same location, return
            if (ins.op1 == ins.op2) {
                return;
            }
            finalInstructions.push_back(ins);
        }

        // in this case variable does not exist and needs to be converted from base variable
        else {
            // first find the main variable
            std::string searched = source.substr(3, source.size() - 3);
            DataLocation baseLocation;
            VariableType baseType;
            bool baseDies = false;
            for (auto& n : variableLifetimes.map) {
                if (n.first.ends_with(searched) and n.second.isMainValue) {
                    baseLocation = generatorMemory.findThing(n.first);
                    // check if it's the last use of the base, if it is then it allows for more optimized operations
                    if (variableLifetimes[n.first].lastUse <= index) {
                        baseDies = true;
                    }
                    if (baseLocation.type == Operand::none) {
                        CodeGeneratorError("Bug: searched variable was not created!");
                    }
                    baseType = VariableType(n.first);
                    break;
                }
            }
            if (baseLocation.type == Operand::none) {
                CodeGeneratorError("Base variable ending with: " + searched + " does not exist!");
            }

            // get target size from size of source variable
            uint8_t targetSize = std::stoull(source.substr(1, 1));

            // now that base variable is found convert it
            if (baseType.type != ParserType::floatingPoint and targetType != ParserType::floatingPoint) {
                // both are integers, it's simple in that case
                Instruction ins;
                ins.sizeAfter = targetSize;
                // size before is the same if base is bigger or the same and smaller if it's smaller

                // first set the operation type and source operands

                // if the target is smaller or equal to base just move the value
                if (targetSize <= baseType.size) {
                    if (baseDies) {
                        SetContent(baseLocation, contentToSet);
                    }
                    ins.sizeBefore = targetSize;
                    ins.type = x86_64::mov;
                    ins.op2 = baseLocation;
                }
                // if it's bigger move the value with movzx or movsx depending on the source signedness to better preserve values
                else {
                    ins.sizeBefore = baseType.size;
                    if (baseType.type == ParserType::signedInteger) {
                        ins.type = x86_64::movsx;
                    }
                    else {
                        ins.type = x86_64::movzx;
                    }

                    ins.op2 = baseLocation;
                }

                // set destination operands
                // if both are on the stack do a conversion into the designated register for the converted type or %rax and then move it to stack
                if (targetType == Operand::sta and baseLocation.type == Operand::sta) {
                    // TODO: add move back to location instruction and get the value out of the destination register and move there
                    CodeGeneratorError("Stack to stack type conversions not yet supported!");
                }
                else {
                    // this case is simple, just move the value there
                    if (targetType == Operand::reg) {
                        ins.op1 = {Operand::reg, targetNumber};
                    }
                    else if (targetType == Operand::sta) {
                        auto* temp = AddStackVariable(target);
                        temp->content.value = contentToSet;
                        ins.op1 = {Operand::sta, temp->offset};
                    }
                    finalInstructions.push_back(ins);
                    SetContent(ins.op1, contentToSet);
                }
            }
            else {
                CodeGeneratorError("Unimplemented: Floating point type/size conversions!");
            }
        }
    }
}

void FillDesignatedPlaces(uint64_t index) {
    // TODO: add predesignated stack offsets or something, as this might cause problems with variable keeping
    // loop might be useless but it makes it more reliable
    bool secondRun = false;
    while (true) {
        uint64_t changes = 0;
        for (auto& n : variableLifetimes.map) {
            if (n.second.firstUse <= index and n.second.lastUse >= index) {
                // this variable exists, it needs to be ensured that it's in place
                auto data = generatorMemory.findThing(n.first);
                if (data.type == Operand::none) {
                    CodeGeneratorError("Bug: Move of non existent variable to designated register, should be unreachable!");
                    return;
                }

                // if it's fine check if the data is in the right place
                if (n.second.assignStatus == VariableStatistics::reg) {
                    if (generatorMemory.registers[n.second.assigned].content.value != n.first) {
                        MoveValue(n.first, "%" + std::to_string(n.second.assigned), n.first, n.first[1] - '0', index);
                        changes++;
                        SetContent(data, "!");
                    }
                }
                else if (n.second.assignStatus == VariableStatistics::sta) {
                    if (FindStackVariableByName(n.first) == nullptr) {
                        auto* sta = AddStackVariable(n.first);
                        sta->content.value = "!";
                        MoveValue(n.first, "@" + std::to_string(sta->offset), n.first, n.first[1] - '0', index);
                        changes++;
                        SetContent(data, "!");
                    }
                }
                else {
                    CodeGeneratorError("Unimplemented: non stack/register designed place move!");
                }
            }
        }
        if (changes == 0) {
            break;
        }
        if (secondRun) {
            std::cout << "You were right!\n";
        }
        secondRun = true;
    }
}

void GenerateInstruction(InstructionRequirements req, uint64_t index) {

    // get the operand types passed by caller
    auto types = GetOperandTypes(req);

    // choose the best valid operand combination if there is one
    OpCombination& combination = ChooseOpCombination(req, types);

    Instruction ins;
    ins.type = req.instructionNumber;
    ins.sizeBefore = ins.sizeAfter = req.instructionSize;

    // now it's time to check which operands need to be moved
    std::vector<std::pair<std::string&, DataLocation>> operands;
    std::vector<std::pair<uint8_t, std::vector<uint16_t>*>> vec;

    switch (types.size()) {
        case 4:
            operands = {{{req.op1, generatorMemory.findThing(req.op1)}, {req.op2, generatorMemory.findThing(req.op2)}, {req.op3, generatorMemory.findThing(req.op3)}, {req.op4, generatorMemory.findThing(req.op4)}}};
            vec = {{uint8_t(combination.type1), &combination.allowed1}, {uint8_t(combination.type2), &combination.allowed2}, {uint8_t(combination.type3), &combination.allowed3}, {uint8_t(combination.type4), &combination.allowed4}};
            break;
        case 3:
            operands = {{{req.op1, generatorMemory.findThing(req.op1)}, {req.op2, generatorMemory.findThing(req.op2)}, {req.op3, generatorMemory.findThing(req.op3)}}};
            vec = {{uint8_t(combination.type1), &combination.allowed1}, {uint8_t(combination.type2), &combination.allowed2}, {uint8_t(combination.type3), &combination.allowed3}};
            break;
        case 2:
            operands = {{{req.op1, generatorMemory.findThing(req.op1)}, {req.op2, generatorMemory.findThing(req.op2)}}};
            vec = {{uint8_t(combination.type1), &combination.allowed1}, {uint8_t(combination.type2), &combination.allowed2}};
            break;
        case 1:
            operands = {{{req.op1, generatorMemory.findThing(req.op1)}}};
            vec = {{uint8_t(combination.type1), &combination.allowed1}};
            break;
    }

    struct MoveStruct {
        enum {
            copy, move
        };
        uint8_t number;
        uint8_t type;
        DataLocation where;
        MoveStruct(uint8_t number, uint8_t type, DataLocation where) : number(number), type(type), where(where) {}
    };
    std::vector <MoveStruct> operandsToMove;
    std::vector <DataLocation> valueRegisters;
    std::vector <bool> occupied;
    if (Options::targetArchitecture == "X86_64") {
        occupied.resize(64, false);
    }

    // And another rewrite of this, this MUST work well

    // we have the locations of operands
    // we have the allowed locations
    // let's place them where they are needed

    // find which registers have constants in them
    for (auto& m : combination.registerValues) {
        occupied[m.first] = true;
    }

    // first check if the operands are in the right places
    for (uint8_t n = 0; n < operands.size(); n++) {
        if (operands[n].second.type != vec[n].first) {
            // location needs to change
            if (operands[n].second.type == Operand::reg) {
                if (vec[n].first == Operand::sta) {
                    // move to stack
                    auto* stackLoc = FindStackVariableByName(operands[n].first);
                    if (stackLoc == nullptr) {
                        stackLoc = AddStackVariable(operands[n].first);
                        stackLoc->content.value = "!";
                    }
                    operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::sta, stackLoc->offset));
                }
                else {
                    CodeGeneratorError("Unimplemented!");
                }
            }
            else if (operands[n].second.type == Operand::sta) {
                // needs to be moved to register
                // no certain location yet
                operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::reg, uint64_t()));
            }
            else if (operands[n].second.type == Operand::imm) {
                if (vec[n].first == Operand::reg) {
                    // needs to be moved to register
                    // no certain location yet
                    operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::reg, uint64_t()));
                }
                else if (vec[n].first == Operand::sta) {
                    // move to stack
                    auto* stackLoc = AddStackVariable(operands[n].first);
                    stackLoc->content.value = "!";
                    operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::sta, stackLoc->offset));
                }
            }
            else if (operands[n].second.type == Operand::none) {
                // in that case the value does not yet exist, needs to be converted
                auto& main = FindMain(operands[n].first);
                auto mainLocation = generatorMemory.findThing(*lastMainName);
                // if main was used for the last time here, it can be used instead of a new register
                // but for now only if it's smaller
                bool isValid = false;
                if (main.lastUse == index and lastMainName->at(1) >= operands[n].first[1]) {
                    if (mainLocation.type == Operand::reg) {
                        for (auto& k : *vec[n].second) {
                            if (k == mainLocation.number) {
                                isValid = true;
                                break;
                            }
                        }
                    }
                    else {
                        // lok out for this
                        isValid = true;
                    }
                }
                if (isValid) {
                    operands[n].second = mainLocation;
                }
                else {
                    if (vec[n].first == Operand::reg) {
                        // needs to be moved to register
                        // no certain location yet
                        operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::reg, uint64_t()));
                    }
                    else if (vec[n].first == Operand::sta) {
                        // move to stack
                        auto* stackLoc = AddStackVariable(operands[n].first);
                        stackLoc->content.value = "!";
                        operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::sta, stackLoc->offset));
                    }
                }
            }
            else {
                CodeGeneratorError("Unimplemented!");
            }
        }
        else if (operands[n].second.type == Operand::reg) {
            bool isValid = false;
            for (auto& m : *vec[n].second) {
                if (m == operands[n].second.value) {
                    isValid = true;
                    break;
                }
            }
            if (isValid) {
                occupied[operands[n].second.value] = true;
            }
            else {
                operandsToMove.emplace_back(n, MoveStruct::move, DataLocation(Operand::reg, uint64_t()));
            }
        }
    }

    // find the registers for things that require them
    for (auto& n : operandsToMove) {
        if (n.where.type == Operand::reg) {
            bool found = false;
            for (auto& m : *vec[n.number].second) {
                // find the first valid register
                if (not occupied[m]) {
                    found = true;
                    occupied[m] = true;
                    n.where.value = m;
                    break;
                }
            }
            if (not found) {
                CodeGeneratorError("To dev: I'm really sorry, you need to implement better operand move :(");
            }
        }
    }

    // now all values before the instruction have been considered
    // that means that the values that change after it need to be taken care of now
    // and this will be a mess so the functions will be dumbed down for now and expanded in the future if needed
    // I don't want to spend weeks making this part work

    // find which values need to be copied too
    for (uint8_t n = 0; n < operands.size(); n++) {
        if (operands[n].second.type == Operand::imm) {
            continue;
        }
        auto& life = variableLifetimes[operands[n].first];

        // now see if the variable will still exist after the instruction
        if (life.lastUse > index) {
            // if it will check if it's value will be removed
            for (auto& m : combination.results) {
                if (m.first.type == Operand::replace and m.first.value == n) {
                    // replace var on stack or in register
                    if (operands[n].second.type == Operand::reg) {
                        // it's a register
                        // TODO: add a check if the copy could be moved away from assigned location and only if not move the original
                        DataLocation& current = operands[n].second;
                        if (life.assignStatus == VariableStatistics::reg) {
                            if (current.number != life.assigned) {
                                operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::reg, uint64_t(life.assigned)));
                                continue;
                            }
                            else {
                                // find a free register, starting from back to reduce the risk of multiple moves
                                bool found = false;
                                for (int64_t k = generatorMemory.registers.size() -1; k >= 0; k--) {
                                    auto& reg = generatorMemory.registers[k];
                                    if (reg.content.value == "!" and not occupied[k]) {
                                        operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::reg, uint64_t(k)));
                                        found = true;
                                        occupied[k] = true;
                                        break;
                                    }
                                }
                                if (found) {
                                    continue;
                                }
                                for (int64_t k = generatorMemory.registers.size() -1; k >= 0; k++) {
                                    auto& reg = generatorMemory.registers[k];
                                    if (reg.content.value.starts_with("$") and not occupied[k]) {
                                        operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::reg, uint64_t(k)));
                                        found = true;
                                        occupied[k] = true;
                                        break;
                                    }
                                }
                                if (found) {
                                    continue;
                                }
                            }
                        }

                        // just move a copy to stack for now, I will optimize this once it works
                        auto* stackLoc = AddStackVariable(operands[n].first);
                        stackLoc->content.value = "!";
                        operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::sta, stackLoc->offset));
                    }
                    else if (operands[n].second.type == Operand::sta) {
                        // it's on stack
                        // just copy it to another place on stack for now
                        auto* stackLoc = AddStackVariable(operands[n].first);
                        stackLoc->content.value = "!";
                        operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::sta, stackLoc->offset));
                    }
                    else {
                        CodeGeneratorError("Unimplemented!");
                    }

                }
                else if (m.first.type == Operand::reg and operands[n].second.type == Operand::reg and m.first.value == operands[n].second.value) {
                    // register with this value will be replaced

                    // just move a copy to stack for now, I will optimize this once it works
                    auto* stackLoc = AddStackVariable(operands[n].first);
                    stackLoc->content.value = "!";
                    operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::sta, stackLoc->offset));

                }
            }
        }
    }

    // finally move the values into place here
    for (auto& n : operandsToMove) {
        std::string target;
        switch (n.where.type) {
            case Operand::sta:
                MoveValue(operands[n.number].first, "@" + std::to_string(n.where.offset), operands[n.number].first, req.instructionSize, index);
                break;
            case Operand::reg:
                MoveValue(operands[n.number].first, "%" + std::to_string(n.where.value), operands[n.number].first, req.instructionSize, index);
                break;
            default:
                CodeGeneratorError("Unimplemented: invalid operand move!");
        }
        if (n.type == MoveStruct::move) {
            operands[n.number].second = n.where;
        }
    }

    // and after all this, add the instruction itself

    // move the operands into places
    switch (types.size()) {
        case 4:
            ins.op4 = operands[3].second;
        case 3:
            ins.op3 = operands[2].second;
        case 2:
            ins.op2 = operands[1].second;
        case 1:
            ins.op1 = operands[0].second;
    }


    if (not req.combinations.empty()) {
        // there are no operands, just ensure any needed values are set and call
        for (auto& n : combination.registerValues) {
            MoveValue("$" + std::to_string(n.second), "%" + std::to_string(n.first), "$" + std::to_string(n.second), req.instructionSize, index);
        }
    }

    finalInstructions.push_back(ins);

    // after that all just set the correct content values
    if (not req.combinations.empty()) {
        for (auto& n : combination.results) {
            switch (n.first.type) {
                case Operand::sta:
                    FindStackVariableByOffset(n.first.offset)->content.value = n.second;
                    break;
                case Operand::reg:
                    generatorMemory.registers[n.first.value].content.value = n.second;
                    break;
                case Operand::replace:
                    switch (operands[n.first.value].second.type) {
                        case Operand::sta:
                            FindStackVariableByOffset(operands[n.first.value].second.offset)->content.value = n.second;
                            break;
                        case Operand::reg:
                            generatorMemory.registers[operands[n.first.value].second.value].content.value = n.second;
                            break;
                        default:
                            CodeGeneratorError("Unimplemented: invalid operand type for content assignment!");
                    }
                    break;
                default:
                    CodeGeneratorError("Unimplemented!");

            }
        }
    }

    // checking potential useless moves
    if (Optimizations::checkPotentialUselessStores) {
        if (Options::targetArchitecture == "X86_64") {
            for (auto& n : storesToCheck) {
                // if it starts eating up variables add this, but attempt proper debug
                if (variableLifetimes[n.second].lastUse > index) {
                    continue;
                }
                bool found = false;
                for (uint64_t m = n.first + 1; m < finalInstructions.size(); m++) {
                    if (finalInstructions[m].op1 == finalInstructions[n.first].op1) {
                        found = true;
                        break;
                    }
                    if (finalInstructions[m].op2 == finalInstructions[n.first].op1) {
                        found = true;
                        break;
                    }
                    if (finalInstructions[m].op3 == finalInstructions[n.first].op1) {
                        found = true;
                        break;
                    }
                    if (finalInstructions[m].op4 == finalInstructions[n.first].op1) {
                        found = true;
                        break;
                    }
                }
                if (not found) {
                    for (auto& k : storesToCheck) {
                        if (k.first > n.first) {
                            k.first--;
                        }
                    }
                    SetContent(finalInstructions[n.first].op1, n.second);
                    finalInstructions.erase(finalInstructions.begin() + n.first);
                }
            }
        }
        else {
            CodeGeneratorError("Unimplemented: Non x86_64 useless move optimization!");
        }
    }
    storesToCheck.clear();
}

// in reality this just changes the expression name to the variable, at least it should do that as of the moment I thought it up
void AssignExpressionToVariable(const std::string& exp, const std::string& var) {
    // find all instances of the expression and replace their name with the variable
    // if it's not the same size let it cry
    if (exp[1] != var[1]) {
        CodeGeneratorError("Unimplemented: Expression to variable assignment size conversion!");
    }
    for (auto& n : generatorMemory.registers) {
        if (n.content.value == exp) {
            n.content.value = var;
        }
    }
    for (auto& n : generatorMemory.stack) {
        if (n.content.value == exp) {
            n.content.value = var;
        }
    }
}