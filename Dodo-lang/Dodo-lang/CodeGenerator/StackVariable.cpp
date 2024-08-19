#include "StackVector.hpp"
#include "GenerateCode.hpp"

StackVariable &StackVector::find(const std::string& name) {
    for (auto& n : vec) {
        if (n.name == name) {
            return n;
        }
    }
    CodeError("Variable: " + name + " not found!");
}

uint64_t StackVector::lastOffset() {
    if (not vec.empty()) {
        return vec.back().offset;
    }
    return 0;
}
