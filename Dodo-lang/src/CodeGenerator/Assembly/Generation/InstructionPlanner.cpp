#include "Assembly.hpp"

// This file is responsible for resolving instruction definition variants,
// planning moves, conversions and passing off the tasks to platform dependent functions
// this file is to be platform-agnostic to make introducing other targets easier
void ExecuteInstruction(BytecodeContext& context, Processor& processor, AsmInstructionInfo& instruction, std::vector<AsmInstruction>& instructions, uint32_t index) {

}