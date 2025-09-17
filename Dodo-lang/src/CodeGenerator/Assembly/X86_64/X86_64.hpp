#ifndef X86_64_HPP
#define X86_64_HPP
#include <Assembly.hpp>
#include <TypeObject.hpp>
#include <vector>

#include "InstructionPlanningInternal.hpp"

namespace x86_64 {

    // functions that need to be exposed

    // converts simple moves into assembly
    void AddConversionsToMove(MoveInfo& move, Context& context, std::vector<AsmInstruction>& moves, AsmOperand contentToSet, std::vector<AsmOperand>* forbiddenRegisters = nullptr, bool setContent = true);
    // returns a vector of registers and stack locations where arguments are to be placed
    // and amount of space on stack required for arguments
    // stack arguments are returned in the form used to reference arguments and not pass them (from 16 up)!
    std::pair<std::vector <AsmOperand>, int32_t> GetFunctionMethodArgumentLocations(ParserFunctionMethod& target);
    std::pair<std::vector <AsmOperand>, int32_t> GetFunctionMethodArgumentLocations(std::vector<Bytecode*>& target, Context& context);
    // the driver function for converting bytecode to assembly
    void ConvertBytecode(Context& context, ParserFunctionMethod* source, std::ofstream& out);
    // prints all instructions
    void PrintInstructions(std::vector <AsmInstruction>& instructions, std::ostream& out, int32_t maxOffset);
    // prints a single instruction
    void PrintInstruction(AsmInstruction& ins, std::ostream& out);
    void PrintInstruction(std::ostream& out, AsmInstruction ins);

    // not required to be exposed, printing etc.

    std::ostream& PrintRegisterName(uint16_t number, uint8_t size, std::ostream& out);
}

#endif //X86_64_HPP
