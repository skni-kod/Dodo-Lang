#ifndef X86_64_HPP
#define X86_64_HPP
#include <Assembly.hpp>
#include <vector>

#include "InstructionPlanningInternal.hpp"

namespace x86_64 {
    std::vector <MoveInfo> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc);

    void PrintInstructions(std::vector <AsmInstruction>& instructions, std::ofstream& out);
}

#endif //X86_64_HPP
