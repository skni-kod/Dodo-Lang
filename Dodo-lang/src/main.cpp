#include <iostream>
#include <fstream>
#include <vector>

#include "Misc/Increment.hpp"
#include "Cli/Cli.hpp"
#include "Parser/Parser.hpp"
#include "CodeGenerator/GenerateCode.hpp"
#include "Lexer/Lexing.hpp"


int main(int argc, char* argv[]) {

    std::cout <<
            "𝐃𝐨𝐝𝐨𝐋𝐚𝐧𝐠 𝐜𝐨𝐦𝐩𝐢𝐥𝐞𝐫\n"
            "Version: " << INCREMENTED_VALUE << "\n"
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
            "By Szymon Jabłoński, Michał Kosiorski for SKNI \"KOD\"\n"
            "\n"
            "Usage:\n"
            "dodoc {file names} {options}\n"
            "\n"
            "Options:\n"
            "-o                - sets executable name, pass name as next argument\n"
            "-h, -help         - displays this screen\n"
            "-d                - disables generator optimizations\n"
            "-l{1-3}           - sets information level, with 1 being the lowest and 3 being 𝐯𝐞𝐫𝐲 verbose\n"
            "-target=          - sets target system, default is \"LINUX\"\n"
            "-platform=        - sets target platform, default is \"x86_64\"\n"
            "-stdlibDirectory= - sets standard library directory, default is \"/usr/include/DodoLang\" for linux and none for others\n"
            "-import=          - adds a directory to file source search list\n";
        return 0;
    }

    // new lexing here
    std::vector<LexerFile> lexed;
    try {
        lexed = std::move(RunLexer());
    }
    catch (LexerException& e) {
        std::cout << "Lexing has failed. Compilation aborted!\n";
        return 1;
    }

    // TODO: macro system here

    std::cout << "INFO L1: Lexing done!\nINFO L1: Parsing:\n";
    try {
        RunParsing(lexed);
    }
    catch (ParserException& e) {
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

    //std::system("as -o build/out.o build/out.s");
    //std::system("ld build/out.o -o build/out");

    return 0;
}