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

StackVariable &StackVector::find(uint64_t offset) {
    for (auto& n : vec) {
        for (auto& m : n) {
            if (m.offset == offset) {
                return m;
            }
        }
    }
    CodeError("Variable with offset: " + std::to_string(offset) + " not found at this point!");
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

const StackVariable &StackVector::push(StackVariable var) {
    // for now only places on the back
    // TODO: add stack compression
    if (vec.back().empty()) {
        var.offset = var.size * var.amount;
    }
    else {
        var.offset = vec.back().back().offset + (var.size * var.amount);
    }
    // aligning the variables
    if (var.offset % var.size != 0) {
        var.offset = (var.offset / var.size + 1) * var.size;
    }
    vec.back().push_back(var);
    return vec.back().back();
}

std::string StackVector::pushAndStr(StackVariable var) {
    if (vec.back().empty()) {
        var.offset = var.size * var.amount;
    }
    else {
        var.offset = vec.back().back().offset + (var.size * var.amount);
    }
    // aligning the variables
    if (var.offset % var.size != 0) {
        var.offset = (var.offset / var.size + 1) * var.size;
    }
    vec.back().push_back(var);
    return '-' + std::to_string(vec.back().back().offset) + "(%rbp)";
}

void StackVector::free_back() {
    if (vec.back().empty()) {
        CodeError("Cannot free from an empty stack!");
    }
    vec.back().pop_back();
}

void StackVector::free(uint64_t offset) {
    if (vec.back().empty()) {
        CodeError("Cannot free from an empty stack!");
    }

    for (uint64_t n = 0; n < vec.size(); n++) {
        for (uint64_t m = 0; m < vec[n].size(); m++) {
            if (vec[n][m].offset == offset) {
                vec[n].erase(vec[n].begin() + m);
                return;
            }
        }
    }
    CodeError("Element with given offset does not exist!");
}

void StackVector::free(std::string result) {
    if (result.size() < 8) {
        CodeError("Invalid result passed to free!");
    }
    uint64_t offset = std::stoll(result.substr(1, result.size() - 7));
    free(offset);
}
