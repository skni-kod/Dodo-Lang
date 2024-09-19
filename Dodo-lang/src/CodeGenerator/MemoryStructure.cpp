//
// Created by fifthzoner on 19/09/24.
//
#include "MemoryStructure.hpp"

void MemoryStructure::prepareX86_86() {
    registers.resize(16);
    for (auto& n : registers) {
        n.usedForIntegers = true;
        n.content = "!";
    }
    registers[0].sizeNamePairs =  {{1, "al"  }, {2, "ax"  }, {4, "eax" }, {8, "rax"}};
    registers[1].sizeNamePairs =  {{1, "bl"  }, {2, "bx"  }, {4, "ebx" }, {8, "rbx"}};
    registers[2].sizeNamePairs =  {{1, "cl"  }, {2, "cx"  }, {4, "ecx" }, {8, "rcx"}};
    registers[3].sizeNamePairs =  {{1, "dl"  }, {2, "dx"  }, {4, "edx" }, {8, "rdx"}};
    registers[4].sizeNamePairs =  {{1, "sil" }, {2, "si"  }, {4, "esi" }, {8, "rsi"}};
    registers[5].sizeNamePairs =  {{1, "dil" }, {2, "di"  }, {4, "edi" }, {8, "rdi"}};
    registers[6].sizeNamePairs =  {{1, "spl" }, {2, "sp"  }, {4, "esp" }, {8, "rsp"}};
    registers[7].sizeNamePairs =  {{1, "bpl" }, {2, "bp"  }, {4, "ebp" }, {8, "rbp"}};
    registers[8].sizeNamePairs =  {{1, "r8b" }, {2, "r8w" }, {4, "r8d" }, {8, "r8" }};
    registers[9].sizeNamePairs =  {{1, "r9b" }, {2, "r9w" }, {4, "r9d" }, {8, "r9" }};
    registers[10].sizeNamePairs = {{1, "r10b"}, {2, "r10w"}, {4, "r10d"}, {8, "r10"}};
    registers[11].sizeNamePairs = {{1, "r11b"}, {2, "r11w"}, {4, "r11d"}, {8, "r11"}};
    registers[12].sizeNamePairs = {{1, "r12b"}, {2, "r12w"}, {4, "r12d"}, {8, "r12"}};
    registers[13].sizeNamePairs = {{1, "r13b"}, {2, "r13w"}, {4, "r13d"}, {8, "r13"}};
    registers[14].sizeNamePairs = {{1, "r14b"}, {2, "r14w"}, {4, "r14d"}, {8, "r14"}};
    registers[15].sizeNamePairs = {{1, "r15b"}, {2, "r15w"}, {4, "r15d"}, {8, "r15"}};
}

void MemoryStructure::cleanX86_86() {
    for (auto& n : registers) {
        n.content = "!";
    }
}
