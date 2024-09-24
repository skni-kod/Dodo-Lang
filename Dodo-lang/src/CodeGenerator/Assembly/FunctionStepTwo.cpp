#include "../GenerateCodeInternal.hpp"
#include "Bytecode/Bytecode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "LinearAnalysis.hpp"

void GenerateFunctionStepTwo(const ParserFunction& function) {
    // this step converts the bytecode into platform specific instructions

    RunLinearAnalysis();

    // for now only x86-64 support
    if (options::targetArchitecture == "X86_64") {
        for (uint64_t n = 0; n < bytecodes.size(); n++) {
            x86_64::ConvertBytecode(bytecodes[n]);
        }
    }
}