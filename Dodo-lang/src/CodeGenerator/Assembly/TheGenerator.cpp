#include "TheGenerator.hpp"

#include <complex>

#include "GenerateCode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "LinearAnalysis.hpp"
#include "Options.hpp"

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
    if (operand.front() == '^') {
        return Operand::adr;
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
            if (life.assignStatus == Operand::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == Operand::reg) {
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
            if (life.assignStatus == Operand::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == Operand::reg) {
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
            if (life.assignStatus == Operand::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == Operand::reg) {
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
            if (life.assignStatus == Operand::sta) {
                loc.type = Operand::sta;
            }
            else if (life.assignStatus == Operand::reg) {
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

void UpdateVariables() {
    for (auto& n: generatorMemory.registers) {
        uint8_t type = GetOperandType(n.content.value);
        if (type == Operand::var and variableLifetimes[n.content.value].lastUse <= currentBytecodeIndex) {
            n.content.value = "!";
        }
    }
    for (uint64_t n = 0; n < generatorMemory.stack.size(); n++) {
        uint8_t type = GetOperandType(generatorMemory.stack[n].content.value);
        if (type == Operand::var and
            variableLifetimes[generatorMemory.stack[n].content.value].lastUse <= currentBytecodeIndex) {
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
    auto& life = variableLifetimes[name];
    if (life.assignStatus == Operand::sta) {
        auto size = GetVariableType(name).size;
        for (uint64_t n = 0; n < generatorMemory.stack.size(); n++) {
            if (generatorMemory.stack[n].offset < life.staOffset) {
                generatorMemory.stack.insert(generatorMemory.stack.begin() + n, {variableLifetimes[name].staOffset, 1, size, internal::ContentEntry("!")});
                return &generatorMemory.stack[n];
            }
        }
        generatorMemory.stack.emplace_back(variableLifetimes[name].staOffset, 1, size, internal::ContentEntry("!"));
        return &generatorMemory.stack.back();
    }
    
    int64_t size = std::stoll(name.substr(1, 1));
    if (GetOperandType(name) != Subtype::value) {
        size = Options::addressSize;
    }

    // TODO: add amount later if necessary

    // searching for a valid space between vars
    for (uint64_t n = 1; n < generatorMemory.stack.size(); n++) {
        auto& b = generatorMemory.stack[n - 1];
        auto& s = generatorMemory.stack[n];
        if (b.offset < minimumSafeStackOffset) {
            continue;
        }
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

VariableType GetVariableType(const std::string& name) {
    VariableType value;
    if (name.front() == '^') {
        value.isAddress = 1;
    }
    switch (name[value.isAddress]) {
        case 'u':
            value.type = Value::unsignedInteger;
            break;
        case 'i':
            value.type = Value::signedInteger;
            break;
        case 'f':
            value.type = Value::floatingPoint;
            break;
        case '$':
            value.type = Value::none;
            return value;
        case '!':
            value.type = Value::none;
        return value;
        case '\"':
            value.type = Value::none;
            return value;
        default:
            CodeGeneratorError("Bug: Invalid variable prefix!");
    }
    uint64_t index = 1 + value.isAddress;
    while (name[index] >= '0' and name[index] <= '9') {
        index++;
    }
    value.size = std::stoull(name.substr(1 + value.isAddress, index - 1));
    while (name[index] == '*') {
        value.subtype++;
        index++;
        value.isAddress = true;
    }
    return value;
}

VariableInfo::VariableInfo(VariableType value, DataLocation location, std::string identifier) : value(value), location(location), identifier(identifier)     {}

VariableInfo VariableInfo::FromLocation(DataLocation location) {
    switch (location.type) {
        case Operand::reg:
            return VariableInfo("%" + std::to_string(location.number));
        case Operand::sta:
            return VariableInfo("@" + std::to_string(location.offset));
        default:
            CodeGeneratorError("Unimplemented: non register/stack variable info from location!");
    }
    return {};
}

VariableInfo::VariableInfo(const std::string& name) {
    identifier = name;
    // check if it's even a variable
    if (identifier == "!") {
        // it's literally nothing
        location.type = Operand::none;
        // and type
        value.type = Type::none;
        return;
    }
    if (identifier.starts_with("\"")) {
        // it's a const string somewhere in memory
        location.type = Operand::sla;
        location.number = std::stoull(identifier.substr(1));
        value.type = Type::none;
        value.isAddress = true;
        value.isString = true;
        return;
    }
    if (identifier.starts_with("$")) {
        // it's a value
        location.type = Operand::imm;
        location.value = std::stoull(identifier.substr(1));

        // and type
        value.type = Type::none;
        return;
    }
    if (identifier.starts_with("%")) {
        // it's a register
        location.type = Operand::reg;
        location.value = std::stoull(identifier.substr(1));

        // and type, let's check what's in the register
        auto& content = generatorMemory.registers[location.value].content.value;
        if (content == "!" or content.starts_with("$")) {
            // it's nothing, let's assume nothing is there
            value.type = Type::none;
            return;
        }

        if (GetVariableType(content).subtype != Subtype::value) {
            // in that case check if a pointer of this exact type exists
            auto location = generatorMemory.findThing(content);
            if (location.type == Operand::none) {
                // in that case change the identifier to the main value
                content = FindMainName(content);
            }
        }

        // in this case there is something there
        if (content.contains("glob.")) {
            //CodeGeneratorError("Unimplemented: global variable in register replacement!");
            // it is, to get the variable name we need to cut off the first part
            std::string realName = content.substr(content.find("glob."));
            if (realName.contains("#")) {
                realName = realName.substr(0, realName.find_last_of('#'));
            }
            auto& var = globalVariablesOLD[realName];

            // search for the location in simulated memory
            location = generatorMemory.findThing(content);

            if (location.type == Operand::none) {
                // location wasn't found, so it's on the heap or wherever the assembler decides to put it
                location.type = Operand::aadr;
                location.globalPtr = &var;
            }
            value = GetVariableType(content);
            identifier = content;
            return;
        }

        // it's a local variable, return its info
        value = GetVariableType(content);
        identifier = content;
        return;

    }
    if (identifier.starts_with("@")) {
        // it's a register
        location.type = Operand::sta;
        location.offset = std::stoll(identifier.substr(1));

        // and type, let's check what's in the register
        auto& content = FindStackVariableByOffset(location.offset)->content.value;
        if (content == "!" or content.starts_with("$")) {
            // it's nothing, let's assume nothing is there
            value.type = Type::none;
            return;
        }

        value = GetVariableType(content);

        if (GetVariableType(content).subtype != Subtype::value) {
            // in that case check if a pointer of this exact type exists
            auto location = generatorMemory.findThing(content);
            if (location.type == Operand::none) {
                // in that case change the identifier to the main value
                content = FindMainName(content);
            }
        }

        // in this case there is something there
        if (content.contains("glob.")) {
            //CodeGeneratorError("Unimplemented: global variable in register replacement!");
            // it is, to get the variable name we need to cut off the first part
            std::string realName = content.substr(content.find("glob."));
            if (realName.contains("#")) {
                realName = realName.substr(0, realName.find_last_of('#'));
            }
            auto& var = globalVariablesOLD[realName];

            // search for the location in simulated memory
            location = generatorMemory.findThing(content);

            if (location.type == Operand::none) {
                // location wasn't found, so it's on the heap or wherever the assembler decides to put it
                location.type = Operand::aadr;
                location.globalPtr = &var;
            }
            identifier = content;
            return;
        }

        // it's a local variable, return its inf
        identifier = content;
        return;

    }

    if (GetVariableType(identifier).subtype != Subtype::value) {
        // in that case check if a pointer of this exact type exists
        auto location = generatorMemory.findThing(identifier);
        if (location.type == Operand::none) {
            // in that case change the identifier to the main value
            identifier = FindMainName(identifier);
        }
    }

    // first off let's see if it's global or not
    if (identifier.contains("glob.")) {
        // it is, to get the variable name we need to cut off the first part
        std::string realName = identifier.substr(identifier.find("glob."));
        if (realName.contains("#")) {
            realName = realName.substr(0, realName.find_last_of('#'));
        }
        auto& var = globalVariablesOLD[realName];

        // search for the location in simulated memory
        location = generatorMemory.findThing(identifier);

        if (location.type == Operand::none) {
            // location wasn't found, so it's on the heap or wherever the assembler decides to put it
            location.type = Operand::aadr;
            location.globalPtr = &var;
        }
        value = GetVariableType(name);
        
        return;
    }

    // now there is the case with the normal variables
    value = GetVariableType(name);
    // now the location of this thing, if the type is none then the variable needs to be converted
    // but that must be handled on a case by case basis
    location = generatorMemory.findThing(identifier);
}

// this function does not care if the variable already exists elsewhere, 
// if it needs to exist in one place use the MoveVariableElsewhere
void CopyVariableElsewhereNoReference(VariableInfo source) {
    CopyVariableElsewhere(source);
}
void CopyVariableElsewhere(VariableInfo& source) {
    // first off see if the variable isn't in it's designated spot
    {
        auto loc = variableLifetimes[source.identifier].toLocation();
        if (loc != source.location) {
            // just move it to its correct locations
            MoveValue(source, VariableInfo::FromLocation(loc), source.identifier, source.value.size);
            if (Optimizations::checkPotentialUselessStores) {
                storesToCheck.emplace_back(finalInstructions.size() - 1, source.identifier);
            }
            return;
        }
    }
    
    // in this case it's already in it's assigned place, look for a free register
    for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
        // TODO: add allowed value types check to this when adding floats
        if (generatorMemory.registers[n].content.value == "!" or generatorMemory.registers[n].content.value.starts_with("$")) {
            MoveValue(source, VariableInfo::FromLocation({Operand::reg, n}), source.identifier, source.value.size);
            if (Optimizations::checkPotentialUselessStores) {
                storesToCheck.emplace_back(finalInstructions.size() - 1, source.identifier);
            }
            return;
        }
    }
    
    // in other cases move it to the stack at a safe offset
    MoveValue(source, VariableInfo::FromLocation({Operand::sta, AddStackVariable(source.identifier)->offset}), source.identifier, source.value.size);
    if (Optimizations::checkPotentialUselessStores) {
        storesToCheck.emplace_back(finalInstructions.size() - 1, source.identifier);
    }
}

// a wrapper for CopyVariableInfo to check if the variable exists elsewhere
void MoveVariableElsewhereNoReference(VariableInfo source) {
    MoveVariableElsewhere(source);
}
void MoveVariableElsewhere(VariableInfo& source) {
    for (size_t n = 0; n < generatorMemory.registers.size(); n++) {
        if (generatorMemory.registers[n].content.value == source.identifier and 
        source.location.type != Operand::reg and source.location.number != n) {
            // it's duplicated, end the function
            return;
        }
    }

    for (auto & n : generatorMemory.stack) {
        if (n.content.value == source.identifier and
            source.location.type != Operand::sta and source.location.offset != n.offset) {
            // it's duplicated, end the function
            return;
        }
    }

    CopyVariableElsewhere(source);
}

ParserVariable& VariableInfo::extractGlobalVariable() const{
    if (not identifier.contains("glob.")) {
        CodeGeneratorError("Bug: Tried to extract global from non global variable!");
    }
    uint64_t index = 3;
    if (value.subtype == Subtype::value) {
        index = identifier.find_first_of("$") + 1;
    }
    else if (value.subtype == Subtype::pointer) {
        index = identifier.find_first_of("*") + 1;
    }

    // now return the variable with the name without the #
    return globalVariablesOLD[identifier.substr(index, identifier.find_last_of("#") - index)];
}

// this places the value from source, to target, using specified type
// assumes source and target are correct
void X86_64PlaceConvertedValue(VariableInfo source, VariableInfo target) {
    // this should be fairly simple
    // TODO: add cases for conversion edge cases
    if (source.value.type != Value::floatingPoint and target.value.type != Value::floatingPoint) {
        // we have two integers so that's laughably simple
        DEPRECATEDInstruction ins;
        
        // determine the instruction type
        if (source.value.size < target.value.size) {
            if (source.value.type == Value::unsignedInteger) {
                ins.type = x86_64::OLD_movzx;
            }
            else {
                ins.type = x86_64::OLD_movsx;
            }
        }
        else {
            ins.type = x86_64::OLD_mov;
        }
        
        // now assign sizes
        ins.sizeAfter = target.value.size;
        ins.sizeBefore = source.value.size;
        // if the size after is smaller, it's a mov of part of value so do an equal size move
        if (ins.sizeBefore > ins.sizeAfter) {
            ins.sizeBefore = ins.sizeAfter;
        }

        // and now the operands themselves, the simplest part
        ins.op1 = target.location;
        ins.op2 = source.location;
        finalInstructions.push_back(ins);
        SetContent(target.location, source.identifier);
    }
        
    else {
        CodeGeneratorError("Unimplemented: x86-64 floating point conversions!");
    }
}

// finds the best candidate register to move a value to
DataLocation FindViableRegister(DataLocation reserved) {
    for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
        if (generatorMemory.registers[n].content.value == "!") {
            if (reserved.type == Operand::reg and n == reserved.number) {
                continue;
            }
            return {Operand::reg, n};
        }
    }
    for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
        if (generatorMemory.registers[n].content.value.starts_with("$")) {
            if (reserved.type == Operand::reg and n == reserved.number) {
                continue;
            }
            return {Operand::reg, n};
        }
    }
    CodeGeneratorError("Unimplemented: Complex register finding!");
    return {};
}

// does a move only, only check if operand types are right and if stack locations are of the correct size
// route register refers to a register used to facilitate stack to stack transfers
// DOES NOT set content in controllable ways
void X86_64PureMove(DataLocation source, DataLocation target, uint64_t size, bool findRouteRegister = false) {
    if (size > 8) {
        CodeGeneratorError("Unimplemented: Multimoves!");
    }
    DEPRECATEDInstruction ins;
    ins.type = x86_64::OLD_mov;
    ins.sizeAfter = ins.sizeBefore = size;
    ins.op2 = source;
    if (source.type == Operand::sta) {
        if (target.type == Operand::sta or source.extractAddress) {
            if (not findRouteRegister) {
                CodeGeneratorError("Bug: unexpected stack to stack move");
            }
            auto where = FindViableRegister();
            X86_64PureMove(source, where, size);
            SetContent(where, FindStackVariableByOffset(source.offset)->content.value);
            ins.op2 = where;
        }
    }
    ins.op1 = target;

    finalInstructions.push_back(ins);
}

// moves a variable to it's assigned stack position
void MoveAssignedToStack(VariableInfo& var) {
    // it assumes the calculated location is correct
    auto& life = variableLifetimes[var.identifier];
    for (uint64_t n = 0; n < generatorMemory.stack.size(); n++) {
        if (generatorMemory.stack[n].offset < life.staOffset) {
            internal::StackEntry temp(life.staOffset, 1, var.value.size, internal::ContentEntry(var.identifier));
            generatorMemory.stack.emplace(generatorMemory.stack.begin() + n, std::move(temp));
            var.location.type = Operand::sta;
            var.location.offset = life.staOffset;
            return;
        }
    }

    // if it gets here then there is no variable with more negative offset so just push it on the back
    internal::StackEntry temp(life.staOffset, 1, var.value.size, internal::ContentEntry(var.identifier));
    generatorMemory.stack.emplace_back(std::move(temp));
    var.location.type = Operand::sta;
    var.location.offset = life.staOffset;
}

// places the value from a pointer into the target, does not set content!
void X86_64DereferencePointer(VariableInfo source, VariableInfo target) {
    if (source.value.subtype == Subtype::value and not source.value.isAddress) {
        CodeGeneratorError("Bug: tried to dereference a non-pointer!");
    }
    if (source.location.type != Operand::reg) {
        auto where = FindViableRegister();
        MoveValue(source, VariableInfo::FromLocation(where), source.identifier, Options::addressSize);
        source.location = where;
        source.location.extractAddress = true;
    }
    if (target.location.type != Operand::reg) {
        CodeGeneratorError("Bug: tried to dereference a variable into a non register location!");
    }
    
    // ensure there is nothing in the target
    if (target.value.type != ParserType::none and not target.identifier.contains("glob.")) {
        MoveVariableElsewhere(target);
        SetContent(target.location, "!");
    }

    target.value = source.value;
    // now we just need to dereference the variable itself
    X86_64PlaceConvertedValue(VariableInfo(GetVariableType(source.identifier), {Operand::reg, source.location.number, true}, source.identifier), target);
}

// gets the address of the source variable into the designated location
void X86_64GetVariableAddress(VariableInfo source, VariableInfo target, std::string contentToSet) {
    if (source.identifier.contains("glob.")) {
        X86_64PureMove({Operand::aadr, &source.extractGlobalVariable()}, target.location, Options::addressSize);
        SetContent(target.location, contentToSet);
        return;
    }

    // if it's not a global variable then we need to ensure it actually exists in the stack
    if (source.location.type != Operand::sta) {
        auto& life = variableLifetimes[source.identifier];
        if (life.assignStatus != Operand::sta) {
            CodeGeneratorError("Bug: Variable that has it's address taken wasn't assigned to stack!");
        }

        // in that case it's correctly assigned to the stack but not there, that's unacceptable
        // but let's check if it didn't just grab the wrong variable instance
        if (FindStackVariableByOffset(life.staOffset) != nullptr) {
            source.location.type = Operand::sta;
            source.location.offset = life.staOffset;
        }
        else {
            // we need to move the variable to the stack
            MoveAssignedToStack(source);
        }
    }

    // now we are sure the value is in stack, we can finally get the address
    DEPRECATEDInstruction ins;
    ins.type = x86_64::OLD_lea;
    ins.op1 = target.location;
    ins.op2 = source.location;
    ins.sizeAfter = ins.sizeBefore = Options::addressSize;
    if (target.location.type != Operand::reg) {
        CodeGeneratorError("Bug: Tried to load variable address not into register!");
    }

    finalInstructions.push_back(ins);
    SetContent(target.location, contentToSet);
}

// this places a global variable in given place
void PlaceGlobalVariable(VariableInfo source, VariableInfo target, bool addressOnly = false) {
    if (target.location.type != Operand::reg) {
        CodeGeneratorError("Unimplemented: Unsupported global variable placement point!");
    }

    // for now only registers so it's simple
    // get the vase variable
    auto& base = source.extractGlobalVariable();

    // put the pointer into a register
    DEPRECATEDInstruction ins;
    ins.type = x86_64::OLD_mov;
    ins.sizeAfter = ins.sizeBefore = Options::addressSize;
    ins.op1 = FindViableRegister();
    ins.op2 = {Operand::aadr, &base};
    finalInstructions.push_back(ins);
    SetContent(ins.op1, "^" + source.identifier);
    if (addressOnly) {
        return;
    }

    // now that the pointer is there we can move the value from it into the register
    if (base.type == source.value) {
        // they are of the same type so just extract the value
        ins.sizeAfter = ins.sizeBefore = source.value.size;
        ins.op1 = target.location;
        ins.op2 = {Operand::reg, target.location.number, true};
        finalInstructions.push_back(ins);
        SetContent(target.location, source.identifier);
    }
    else {
        X86_64PlaceConvertedValue({base.type, {Operand::reg, target.location.number, true}, source.identifier}, {source.value, target.location, target.identifier});
    }
}

void SetValueAtAddress(VariableInfo source, uint64_t addressRegister, uint64_t size) {
    X86_64PureMove(source.location, {Operand::reg, addressRegister, true}, size);
}

std::string shutupcompiler = "";

const std::string& ContentAtLocation(DataLocation location) {
    switch (location.type) {
        case Operand::reg:
            return generatorMemory.registers[location.value].content.value;
        case Operand::sta:
            return FindStackVariableByOffset(location.offset)->content.value;
    }
    CodeGeneratorError("Unimplemented: Non stack/register content set!");
    return shutupcompiler;
}

// new MoveValue implementation with much better functionality support
void MoveValue(VariableInfo source, VariableInfo target, std::string contentToSet, uint64_t operationSize) {
    if (ContentAtLocation(target.location) == contentToSet) {
        // they are exactly the same, so there is no need for a move
        return;
    }

    auto contentType = VariableType(contentToSet);
    if (contentType.isAddress != source.value.isAddress and contentType.type != Value::none) {
        CodeGeneratorError("Bug: Invalid type combination in value move!");
    }
    
    if (target.value.type != Value::none and not target.identifier.contains("glob.") and target.location.type == Operand::reg) {
        // there is something in the target location, it needs to be copied elsewhere 
        MoveVariableElsewhere(target);
        SetContent(target.location, "!");
    }

    // in that case we have things to do
    if (source.identifier.contains("glob.")) {
        if (WasGlobalLocalized(source.identifier)) {
            auto base = VariableInfo::FromLocation(generatorMemory.findThing(FindMainName(source.identifier)));
            if (source.value == base.value) {
                source.location = base.location;
            }
            else {
                source.location.type = Operand::convert;
            }
        }
        else {
            // it's a global variable, these need different handling, let's do it in a different function
            PlaceGlobalVariable(source, target);
            return;
        }
    }
    
    // at this point the original target location can be assumed to be empty and ready for input, hurray!
    if (source.location.type != Operand::convert) {
        // Even more hooray! This variable exists so it just needs to be moved into place and that's it!
        if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
            DEPRECATEDInstruction ins;
            ins.type = x86_64::OLD_mov;
            if (source.value.isAddress) {
                // in that case we're moving a pointer
                ins.sizeBefore = ins.sizeAfter = Options::addressSize;
                ins.op2 = source.location;
                ins.op2.extractAddress = false;
                ins.op1 = target.location;
                ins.op1.extractAddress = false;
            }
            else {
                ins.sizeBefore = ins.sizeAfter = operationSize;
                ins.op2 = source.location;
                ins.op1 = target.location;
            }
            if (ins.op1.type == Operand::sta and ins.op2.type == Operand::sta) {
                CodeGeneratorError("Unimplemented: X86-64 stack to stack move!");
            }
            finalInstructions.push_back(ins);
            SetContent(target.location, contentToSet);
        }
        else {
            CodeGeneratorError("Unimplemented: non x86-64 move!");
        }
        return;
    }

    // conversion here
    if (source.location.type == Operand::imm) {
        X86_64PureMove(source.location, target.location, target.value.size);
        SetContent(target.location, contentToSet);
        return;
    }
    X86_64PlaceConvertedValue(VariableInfo(FindMainName(source.identifier)), {source.value, target.location, source.identifier});
    SetContent(target.location, contentToSet);
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
                if (n.second.assignStatus == Operand::reg) {
                    if (generatorMemory.registers[n.second.regNumber].content.value != n.first) {
                        MoveValue(VariableInfo(n.first), VariableInfo("%" + std::to_string(n.second.regNumber)), n.first, GetVariableType(n.first).size);
                        changes++;
                        SetContent(data, "!");
                    }
                }
                else if (n.second.assignStatus == Operand::sta) {
                    if (FindStackVariableByName(n.first) == nullptr) {
                        auto* sta = AddStackVariable(n.first);
                        sta->content.value = "!";
                        MoveValue(VariableInfo(n.first), VariableInfo("@" + std::to_string(sta->offset)), n.first, GetVariableType(n.first).size);
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

    DEPRECATEDInstruction ins;
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
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
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
                if (operands[n].first.contains("glob.")) {
                    // it's a global variable
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
                    continue;
                }
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
                        // look out for this
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
                        if (life.assignStatus == Operand::reg) {
                            if (current.number != life.regNumber) {
                                operandsToMove.emplace_back(n, MoveStruct::copy, DataLocation(Operand::reg, uint64_t(life.regNumber)));
                                continue;
                            }
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
                MoveValue(VariableInfo(operands[n.number].first), VariableInfo("@" + std::to_string(n.where.offset)), operands[n.number].first, req.instructionSize);
                break;
            case Operand::reg:
                MoveValue(VariableInfo(operands[n.number].first), VariableInfo("%" + std::to_string(n.where.value)), operands[n.number].first, req.instructionSize);
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
            MoveValue(VariableInfo("$" + std::to_string(n.second)), VariableInfo("%" + std::to_string(n.first)), "$" + std::to_string(n.second), req.instructionSize);
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
        if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
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
    if (exp.starts_with("$")) {
        // in that case we just need to increment all the instances of the variable
        return;
    }
    if (var.contains("glob.")) {
        // in that case we also need to update the global variable
        // we're moving value into a global variable, we need to get its address into a register
        // after that it should be a normal move
        DataLocation where = FindViableRegister();
        X86_64GetVariableAddress(VariableInfo(var), VariableInfo::FromLocation(where), "^" + var);
        DEPRECATEDInstruction ins;
        if (Options::targetArchitecture != Options::TargetArchitecture::x86_64) {
            CodeGeneratorError("Unimplemented: Non x86-64 global assignment!");
        }
        VariableInfo source(var);
        ins.sizeAfter = ins.sizeBefore = source.value.size;
        ins.type = x86_64::OLD_mov;
        where.type = Operand::reg;
        where.extractAddress = true;
        ins.op1 = where;
        ins.op2 = generatorMemory.findThing(exp);
        if (ins.op2.type == Operand::none) {
            CodeGeneratorError("Bug: Move of non existent variable to global!");
        }
        finalInstructions.push_back(ins);
    }
    for (auto& n : generatorMemory.registers) {
        if (n.content.value == var) {
            n.content.value = exp;
        }
    }
    for (auto& n : generatorMemory.stack) {
        if (n.content.value == var) {
            n.content.value = exp;
        }
    }
}