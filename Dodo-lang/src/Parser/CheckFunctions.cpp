#include "Parser.hpp"
#include "ParserVariables.hpp"
#include "Assembly/MemoryStructure.hpp"
#include "Options.hpp"


inline void AddX86_64ArgumentPlaces(std::vector <FunctionArgument>& args) {
    std::array <uint8_t, 6> integerRegisters = {x86_64::rdi, x86_64::rsi, x86_64::rdx, x86_64::rcx, x86_64::r8, x86_64::r9};
    //std::array <uint8_t, 8> floatRegisters = {x86_64::xmm0, x86_64::xmm1, x86_64::xmm2, x86_64::xmm3, x86_64::xmm4, x86_64::xmm5, x86_64::xmm6, x86_64::xmm7};
    // for now only integer support
    for (uint64_t n = 0; n < args.size(); n++) {
        // first find out the argument's type, most easily by using the stored ones
        auto type = parserTypes[args[n].typeName].type;
        if (type == ParserType::floatingPoint) {
            ParserError("Unimplemented: Floating point arguments!");
        }

        // now that we are sure this is an integer we just assign it a register or stack location (TBA)
        if (n == integerRegisters.size()) {
            // stack would require things like positive offsets and I don't want to deal with it for now
            ParserError("Unimplemented: Arguments on stack!");
        }

        // just add the register
        args[n].locationType = Operand_Old::reg;
        args[n].locationValue = integerRegisters[n];
    }
}

void PrepareFunctionArguments() {
    if (Options::targetArchitecture == Options::TargetArchitecture::x86_64) {
        for (auto& n : parserFunctions.map) {
            AddX86_64ArgumentPlaces(n.second.arguments);
        }
    }
    else {
        ParserError("Unimplemented: Non x86-64 function argument preparation");
    }
}