#ifndef DODO_LANG_X86_64_ASSEMBLY_HPP
#define DODO_LANG_X86_64_ASSEMBLY_HPP

#include <cstdint>
#include "Bytecode/Bytecode.hpp"
#include "../MemoryStructure.hpp"

namespace x86_64 {
    void ConvertBytecode(BytecodeOld& bytecode, uint64_t index);
    
    void AddGlobalVariables(std::ofstream& out);

    enum {
        // (mov+p1) move : op1 = op2, reg/mem + reg, reg + reg/mem, reg/mem + imm
        OLD_mov,
        // (movz+p1+p2) move with sign extension: op1 = op2, reg + reg/mem
        OLD_movsx,
        // (movz+p1+p2) move with zero extension: op1 = op2, reg + reg/mem
        OLD_movzx,
        // (mul + p1) unsigned multiply content of register a by op2: reg0 = reg0 * reg/mem
        OLD_mul,
        //
        OLD_imul,
        // (mul + p1) unsigned divide content of register a by op2: reg0 = reg0 * reg/mem, set 0 to reg3
        OLD_div,
        //
        OLD_idiv,
        //
        OLD_cxtx,
        //
        OLD_call,
        //
        OLD_ret,
        //
        OLD_push,
        //
        OLD_pop,
        // (add + p1) add signed/unsigned integer op1 = op1 + op2 reg + reg/mem, reg/mem + reg, reg/mem + imm
        OLD_add,
        // (sub + p1) subtract signed/unsigned integer op1 = op1 + op2 reg + reg/mem, reg/mem + reg, reg/mem + imm
        OLD_sub,
        //
        OLD_syscall,
        // jmp, label in op1
        OLD_jmp,
        // label in op1
        OLD_jb,
        // label in op1
        OLD_jbe,
        // label in op1
        OLD_jg,
        // label in op1
        OLD_jge,
        // label in op1
        OLD_je,
        // label in op1
        OLD_jne,
        //
        OLD_cmp,
        // (lea + p1) load effective address of variable in memory op1 = &op2: reg + mem
        OLD_lea,
        // (.LC + op1.number + ':')
        OLD_jumpLabel,
        OLD_returnPoint,
        OLD_leave
    };
}

#endif //DODO_LANG_ASSEMBLY_HPP
