#ifndef DODO_LANG_LINEAR_ANALYSIS_HPP
#define DODO_LANG_LINEAR_ANALYSIS_HPP

#include <string>
#include <unordered_map>
#include <cstdint>
#include "MapWrapper.tpp"

struct VariableStatistics {
    uint64_t firstUse = 0;
    uint64_t lastUse = 0;
    uint64_t usageAmount:47 = 0;
    enum AssignStatus {
        none, reg, sta
    };
    uint8_t assignStatus:2 = none;
    // assigned register if there is one
    uint16_t assigned:14 = 0;
    VariableStatistics(uint64_t number);
    VariableStatistics() = default;
};

inline MapWrapper <std::string, VariableStatistics> variableLifetimes;

void RunLinearAnalysis();

#endif //DODO_LANG_LINEAR_ANALYSIS_HPP
