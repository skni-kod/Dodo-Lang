#ifndef DODO_LANG_X86_64_ASSEMBLY_HPP
#define DODO_LANG_X86_64_ASSEMBLY_HPP

#include <cstdint>
#include "Bytecode/Bytecode.hpp"
#include "../MemoryStructure.hpp"

namespace x86_64 {
    void ConvertBytecode(Bytecode& bytecode, uint64_t index);

    enum {
        // (mov+p1) move : op1 = op2, reg/mem + reg, reg + reg/mem, reg/mem + imm
        mov,
        // (movz+p1+p2) move with sign extension: op1 = op2, reg + reg/mem
        movsx,
        // (movz+p1+p2) move with zero extension: op1 = op2, reg + reg/mem
        movzx,
        // (mul + p1) unsigned multiply content of register a by op2: reg0 = reg0 * reg/mem
        mul,
        //
        imul,
        // (mul + p1) unsigned divide content of register a by op2: reg0 = reg0 * reg/mem, set 0 to reg3
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
        // (sub + p1) subtract signed/unsigned integer op1 = op1 + op2 reg + reg/mem, reg/mem + reg, reg/mem + imm
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
