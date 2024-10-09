#ifndef DODO_LANG_OPTIONS_HPP
#define DODO_LANG_OPTIONS_HPP

#include <cstdint>

namespace Options {
    // default values inside
    enum InformationLevel {
        // only general stages, time
        minimal,
        // stages, amounts of files, types, time
        general,
        // full information about the compilation process
        full
    };
    inline uint8_t informationLevel = InformationLevel::full;
    inline std::string targetArchitecture = "X86_64";
    inline std::string targetSystem = "LINUX";
    inline std::string commentPrefix = "# ";
    inline std::string jumpLabelPrefix = ".LC";
    enum AssemblyFlavor {
        GNU_AS, NASM
    };
    inline uint8_t assemblyFlavor = AssemblyFlavor::GNU_AS;
    inline uint8_t spaceOnLeft = 8;
}

namespace Optimizations {
    // while outputting assembly
    inline bool skipUselessMoves = true;
    inline bool mergeThreeOperandInstruction = true;
    inline bool skipDoubleJumps = false;

    // when converting bytecode
    inline bool skipUnusedVariables = true;
    inline bool swapExpressionOperands = true;
    inline bool checkPotentialUselessStores = true;

    // in bytecode optimization function before conversion
    inline bool optimizeBytecode = false;
    inline bool replaceKnownValueVariables = false;

    // when generating bytecode

    // mixed
    // groups main variable instances to always be in the same register during the entire lifetime, making them interchangeable
    // this allows for much better use of registers in conditional statements compared to keeping a copy od the one from before
    // actually now it must be on for the compiler to work
    inline bool groupVariableInstances = true;
}

#endif //DODO_LANG_OPTIONS_HPP
