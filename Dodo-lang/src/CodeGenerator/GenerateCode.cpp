#include "GenerateCode.hpp"
#include "GenerateCodeInternal.hpp"
#include "CodeGenerator/Assembly/MemoryStructure.hpp"
#include "CodeGenerator/Bytecode/Bytecode.hpp"
#include "Assembly/X86_64/X86_64Assembly.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const char* __CodeGeneratorException::what() {
    return "Code generator has encountered unexpected input";
}

void CodeGeneratorError(std::string message) {
    if (currentlyGeneratedInstruction != nullptr) {
        std::cout << "ERROR! " << *currentlyGeneratedInstruction->sourceFile << " at line : "
                  << currentlyGeneratedInstruction->sourceLine + 1 << " : " << message << "\n";
    }
    else {
        std::cout << "ERROR! " << message << "\n";
    }

    throw __CodeGeneratorException();
}


void GenerateCode() {
    doneParsing = true;
    std::ofstream out;
    if (!fs::is_directory("build")) {
        fs::create_directory("build");
    }
    out.open("build/out.s");
    if (!out.is_open()) {
        CodeGeneratorError("Failed to create/open output assembly file");
    }

    // Code generation is done on a per-function basis to reduce peak memory usage, and there's no downsides I'm aware of

    // prepare structures here
    // TODO: make this a dedicated function probably
    if (Options::targetArchitecture == "X86_64") {
        generatorMemory.prepareX86_86();
        Options::commentPrefix = "# ";
        Options::jumpLabelPrefix = ".LC";
        out << Options::commentPrefix << "Generated by Dodo-Lang compiler by SKNI \"KOD\"\n";
        out << Options::commentPrefix << "Target architecture: X86_64\n";
        if (Options::targetSystem == "LINUX") {
            out << Options::commentPrefix << "Target system: LINUX\n";
        }
        else if (Options::targetSystem == "WINDOWS") {
            CodeGeneratorError("Windows target is not supported!");
        }
        else {
            CodeGeneratorError("Invalid target system!");
        }
        out << ".data\n"
            << ".text\n"
            << ".global _start\n";
    }
    else if (Options::targetArchitecture == "X86_32") {
        CodeGeneratorError("X86-32 not yet supported!");
    }
        // might change arm names to versions when support comes
    else if (Options::targetArchitecture == "ARM32") {
        CodeGeneratorError("ARM32 not yet supported!");
    }
    else if (Options::targetArchitecture == "ARM64") {
        CodeGeneratorError("ARM64 not yet supported!");
    }
    else {
        CodeGeneratorError("Invalid target architecture!");
    }

    // put beginning things here, like message declaration

    // Functions:
    for (auto& current: parserFunctions.map) {
        // cleaning up after possible previous thing
        bytecodes.clear();
        finalInstructions.clear();
        if (Options::targetArchitecture == "X86_64") {
            generatorMemory.cleanX86_86();
            out << "\n"
                << current.second.name << ":" << "\n";
            PrintWithSpaces("pushq", out);
            out << "%rbp\n";
            PrintWithSpaces("movq", out);
            out << "%rsp, %rbp\n";
        }

        // first off the instructions need to be changed into raw operations universal for platforms
        GenerateFunctionStepOne(current.second);

        // general optimizations will take place here
        if (Optimizations::optimizeBytecode) {
            OptimizeBytecode();
        }

        // next convert them into platform specific code
        GenerateFunctionStepTwo(current.second);
        // platform specific optimizations will be done here

        // at the end use the calculated stack offset and put the thing into code
        for (auto& finalInstruction: finalInstructions) {
            finalInstruction.outputX86_64(out);
        }

        // end function
        if (Options::targetArchitecture == "X86_64" and not finalInstructions.empty() and finalInstructions.back().type != x86_64::ret) {
            PrintWithSpaces("popq", out);
            out << "%rbp\n"
                << "ret\n";
        }
    }
    // add end code here along with the runner code fragment
    if (Options::targetArchitecture == "X86_64") {
        out << "\n"
            << "_start:\n";
        PrintWithSpaces("call", out);
        out << "main\n";
        PrintWithSpaces("movq", out);
        out << "%rax, %rdi\n";
        PrintWithSpaces("movq", out);
        out << "$60, %rax\n"
            << "syscall\n";
    }

    std::cout << "INFO L1: Assembly output generation complete!\n";
}
