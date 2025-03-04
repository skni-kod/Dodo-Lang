#include "../GenerateCodeInternal.hpp"
#include "Bytecode/Bytecode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include "LinearAnalysis.hpp"
#include "TheGenerator.hpp"
#include "Options.hpp"

void GenerateFunctionStepTwo(const ParserFunction& function) {
    // this step converts the bytecode into platform specific instructions

    RunLinearAnalysis();

    // for now only x86-64 support
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        for (uint64_t n = 0; n < bytecodes.size(); n++) {
            currentBytecodeIndex = n;
            x86_64::ConvertBytecode(bytecodes[n], n);
            UpdateVariables();
        }
    }

    // now do the register and stack stuff for function calls;
    AddFunctionHeaders();

    if (Options::informationLevel == Options::InformationLevel::full) {
        std::cout << "INFO L3: Instruction generation for this function succeeded using: " << finalInstructions.size() << " instructions before final additions\n";
    }


}
