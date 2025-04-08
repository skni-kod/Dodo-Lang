#ifndef DODO_LANG_LINEAR_ANALYSIS_HPP
#define DODO_LANG_LINEAR_ANALYSIS_HPP

#include <string>
#include <unordered_map>
#include <cstdint>
#include "MapWrapper.tpp"
#include "MemoryStructure.hpp"

struct VariableStatistics {
    uint64_t firstUse = 0;
    uint64_t lastUse = 0;
    uint64_t usageAmount: 54 = 0;
    uint64_t isMainValue: 1 = false;
    uint64_t isPointedTo: 1 = false;
    uint64_t assignStatus:8 = Operand_Old::none;
    // assigned register if there is one
    union {
        uint64_t regNumber = 0;
        int64_t staOffset;
    };
    
    DataLocation toLocation();
    

    VariableStatistics(uint64_t number, bool isMain = false, bool isPointedTo = false);

    VariableStatistics() = default;
};

inline MapWrapper<std::string, VariableStatistics> variableLifetimes;
// offset of the last element in the precalculated stack
inline int64_t minimumSafeStackOffset = 0;

void RunLinearAnalysis();

bool WasGlobalLocalized(std::string& name);
VariableStatistics& FindMain(std::string& child);
const std::string& FindMainName(std::string& child);
inline const std::string* lastMainName = nullptr;

std::string GetNextVariableAssignment(const std::string& var);
std::string GetPreviousVariableAssignment(const std::string& var);

#endif //DODO_LANG_LINEAR_ANALYSIS_HPP
