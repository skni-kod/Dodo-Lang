#ifndef DODO_LANG_MEMORY_STRUCTURE_HPP
#define DODO_LANG_MEMORY_STRUCTURE_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <stack>

// the memory structure simulates the cpu registers and stack, allowing for more optimized and generalized approach to memory management

namespace internal {
    struct Register {
        // names for given sizes
        std::vector <std::pair <uint64_t, std::string>> sizeNamePairs;
        // name of thing inside
        std::string content = "!";
        bool usedForIntegers = false;
        bool usedForFloats = false;
    };

    struct StackEntry {
        int64_t offset;
        uint64_t amount;
        uint64_t size;
        std::string content;
    };

    struct Variable {
        std::string identifier;
        std::string typeName;
    };
}

namespace x86_64 {
    // contains all used register names for convenience
    enum {
        a = 0, rax = 0, eax = 0, ax = 0, al = 0,
        b = 1, rbx = 1, ebx = 1, bx = 1, bl = 1,
        c = 2, rcx = 2, ecx = 2, cx = 2, cl = 2,
        d = 3, rdx = 3, edx = 3, dx = 3, dl = 3,
        rsi = 4, esi = 4, si = 4, sil = 4,
        rdi = 5, edi = 5, di = 5, dil = 5,
        rsp = 6, esp = 6, sp = 6, spl = 6,
        rbp = 7, ebp = 7, bp = 7, bpl = 7,
        r8 = 8, r8d = 8, r8w = 8, r8b = 8,
        r9 = 9, r9d = 9, r9w = 9, r9b = 9,
        r10 = 10, r10d = 10, r10w = 10, r10b = 10,
        r11 = 11, r11d = 11, r11w = 11, r11b = 11,
        r12 = 12, r12d = 12, r12w = 12, r12b = 12,
        r13 = 13, r13d = 13, r13w = 13, r13b = 13,
        r14 = 14, r14d = 14, r14w = 14, r14b = 14,
        r15 = 15, r15d = 15, r15w = 15, r15b = 15,
    };
    // TODO: add float registers
    // could add apx support here (32 registers)
}

struct MemoryStructure {
    int64_t maxOffset = 0;
    std::vector <internal::Register> registers;
    std::vector <internal::StackEntry> stack;
    std::vector <internal::StackEntry> arguments;
    std::stack  <std::vector <internal::Variable>> variableLevels;
    MemoryStructure() = default;
    void prepareX86_86();
    void cleanX86_86();
    void addArguments(const std::string& functionName);

};

inline MemoryStructure generatorMemory;

#endif //DODO_LANG_MEMORY_STRUCTURE_HPP