#ifndef ASSIGN_MEMORY_HPP
#define ASSIGN_MEMORY_HPP

#include "Assembly.hpp"

// assumes that lifetimes have already been calculated
void CalculateMemoryAssignments(Processor& proc, BytecodeContext& context);

#endif //ASSIGN_MEMORY_HPP