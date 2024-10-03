#ifndef DODO_LANG_X86_64_ASSEMBLY_HPP
#define DODO_LANG_X86_64_ASSEMBLY_HPP

#include <cstdint>
#include "Bytecode/Bytecode.hpp"
#include "../MemoryStructure.hpp"

namespace x86_64 {
    void ConvertBytecode(const Bytecode& bytecode);

    enum {
        // (mov+p1) op1 = op2, reg/mem + reg, reg + reg/mem, reg/mem + imm
        mov,
        // (movz+p1+p2) move with sign extension op1 = op2, reg + reg/mem
        movsx,
        // (movz+p1+p2) move with zero extension op1 = op2, reg + reg/mem
        movzx,
        //
        mul,
        //
        imul,
        //
        div,
        //
        idiv,
        //
        cxtx,
        //
        call,
        //
        ret,
        //
        push,
        //
        pop,
        // (add + p1) add signed/unsigned integer op1 = op1 + op2 reg + reg/mem, reg/mem + reg, reg/mem + imm
        add,
        //
        sub,
        //
        syscall,
        //
        jump,
        //
        jc,
        //
        cmp
    };
}

#endif //DODO_LANG_ASSEMBLY_HPP
