#ifndef X86_64_HPP
#define X86_64_HPP
#include <Assembly.hpp>
#include <vector>

#include "InstructionPlanningInternal.hpp"

namespace x86_64 {
    std::vector <AsmInstruction> AddConvertionsToMove(MoveInfo& move, BytecodeContext& context, Processor& proc);
    // returns a vector of registers and stack locations where arguments are to be placed
    // and amount of space on stack required for arguments
    // stack arguments are returned in the form used to reference arguments and not pass them (from 16 up)!
    std::pair<std::vector <AsmOperand>, int32_t> GetFunctionMethodArgumentLocations(ParserFunctionMethod& target);

    void PrintInstructions(std::vector <AsmInstruction>& instructions, std::ostream& out);
    void PrintInstruction(AsmInstruction& ins, std::ostream& out);
    std::ostream& PrintRegisterName(uint16_t number, uint8_t size, std::ostream& out);
}

#endif //X86_64_HPP
