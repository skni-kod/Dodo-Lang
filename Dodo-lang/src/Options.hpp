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

    // when converting bytecode
    inline bool skipUnusedVariables = true;
    inline bool swapExpressionOperands = true;

    // in bytecode optimization function before conversion
    inline bool optimizeBytecode = false;
    inline bool replaceKnownValueVariables = true;
}

#endif //DODO_LANG_OPTIONS_HPP
