#include <iostream>
#include <fstream>
#include <vector>

#include "ErrorHandling.hpp"
#include "Misc/Increment.hpp"
#include "Cli/Cli.hpp"
#include "Parser/Parser.hpp"
#include "CodeGenerator/GenerateCode.hpp"
#include "Lexer/Lexing.hpp"


int main(int argc, char* argv[]) {

    std::cout <<
            "𝐃𝐨𝐝𝐨𝐋𝐚𝐧𝐠 𝐜𝐨𝐦𝐩𝐢𝐥𝐞𝐫\n"
            "Version: " << incrementedVersionValue << "\n"
#if defined(__linux__) or defined(__gnu_linux__)
            "Platform: Linux "
#else
            "Platform: Not linux "
#endif
#if defined(__x86_64__)
            "x86-64 "
#elif defined(i386) or defined(__i386__) or defined(__i386) or defined(_M_IX86)
            "x86-32 "
#else
            "unknown architecture "
#endif
#if defined(_MSC_VER)
            "MSVC\n";
#elif defined(__GNUC__)
            "Gcc\n";
#elif defined(__clang__)
            "Clang\n";
#elif defined(__MINGW32) or defined(__MINGW64__)
            "MinGW\n";
#endif


    if (ApplyCommandLineArguments(argc, argv) == false) {
        std::cout << "Command line argument parsing failed. Compilation aborted!\n";
        return 1;
    }


    if (Options::helpOption) {

            std::cout <<
            "By Szymon Jabłoński, started by Michał Kosiorski, developed as a SKNI \"KOD\" project at Rzeszów University of Technology;\n"
            "\n"
            "Usage:\n"
            "dodoc <file names> <options>\n"
            "🀱🀲🀺🁂🁊🁒🁚🁡🁠🁘🁐🁈🁀🀸🀱\n"
            "Options:\n"
            "All flags can be written in lowercase or uppercase;\n"
            "\n"
            "-o                - sets executable name, pass name as next argument;\n"
            "-h, -help         - displays this screen;\n"
            "-r                - release - enables all optimizations (by default only some are enabled);\n"
            "-d                - debug - disabled all optimizations;\n"
            "-l{1-3}           - sets information level, with 1 being the lowest and 3 being 𝐯𝐞𝐫𝐲 verbose;\n"
            "-target=          - sets target system, default is \"LINUX\";\n"
            "-platform=        - sets target platform, default is \"x86_64\";\n"
            "-stdlibdirectory= - sets standard library directory, default is \"/usr/include/DodoLang\" for linux and none for others;\n"
            "-import=          - adds a directory to file source search list;\n"
            "-extensions=      - sets extension level supported by target, for x86-64 values are: x86-64_v1 (default), x86-64_v2, x86-64_v3, x86-64_v4;\n"
            "🀱🀲🀺🁂🁊🁒🁚🁡🁠🁘🁐🁈🁀🀸🀱\n"
            "Architecture versions are defined per the 2020 x86-64_v1-v4 standard agreed upon by Intel, AMD, RedHat and SUSE:\n"
            "- x86-64_v1 - the baseline, SSE and SSE2 are used extensively for floating point operations;\n"
            "- x86-64_v2 - as of now this target changes little, it introduces SSE3, SSE4_1 and SSE4_2, instructions provided by which are not used;\n"
            "- x86-64_v3 - provides AVX and AVX2, their instructions can be used to improve floating point performance;\n"
            "- x86-64_v4 - introduces AVX512, not used as of now due to lesser support and less use cases.\n"
            ;
        return 0;
    }

    // new lexing here
    std::vector<LexerFile> lexed;
    try {
        SetCompilationStage(CompilationStage::lexing);
        lexed = std::move(RunLexer());
    }
    catch (LexerException& e) {
        std::cout << "Lexing has failed. Compilation aborted!\n";
        return 1;
    }
    catch (CompilerException& e) {
        std::cout << e.what();
        std::cout << "Lexing has failed. Compilation aborted!\n";
        return 1;
    }

    // TODO: macro system here

    std::cout << "INFO L1: Lexing done!\nINFO L1: Parsing:\n";
    try {
        SetCompilationStage(CompilationStage::parsing);
        RunParsing(lexed);
    }
    catch (ParserException& e) {
        std::cout << "Parsing has failed. Compilation aborted!\n";
        return 1;
    }
    catch (CompilerException& e) {
        std::cout << e.what();
        std::cout << "Parsing has failed. Compilation aborted!\n";
        return 1;
    }
    std::cout << "INFO L1: Parsing completed successfully!\nINFO L1: Generating code:\n";


    try {
        GenerateCode();
    }
    catch (__CodeGeneratorException& e) {
        std::cout << "Code generation has failed. Compilation aborted!\n";
        return 1;
    }
    catch (CompilerException& e) {
        std::cout << e.what();
        std::cout << "Code generation has failed. Compilation aborted!\n";
        return 1;
    }

    //std::system("as -o build/out.o build/out.s");
    //std::system("ld build/out.o -o build/out");

    return 0;
}