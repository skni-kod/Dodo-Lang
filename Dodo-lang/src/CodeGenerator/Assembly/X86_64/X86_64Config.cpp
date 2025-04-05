#include "X86_64Config.hpp"
#include "X86_64Enums.hpp"

namespace x86_64 {

void PrepareProcessor(Processor& proc) {

    // first off things that are always there
    // these enums must (or at least probably should) be in order
    proc.registers = {
        {x86_64::RAX, true , true , true , true , false, false, false},
        {x86_64::RBX, true , true , true , true , false, false, false},
        {x86_64::RCX, true , true , true , true , false, false, false},
        {x86_64::RDX, true , true , true , true , false, false, false},
        {x86_64::RSI, true , true , true , true , false, false, false},
        {x86_64::RDI, true , true , true , true , false, false, false},
        {x86_64::RSP, true , true , true , true , false, false, false},
        {x86_64::RBP, true , true , true , true , false, false, false},
        {x86_64::R8 , true , true , true , true , false, false, false},
        {x86_64::R9 , true , true , true , true , false, false, false},
        {x86_64::R10, true , true , true , true , false, false, false},
        {x86_64::R11, true , true , true , true , false, false, false},
        {x86_64::R12, true , true , true , true , false, false, false},
        {x86_64::R13, true , true , true , true , false, false, false},
        {x86_64::R14, true , true , true , true , false, false, false},
        {x86_64::R15, true , true , true , true , false, false, false},
        {x86_64::RIP, false, true , true , true , false, false, false},
        {x86_64::RIP, false, true , true , true , false, false, false},
        // SSE instructions only support 32 bit floats
    };
    // TODO: read into intel instruction reference and think about this A LOT

}

}