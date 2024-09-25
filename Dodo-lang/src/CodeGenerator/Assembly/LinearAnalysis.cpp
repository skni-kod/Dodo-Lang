#include "LinearAnalysis.hpp"
#include "../Bytecode/Bytecode.hpp"
#include "MemoryStructure.hpp"

VariableStatistics::VariableStatistics(uint64_t number) : firstUse(number), lastUse(number) {}

bool IsVariable(const std::string& input) {
    for (auto& n: input) {
        if ((n < '0' or n > '9') and n != '-' and n != '.') {
            return true;
        }
    }
    return false;
};

void CalculateLifetimes() {
    for (uint64_t n = 0; n < bytecodes.size(); n++) {
        switch (bytecodes[n].code) {
            case Bytecode::declare:
                variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].target, {n});
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                        temp.isMainValue = true;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                    }
                }
                break;
            case Bytecode::add:
            case Bytecode::subtract:
            case Bytecode::multiply:
            case Bytecode::divide:
                // create the expression
                variableLifetimes.insert(
                        bytecodes[n].type.GetPrefix() + "=#" + std::to_string(bytecodes[n].number) + "-0", {n});
                // handle source
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                        // this is not the original value and as such the original one must still exist to convert from
                        const std::string& ending = bytecodes[n].source;
                        for (auto& current: variableLifetimes.map) {
                            if (current.first.ends_with(ending)) {
                                current.second.usageAmount++;
                                current.second.lastUse = n;
                            }
                        }
                    }
                }
                // handle target
                if (IsVariable(bytecodes[n].target)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].target)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].target);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].target, {n});
                        // this is not the original value and as such the original one must still exist to convert from
                        const std::string& ending = bytecodes[n].target;
                        for (auto& current: variableLifetimes.map) {
                            if (current.first.ends_with(ending)) {
                                current.second.usageAmount++;
                                current.second.lastUse = n;
                            }
                        }
                    }
                }
                break;
            case Bytecode::compare:
                // handle source
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                    }
                }
                // handle target
                if (IsVariable(bytecodes[n].target)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].target)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].target);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].target, {n});
                    }
                }
                break;
            case Bytecode::returnValue:
                // handle source
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                    }
                }
                break;
            case Bytecode::moveArgument:
                // handle source
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                    }
                }
                break;
            case Bytecode::assign:
                // handle target
            {
                variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].target, {n});
                auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].target);
                temp.usageAmount++;
                temp.lastUse = n;
                temp.isMainValue = true;
            }
                // handle source
                if (IsVariable(bytecodes[n].source)) {
                    if (variableLifetimes.isKey(bytecodes[n].type.GetPrefix() + bytecodes[n].source)) {
                        auto& temp = variableLifetimes.find(bytecodes[n].type.GetPrefix() + bytecodes[n].source);
                        temp.usageAmount++;
                        temp.lastUse = n;
                    }
                    else {
                        variableLifetimes.insert(bytecodes[n].type.GetPrefix() + bytecodes[n].source, {n});
                    }
                }
                break;
        }
    }
}

std::vector<std::pair<std::string, VariableStatistics>> lifetimeVector;

bool LifetimeVectorSorter(const std::pair<std::string, VariableStatistics>& l,
                          const std::pair<std::string, VariableStatistics>& r) {
    if (r.second.usageAmount > l.second.usageAmount) {
        return false;
    }
    return true;
}

struct AssignedRegister {
    uint64_t reg = 0;
    std::vector<uint64_t> assigned;

    explicit AssignedRegister(uint64_t reg) : reg(reg) {}
};

std::vector<AssignedRegister> assignments;

void PrepareLifetimesForAnalysis() {
    // first put them in the vector
    for (auto& n: variableLifetimes.map) {
        lifetimeVector.emplace_back(n.first, n.second);
    }

    // now sort them by usage count
    std::sort(lifetimeVector.begin(), lifetimeVector.end(), &LifetimeVectorSorter);

    // and after that prepare usable registers
    for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
        if (generatorMemory.registers[n].usedForStorage) {
            assignments.emplace_back(n);
        }
    }
}

void RunLinearAnalysis() {
    variableLifetimes.map.clear();

    // first, track first, last and amount of uses for every variable
    CalculateLifetimes();

    // next it's time to assign registers to variables
    PrepareLifetimesForAnalysis();

    // and finally it's time to assign variables to registers
    // go through all the variables starting with the most used ones
    for (uint64_t n = 0; n < lifetimeVector.size(); n++) {
        auto& var = lifetimeVector[n].second;
        bool found = false;

        // try to find an unused register
        for (auto& reg: assignments) {
            bool isValid = true;
            for (auto& m: reg.assigned) {
                auto& current = lifetimeVector[m].second;
                // check if the already assigned variable was used during the needed time
                if (current.firstUse > var.lastUse or current.lastUse < var.firstUse) {
                    continue;
                }
                // if it got here then it was, move on
                isValid = false;
                break;
            }
            if (isValid) {
                reg.assigned.push_back(n);
                auto& temp = variableLifetimes[lifetimeVector[n].first];
                temp.assigned = reg.reg;
                temp.assignStatus = VariableStatistics::AssignStatus::reg;
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }

        // if it got here then there was no space in the lovely registers of ours, off to stack with it
        variableLifetimes[lifetimeVector[n].first].assignStatus = VariableStatistics::AssignStatus::sta;
    }

    if (options::informationLevel == options::InformationLevel::full) {
        uint64_t stacks = 0;
        uint64_t regs = 0;
        for (auto& n: variableLifetimes.map) {
            if (n.second.assignStatus == VariableStatistics::AssignStatus::sta) {
                stacks++;
            }
        }
        for (auto& n: assignments) {
            regs += not n.assigned.empty();
        }
        std::cout << "INFO L3: Linear analysis for this function concluded with: " << lifetimeVector.size()
                  << " targets and: "
                  << regs << " out of: " << assignments.size() << " registers used and: " << stacks
                  << " variables on stack\n";
    }

    assignments.clear();
    lifetimeVector.clear();
}

