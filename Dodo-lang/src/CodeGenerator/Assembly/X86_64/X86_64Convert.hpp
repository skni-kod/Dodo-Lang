#ifndef X86_64CONVERT_HPP
#define X86_64CONVERT_HPP
#include <Assembly.hpp>
#include <Bytecode.hpp>

namespace x86_64 {
    // converts bytecode to actual assembly within a processor
    void ConvertBytecode(BytecodeContext& context, Processor& processor, std::ofstream& out);
}

#endif
