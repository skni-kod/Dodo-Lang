#include "StackVector.hpp"
#include "GenerateCode.hpp"

std::string StackVariable::getAddress() const {
    return std::to_string(offset) + "(%rbp)";
}

RegisterNames StackVariable::getAddressAsRegisterNames() const {
    return {std::to_string(offset) + "(%rbp)",
            std::to_string(offset) + "(%rbp)",
            std::to_string(offset) + "(%rbp)",
            std::to_string(offset) + "(%rbp)"};
}

StackVariable& StackVector::findByOffset(const std::string& offset) {

    int64_t intOffset = std::stoll(offset.substr(0, offset.size() - 6));
    if (intOffset > 0) {
        for (auto& n: arguments) {
            if (n.offset == intOffset) {
                return n;
            }
        }
    }
    else {
        for (auto& n: vec) {
            for (auto& m: n) {
                if (m.offset == intOffset) {
                    return m;
                }
            }
        }
    }
    CodeError("Variable at offset: " + std::to_string(intOffset) + " not found at this point!");
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
}

StackVariable& StackVector::find(const std::string& name) {
    for (auto& n: arguments) {
        if (n.name == name) {
            return n;
        }
    }
    for (auto& n: vec) {
        for (auto& m: n) {
            if (m.name == name) {
                return m;
            }
        }
    }
    CodeError("Variable: " + name + " not found at this point!");
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
}


StackVariable& StackVector::find(int64_t offset) {
    for (auto& n: arguments) {
        if (n.offset == offset) {
            return n;
        }
    }
    for (auto& n: vec) {
        for (auto& m: n) {
            if (m.offset == offset) {
                return m;
            }
        }
    }
    CodeError("Variable with offset: " + std::to_string(offset) + " not found at this point!");
    // will not be reached anyway, just to please the compiler
    return vec.back().back();
}

int64_t StackVector::lastOffset() {
    if (not vec.empty() and not vec.back().empty()) {
        return vec.back().back().offset;
    }
    return 0;
}

StackVector::StackVector() {
    // so that the first level is always here
    vec.emplace_back();
}

const StackVariable& StackVector::push(StackVariable var) {
    // searching for a place the variable can possibly fit that is not the back of the stack
    if (not vec.back().empty()) {
        // if there is enough space at the front to fit the variable
        // TODO: add squeezing variables between levels
        if (vec.back().size() == 1 and vec.size() == 1) {
            int64_t next = vec.back().front().offset + (vec.back().front().singleSize * vec.back().front().amount);
            if (-next >= var.singleSize * var.amount) {
                var.offset = -var.singleSize * var.amount;
                vec.back().insert(vec.back().begin(), var);
                return vec.back().front();
            }
        }
        for (uint64_t n = 0; n < vec.back().size() - 1; n++) {
            // get the end of free space
            int64_t next = vec.back()[n + 1].offset + (vec.back()[n + 1].singleSize * vec.back()[n + 1].amount);
            int64_t previous = vec.back()[n].offset;
            if (previous - next >= var.singleSize * var.amount) {
                if (next % var.singleSize != 0) {
                    next = (next / var.singleSize + 1) * var.singleSize;
                    if (previous - next < var.singleSize * var.amount) {
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
            var.offset = vec[n].back().offset - var.singleSize * var.amount;
            break;
        }
    }
    if (not found) {
        var.offset = -var.singleSize * var.amount;
    }

    // aligning the variables
    if (var.offset % var.singleSize != 0) {
        var.offset = (var.offset / var.singleSize - 1) * var.singleSize;
    }
    vec.back().push_back(var);
    return vec.back().back();
}

std::string StackVector::pushAndStr(StackVariable var) {
    return std::to_string(push(var).offset) + "(%rbp)";
}

void StackVector::free_back() {
    if (vec.back().empty()) {
        CodeError("Cannot free from an empty stack!");
    }
    vec.back().pop_back();
}

void StackVector::free(int64_t offset) {
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

void StackVector::free(std::string address) {
    if (address.size() < 7) {
        CodeError("Invalid result passed to free!");
    }
    free(std::stoll(address.substr(0, address.size() - 6)));
}

void StackVector::addLevel() {
    vec.emplace_back();
}

void StackVector::popLevel() {
    vec.pop_back();
}

void StackVector::alignTo16() {
    // do nothing if empty
    if (vec.back().empty()) {
        return;
    }

    // move the first element to be in a new 16 if it's not alone already
    if (vec.back().front().offset + (vec.back().front().singleSize * vec.back().front().amount) % 16 != 0) {
        vec.back().front().offset = (vec.back().front().offset / 16 - 1) * 16;
        vec.back().front().offset -= vec.back().front().singleSize * vec.back().front().amount;
    }

    // align every element with the last one
    for (uint64_t n = 1; n < vec.back().size(); n++) {
        vec.back()[n].offset = vec.back()[n - 1].offset - vec.back()[n].singleSize * vec.back()[n].amount;
        if (vec.back()[n].offset % vec.back()[n].singleSize != 0) {
            vec.back()[n].offset = (vec.back()[n].offset / vec.back()[n].singleSize - 1) * vec.back()[n].singleSize;
        }
    }
}

void StackVector::addArguments(const ParserFunction& function) {
    // check if it has any arguments
    if (function.arguments.empty()) {
        return;
    }

    // calculating normal offset
    int64_t size = 0;
    for (const auto& n: function.arguments) {
        StackVariable var;
        const auto& type = parserTypes[n.typeName];
        var.singleSize = type.size;
        var.amount = 1;
        var.isMutable = n.isMutable;
        var.name = n.name;
        var.isArgument = true;
        var.type = type.type;
        var.typeName = n.typeName;
        var.offset = size + var.singleSize * var.amount;
        if (var.offset % var.singleSize != 0) {
            var.offset = (var.offset / var.singleSize + 1) * var.singleSize;
        }
        size = var.offset;
        arguments.emplace_back(var);
    }

    // calculating the distance of the furthest variable
    if (size % 16 != 0) {
        size = (size / 16 + 1) * 16;
    }
    size += 16;

    // calculating variable offset from function base
    for (auto& n: arguments) {
        n.offset = size - n.offset;
    }

    // that's it!
}
