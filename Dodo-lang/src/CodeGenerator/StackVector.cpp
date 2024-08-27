#include "StackVector.hpp"
#include "GenerateCode.hpp"

std::string StackVariable::getAddress() const {
    return '-' + std::to_string(offset) + "(%rbp)";
}

RegisterNames StackVariable::getAddressAsRegisterNames() const {
    return {'-' + std::to_string(offset) + "(%rbp)",
            '-' + std::to_string(offset) + "(%rbp)",
            '-' + std::to_string(offset) + "(%rbp)",
            '-' + std::to_string(offset) + "(%rbp)"};
}

StackVariable &StackVector::findByOffset(const std::string& offset) {
    uint64_t intOffset = std::stoll(offset.substr(1, offset.size() - 7));
    for (auto& n : vec) {
        for (auto& m : n) {
            if (m.offset == intOffset) {
                return m;
            }
        }
    }
    CodeError("Variable at offset: " + std::to_string(intOffset) + " not found at this point!");
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
}

StackVariable &StackVector::find(const std::string& name) {
    for (auto& n : vec) {
        for (auto& m : n) {
            if (m.name == name) {
                return m;
            }
        }
    }
    CodeError("Variable: " + name + " not found at this point!");
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
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
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
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
    // searching for a place the variable can possibly fit that is not the back of the stack
    if (not vec.back().empty()) {
        // if there is enough space at the front to fit the variable
        if (vec.back().size() == 1) {
            uint64_t next = vec.back().front().offset - (vec.back().front().singleSize * vec.back().front().amount);
            if (next >= var.singleSize * var.amount) {
                var.offset = var.singleSize * var.amount;
                vec.back().insert(vec.back().begin(), var);
                return vec.back().front();
            }
        }
        for (uint64_t n = 0; n < vec.back().size() - 1; n++) {
            // get the end of free space
            uint64_t next = vec.back()[n + 1].offset - (vec.back()[n + 1].singleSize * vec.back()[n + 1].amount);
            uint64_t previous = vec.back()[n].offset;
            if (next - previous >= var.singleSize * var.amount) {
                if (next % var.singleSize != 0) {
                    next = (next / var.singleSize - 1) * var.singleSize;
                    if (next - previous < var.singleSize * var.amount) {
                        // not enough space after alignment
                        continue;
                    }
                    // space is correct, insert the variable between n and n + 1
                    var.offset = next;
                    vec.back().insert(vec.back().begin() + n + 1, var);
                    return vec.back()[n + 1];
                }
            }
        }
    }

    bool found = false;
    for (int64_t n = vec.size() - 1; n >= 0; n--) {
        if (!vec[n].empty()) {
            found = true;
            var.offset = vec[n].back().offset + var.singleSize * var.amount;
            break;
        }
    }
    if (not found) {
        var.offset = var.singleSize * var.amount;
    }

    // aligning the variables
    if (var.offset % var.singleSize != 0) {
        var.offset = (var.offset / var.singleSize + 1) * var.singleSize;
    }
    vec.back().push_back(var);
    return vec.back().back();
}

std::string StackVector::pushAndStr(StackVariable var) {
    return '-' + std::to_string(push(var).offset) + "(%rbp)";
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
