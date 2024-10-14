#include "LinearAnalysis.hpp"
#include "../Bytecode/Bytecode.hpp"
#include "MemoryStructure.hpp"
#include "TheGenerator.hpp"

VariableStatistics::VariableStatistics(uint64_t number, bool isMain) : firstUse(number), lastUse(number), isMainValue(isMain) {}

DataLocation VariableStatistics::toLocation() {
    DataLocation data;
    data.type = assignStatus;
    if (data.type == Operand::reg) {
        data.number = regNumber;
        return data;
    }

    if (data.type == Operand::sta) {
        data.offset = staOffset;
        return data;
    }

    CodeGeneratorError("Bug: invalid assigned location!");
    return data;
}

bool IsVariable(const std::string& input) {
    return input.front() == 'u' or input.front() == 'i' or input.front() == 'f';
};

VariableStatistics VSDummy;

bool WasGlobalLocalized(std::string& name) {
    std::string searched = name.substr(2, name.size() - 2);
    for (auto& n : variableLifetimes.map) {
        if (n.first.ends_with(searched) and n.second.isMainValue and n.second.firstUse < currentBytecodeIndex) {
            return true;
        }
    }
    return false;
}

VariableStatistics& FindMain(std::string& child) {
    std::string searched = child.substr(2, child.size() - 2);
    for (auto& n : variableLifetimes.map) {
        if (n.first.ends_with(searched) and n.second.isMainValue) {
            lastMainName = &n.first;
            return n.second;
        }
    }
    CodeGeneratorError("Bug: Could not find main value to add lifetime!");
    return VSDummy;
}

std::string dummyString;

const std::string& FindMainName(std::string& child) {
    std::string searched = child.substr(2, child.size() - 2);
    for (auto& n : variableLifetimes.map) {
        if (n.first.ends_with(searched) and n.second.isMainValue) {
            lastMainName = &n.first;
            return n.first;
        }
    }
    CodeGeneratorError("Bug: Could not find main value to add lifetime!");
    return dummyString;
}

void AddLifetimeToMain(std::string& child, uint64_t index) {
    auto& main = FindMain(child);
    main.lastUse = index;
    main.usageAmount++;
}

// this ensures that the lifetime of variables is extended until the end of a level
std::stack <std::vector <std::string>> variablesToExtend;

void AddLifetimeOrInsert(std::string& name, uint64_t index) {
    if (IsVariable(name)) {
        if (variableLifetimes.isKey(name)) {
            auto& temp = variableLifetimes.find(name);
            temp.usageAmount++;
            temp.lastUse = index;
            if (not temp.isMainValue) {
                AddLifetimeToMain(name, index);
                if (not variablesToExtend.empty() and not Optimizations::groupVariableInstances) {
                    variablesToExtend.top().push_back(*lastMainName);
                }
            }
        }
        else {
            if (name.contains("glob.")) {
                // it's a global variable, check it's actual type first and add main if it exists
                auto& global = VariableInfo(name).extractGlobalVariable();
                if (not name.starts_with(global.typeOrName)) {
                    // check if the main exists only if the checked variable is of a differrent type
                    bool found = false;
                    for (auto& n : variableLifetimes.map) {
                        if (n.first.starts_with(global.typeOrName) and n.second.isMainValue) {
                            // it's a main value, in that case 
                            found = true;
                            break;
                        }
                    }
                    if (not found) {
                        // in that case we also need to insert the global variable
                        variableLifetimes.insert(global.typeOrName + "#0-0", {index, true});
                    }
                }
                else {
                    variableLifetimes.insert(name, {index, true});
                    if (not variablesToExtend.empty() and not Optimizations::groupVariableInstances) {
                        variablesToExtend.top().push_back(*lastMainName);
                    }
                    return;
                }
                
            }
            
            variableLifetimes.insert(name, {index});
            // this is not the original value and as such the original one must still exist to convert from
            AddLifetimeToMain(name, index);
            if (not variablesToExtend.empty() and not Optimizations::groupVariableInstances) {
                variablesToExtend.top().push_back(*lastMainName);
            }
        }
    }
}

void CalculateLifetimes() {
    for (uint64_t n = 0; n < bytecodes.size(); n++) {
        switch (bytecodes[n].code) {
            case Bytecode::declare:
                variableLifetimes.insert(bytecodes[n].target, {n});
                variableLifetimes[bytecodes[n].target].isMainValue = true;
                AddLifetimeOrInsert(bytecodes[n].source, n);
                break;
            case Bytecode::addFromArgument:
                variableLifetimes.insert(bytecodes[n].target, {n});
                variableLifetimes[bytecodes[n].target].isMainValue = true;
                break;
            case Bytecode::add:
            case Bytecode::subtract:
            case Bytecode::multiply:
            case Bytecode::divide:
                // create the expression
                variableLifetimes.insert(bytecodes[n].type.GetPrefix() +
                        "=#" + std::to_string(bytecodes[n].number) + "-0", {n, true});
                // handle source
                AddLifetimeOrInsert(bytecodes[n].source, n);
                // handle target
                AddLifetimeOrInsert(bytecodes[n].target, n);
                break;
            case Bytecode::callFunction:
                if (not bytecodes[n].target.empty()) {
                    variableLifetimes.insert(bytecodes[n].target, {n, true});
                }
                // also extend all arguments if they are present
                {
                    auto& fun = parserFunctions[bytecodes[n].source];
                    for (uint64_t k = n - 1, arguments = 0; arguments < fun.arguments.size(); k--) {
                        if (bytecodes[k].code == Bytecode::moveArgument) {
                            if (IsVariable(bytecodes[k].source)) {
                                // extend the life of the result
                                auto& temp = variableLifetimes.find(bytecodes[k].source);
                                temp.usageAmount++;
                                temp.lastUse = n;
                            }
                            arguments++;
                        }
                    }
                }
                break;
            case Bytecode::compare:
                // handle source
                AddLifetimeOrInsert(bytecodes[n].source, n);
                // handle target
                AddLifetimeOrInsert(bytecodes[n].target, n);
                break;
            case Bytecode::returnValue:
                // handle source
                AddLifetimeOrInsert(bytecodes[n].source, n);
                break;
            case Bytecode::moveArgument:
                // handle source
                AddLifetimeOrInsert(bytecodes[n].source, n);
                // now find the last function call to put into bytecode
                for (uint64_t k = n; n < bytecodes.size(); k++) {
                    if (bytecodes[k].code == Bytecode::callFunction) {
                        bytecodes[n].number = k;
                        break;
                    }
                }
                break;
            case Bytecode::moveValue:
            case Bytecode::assign:
                // handle target
            {
                variableLifetimes.insert(bytecodes[n].target, {n});
                auto& temp = variableLifetimes.find(bytecodes[n].target);
                temp.usageAmount++;
                temp.lastUse = n;
                temp.isMainValue = true;
            }
                // handle source
                AddLifetimeOrInsert(bytecodes[n].source, n);
                break;
            case Bytecode::pushLevel:
                variablesToExtend.emplace();
                break;
            case Bytecode::popLevel:
                for (auto& m : variablesToExtend.top()) {
                    variableLifetimes[m].lastUse = n;
                }
                variablesToExtend.pop();
                break;
        }
    }
}

std::vector<std::pair<std::string, VariableStatistics>> lifetimeVector;

bool LifetimeVectorSorter(const std::pair<std::string, VariableStatistics>& l,
                          const std::pair<std::string, VariableStatistics>& r) {
    if (float(r.second.usageAmount) / float(r.second.lastUse - r.second.firstUse + 1) >
        float(l.second.usageAmount) / float(l.second.lastUse - l.second.firstUse + 1)) {
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

void RunNaiveLinearAnalysis() {
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
                temp.regNumber = reg.reg;
                temp.assignStatus = Operand::reg;
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }

        // if it got here then there was no space in the lovely registers of ours, off to stack with it
        variableLifetimes[lifetimeVector[n].first].assignStatus = Operand::sta;
    }
}

bool GroupSorter(const std::tuple<std::vector <std::string>, uint64_t, uint64_t, uint64_t>& l,
                          const std::tuple<std::vector <std::string>, uint64_t, uint64_t, uint64_t>& r) {
    if (float(std::get<3>(r)) / float(std::get<2>(r) - std::get<1>(r) + 1) >
        float(std::get<3>(l)) / float(std::get<2>(l) - std::get<1>(l) + 1)) {
        return false;
    }
    return true;
}

struct AssignStackEntry {
    int64_t offset = 0;
    int64_t size = 0;
    AssignStackEntry(int64_t offset, int64_t size) : offset(offset), size(size) {}
};

std::vector <AssignStackEntry> assignmentStack;

int64_t AddStackAssignment(int64_t size) {
    // TODO: add lifetime support to stack
    // now let's find a place for this thing
    for (uint64_t n = 1; n < assignmentStack.size(); n++) {
        auto& b = assignmentStack[n - 1];
        auto& s = assignmentStack[n];
        int64_t space = b.offset - (s.offset + s.size);
        if (space >= size) {
            // check alignment here
            int64_t aligned = s.offset + (s.offset + s.size);
            if (aligned % size != 0) {
                aligned = (aligned / size - 1) * size;
            }
            if (b.offset - aligned >= size) {
                assignmentStack.emplace(assignmentStack.begin() + n, aligned, size);
                return aligned;
            }
        }
    }
    if (assignmentStack.empty()) {
        assignmentStack.emplace_back(-size, size);
        return -size;
    }
    else {
        int64_t offset = assignmentStack.back().offset - size;
        if (offset % size != 0) {
            offset = (offset / size - 1) * size;
        }
        assignmentStack.emplace_back(offset, size);
        return offset;
    }
};

void RunGroupingLinearAnalysis() {
    // this one is a bit more complex
    // first we need to group all the main variables together
    
    // vec, earliest, last, uses
    std::vector <std::tuple<std::vector <std::string>, uint64_t, uint64_t, uint64_t>> groups;
    // go through all the variables and find first assignments of the instances
    for (auto& current : variableLifetimes.map) {
        if (current.first.ends_with("#0-0")) {
            // now that we found the first one we need to check if it's the main value
            // only main values are grouped as converted ones will be temporary
            if (not current.second.isMainValue) {
                continue;
            }
            groups.emplace_back();
            // now we need to find all the variables that belong to this group
            // this one will be in format <type><name>#<instance>- without assignment number
            std::string searched = current.first.substr(0, current.first.find_last_of('-') + 1);
            uint64_t earliest = current.second.firstUse;
            uint64_t last = current.second.lastUse;
            uint64_t uses = 0;
            for (auto& n : variableLifetimes.map) {
                if (n.first.starts_with(searched)) {
                    if (n.second.lastUse > last) {
                        last = n.second.lastUse;
                    }
                    if (n.second.firstUse < earliest) {
                        earliest = n.second.firstUse;
                    }
                    uses += n.second.usageAmount;
                    std::get<0>(groups.back()).push_back(n.first);
                }
            }
            std::get<1>(groups.back()) = earliest;
            std::get<2>(groups.back()) = last;
            std::get<3>(groups.back()) = uses;
        }
    }
    // now all groups are ready, they need to be sorted
    std::sort(groups.begin(), groups.end(), &GroupSorter);

    // first put them in the vector
    for (auto& n: variableLifetimes.map) {
        if (not n.second.isMainValue) {
            lifetimeVector.emplace_back(n.first, n.second);
        }
    }

    // now sort them by usage count
    std::sort(lifetimeVector.begin(), lifetimeVector.end(), &LifetimeVectorSorter);

    // and after that prepare usable registers
    for (uint64_t n = 0; n < generatorMemory.registers.size(); n++) {
        if (generatorMemory.registers[n].usedForStorage) {
            assignments.emplace_back(n);
        }
    }

    uint64_t convertedSize = lifetimeVector.size();
    // after all that the analysis itself can begin, yay a double for loop
    for (uint64_t groupNumber = 0, convertedNumber = 0; groupNumber < groups.size() or convertedNumber < convertedSize;) {
        float groupRatio = 0, convertedRatio = 0;
        if (groupNumber < groups.size()) {
            groupRatio = float(std::get<3>(groups[groupNumber])) / float(std::get<2>(groups[groupNumber]) - std::get<1>(groups[groupNumber]) + 1.f);
        }
        if (convertedNumber < convertedSize) {
            convertedRatio = float(lifetimeVector[convertedNumber].second.usageAmount) / float(lifetimeVector[convertedNumber].second.lastUse - lifetimeVector[convertedNumber].second.firstUse + 1);
        }

        // now choose the more instruction dense one
        if (convertedRatio > groupRatio or groupNumber == groups.size()) {
            auto& var = lifetimeVector[convertedNumber].second;
            bool found = false;
            // a small one, this one will be easy
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
                    reg.assigned.push_back(convertedNumber);
                    auto& temp = variableLifetimes[lifetimeVector[convertedNumber].first];
                    temp.regNumber = reg.reg;
                    temp.assignStatus = Operand::reg;
                    found = true;
                    break;
                }
                else {

                }
            }
            if (found) {
                convertedNumber++;
                continue;
            }

            // if it got here then there was no space in the lovely registers of ours, off to stack with it
            
            auto& temp = variableLifetimes[lifetimeVector[convertedNumber].first];
            temp.assignStatus = Operand::sta;
            temp.staOffset = AddStackAssignment(GetVariableType(lifetimeVector[convertedNumber].first).size);
            convertedNumber++;
        }
        else if (groupNumber < groups.size()) {
            // the same but with the group and more complex when found
            VariableStatistics var;
            var.firstUse = std::get<1>(groups[groupNumber]);
            var.lastUse = std::get<2>(groups[groupNumber]);
            var.usageAmount = std::get<3>(groups[groupNumber]);
            bool found = false;
            // a small one, this one will be easy
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
                    for (auto& n : std::get<0>(groups[groupNumber])) {
                        reg.assigned.push_back(lifetimeVector.size());
                        std::pair <std::string, VariableStatistics> pair;
                        pair.first = n;
                        pair.second.firstUse = std::get<1>(groups[groupNumber]);
                        pair.second.lastUse = std::get<2>(groups[groupNumber]);
                        lifetimeVector.push_back(pair);
                        auto& temp = variableLifetimes[n];
                        temp.regNumber = reg.reg;
                        temp.assignStatus = Operand::reg;
                    }
                    found = true;
                    break;
                }
                else {

                }
            }
            if (found) {
                groupNumber++;
                continue;
            }

            // if it got here then there was no space in the lovely registers of ours, off to stack with it
            for (auto& n : std::get<0>(groups[groupNumber])) {
                auto& temp = variableLifetimes[n];
                temp.assignStatus = Operand::sta;
                temp.staOffset = AddStackAssignment(GetVariableType(lifetimeVector[convertedNumber].first).size);
            }
            groupNumber++;
        }
        else {
            CodeGeneratorError("Bug: Error in group analysis loop!");
        }
    }
    if (assignmentStack.empty()) {
        minimumSafeStackOffset = 0;
    }
    else {
        minimumSafeStackOffset = assignmentStack.back().offset;
    }
}

void RunLinearAnalysis() {
    variableLifetimes.map.clear();
    assignmentStack.clear();

    // first, track first, last and amount of uses for every variable
    CalculateLifetimes();

    if (Optimizations::groupVariableInstances) {
        RunGroupingLinearAnalysis();
    }
    else {
        PrepareLifetimesForAnalysis();
        RunNaiveLinearAnalysis();
    }

    if (Options::informationLevel == Options::InformationLevel::full) {
        uint64_t stacks = 0;
        uint64_t regs = 0;
        for (auto& n: variableLifetimes.map) {
            if (n.second.assignStatus == Operand::sta) {
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

