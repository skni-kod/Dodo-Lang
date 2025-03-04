#include "Bytecode.hpp"
#include "Options.hpp"

void OptimizeBytecode() {

    // if a variable is assigned a known value at the beginning it can most likely be replaced with a simple value until modified
    if (Optimizations::replaceKnownValueVariables) {
        for (uint64_t n = 0; n < bytecodes.size(); n++) {
            if (bytecodes[n].code == Bytecode::declare and bytecodes[n].source.starts_with("$")) {
                // TODO: when adding pointers there must be a check if this value is not pointed to!
                // TODO: also figure this out
                std::string searched = bytecodes[n].target.substr(2, bytecodes[n].target.size() - 2);
                for (uint64_t m = n + 1; m < bytecodes.size(); m++) {
                    switch (bytecodes[m].code) {
                        case Bytecode::add:
                            if (bytecodes[m].target.ends_with(searched)) {
                                bytecodes[m].source = bytecodes[m].target;
                                bytecodes[m].target = bytecodes[n].source;
                            }
                    }

                }
                bytecodes.erase(bytecodes.begin() + int64_t(n));
            }
        }
    }

    if (Options::informationLevel == Options::InformationLevel::full) {
        std::cout << "INFO L3: Optimized bytecodes for this function:\n";
        uint64_t k = 1;
        for (auto& n: bytecodes) {
            std::cout << "INFO L3: ";
            if (n.code == Bytecode::popLevel) {
                k--;
            }
            for (uint64_t m = 0; m < k; m++) {

                std::cout << "\t";
            }
            if (n.code == Bytecode::pushLevel) {
                k++;
            }
            std::cout << n;
        }
        std::cout << "INFO L3: Optimized bytecode amount for this function: " << bytecodes.size() << "\n";
    }
}
