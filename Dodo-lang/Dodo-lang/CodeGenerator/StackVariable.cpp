#include "StackVector.hpp"
#include "GenerateCode.hpp"

StackVariable &StackVector::find(const std::string& name) {
    for (auto& n : vec) {
        for (auto& m : n) {
            if (m.name == name) {
                return m;
            }
        }
    }
    CodeError("Variable: " + name + " not found at this point!");
}

uint64_t StackVector::lastOffset() {
    if (not vec.empty() and not vec.back().empty()) {
        return vec.back().back().offset;
    }
    return 0;
}

StackVector::StackVector() {
    // so that the first level is always here
    vec.emplace_back();
}
