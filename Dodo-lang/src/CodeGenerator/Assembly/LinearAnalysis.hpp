#ifndef DODO_LANG_LINEAR_ANALYSIS_HPP
#define DODO_LANG_LINEAR_ANALYSIS_HPP

#include <string>
#include <unordered_map>
#include <cstdint>
#include "MapWrapper.tpp"

struct VariableStatistics {
    uint64_t firstUse = 0;
    uint64_t lastUse = 0;
    uint64_t usageAmount: 55 = 0;
    enum AssignStatus {
        none = 0, convert = 0, reg, sta, imm,
        // aadr is assember address
        aadr, var, replace, jla, fun
    };
    uint8_t isMainValue: 1 = false;
    uint8_t assignStatus = AssignStatus::none;
    // assigned register if there is one
    union {
        uint64_t regNumber = 0;
        int64_t staOffset;
    };
    

    VariableStatistics(uint64_t number, bool isMain = false);

    VariableStatistics() = default;
};

inline MapWrapper<std::string, VariableStatistics> variableLifetimes;
// offset of the last element in the precalculated stack
inline int64_t minimumSafeStackOffset = 0;

void RunLinearAnalysis();

VariableStatistics& FindMain(std::string& child);
inline const std::string* lastMainName = nullptr;

#endif //DODO_LANG_LINEAR_ANALYSIS_HPP
