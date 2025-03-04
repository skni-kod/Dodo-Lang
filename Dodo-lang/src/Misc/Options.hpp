#ifndef DODO_LANG_OPTIONS_HPP
#define DODO_LANG_OPTIONS_HPP

#include <cstdint>
#include <string>
#include <queue>
#include <filesystem>

// DEVELOPMENT DEFINITIONS

// due to the fact that enums use 4 bytes many of the structures will grow with this enabled
// it's very useful for debugging things and checking for correct enums though
#define ENUM_VARIABLES



namespace fs = std::filesystem;

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
    enum TargetArchitecture {
        x86_64, x86_32
    };
    inline uint8_t informationLevel = InformationLevel::minimal;
    inline uint8_t addressSize = 8;
    inline uint8_t targetArchitecture = TargetArchitecture::x86_64;
    inline std::queue <fs::path> inputFiles;
#if defined(__linux__) or defined(__unix__)
    inline fs::path stdlibDirectory = "/usr/include/DodoLang/";
#else
    // if you're on windows or other non-unix you need to set the dir yourself
    inline fs::path stdlibDirectory = "";
#endif
    inline std::vector <fs::path> importDirectories;
    inline std::string targetSystem = "LINUX";
    inline std::string outputName = "out";
    inline std::string commentPrefix = "# ";
    inline std::string jumpLabelPrefix = ".LC";
    enum AssemblyFlavor {
        GAS, NASM
    };
    inline uint8_t assemblyFlavor = AssemblyFlavor::GAS;
    inline uint8_t instructionSpace = 8;
    inline uint8_t functionIndentation = 4;
    inline bool helpOption = false;
}

namespace Optimizations {
    // while outputting assembly
    inline bool skipUselessMoves = true;
    inline bool mergeThreeOperandInstruction = true;
    inline bool skipDoubleJumps = false;

    // when converting bytecode
    inline bool skipUnusedVariables = false;
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
