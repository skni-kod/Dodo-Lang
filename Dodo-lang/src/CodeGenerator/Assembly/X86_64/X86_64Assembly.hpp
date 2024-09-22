#ifndef DODO_LANG_X86_64_ASSEMBLY_HPP
#define DODO_LANG_X86_64_ASSEMBLY_HPP

#include <cstdint>
#include "Bytecode/Bytecode.hpp"
#include "../MemoryStructure.hpp"

namespace x86_64 {
    void ConvertBytecode(const Bytecode& bytecode);
    void EmitAssemblyFromCode(const Instruction& instruction, std::ofstream& out);
    enum {
        // op1 = op2, reg/mem + reg, reg + reg/mem, reg/mem + imm
        mov,
        //
        movs,
        //
        movz,
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
        //
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
