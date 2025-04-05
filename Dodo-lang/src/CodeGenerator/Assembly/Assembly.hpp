#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "Options.hpp"

// This is the scary file when all the terrifying assembly stuff roots from

// represents a register in cpu
struct Register {
    // unknown if it will be needed
    uint16_t number = 0;

    // supported operand sizes
    // TODO: how to handle SIMD registers?
    bool size8   : 1 = false;
    bool size16  : 1 = false;
    bool size32  : 1 = false;
    bool size64  : 1 = false;
    bool size128 : 1 = false;
    bool size256 : 1 = false;
    bool size512 : 1 = false;
};

struct Processor {
    std::vector <Register> registers;
};

struct AsmInstruction {

};


#endif //ASSEMBLY_HPP
