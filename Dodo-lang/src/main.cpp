#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "LexicalAnalysis.hpp"
#include <memory>

#include "Cli/Cli.hpp"
#include "Parser/Parser.hpp"
#include "CodeGenerator/GenerateCode.hpp"
#include "Lexer/Lexing.hpp"



int main(int argc, char* argv[]) {

    if (ApplyCommandLineArguments(argc, argv) == false) {
        std::cout << "Command line argument parsing failed. Compilation aborted!\n";
        return 1;
    }

    if (Options::helpOption) {
        std::cout <<
            "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n"
            "┃ 𝐃𝐨𝐝𝐨𝐋𝐚𝐧𝐠 𝐜𝐨𝐦𝐩𝐢𝐥𝐞𝐫                                    ┃\n"
            "┃ Build: (TBA)                                         ┃\n"
            "┃ By Szymon Jabłoński, Michał Kosiorski for SKNI \"KOD\" ┃\n"
            "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n"
            "\n"
            "Usage guide:\n"
            "{command name TBD} {file names} {options}\n"
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
    catch (__ParserException& e) {
        std::cout << "Parsing has failed. Compilation aborted!\n";
        return 1;
    }
    std::cout << "INFO L1: Parsing completed successfully!\nINFO L1: Generating assembly code:\n";


    try {
        GenerateCode();
    }
    catch (__CodeGeneratorException& e) {
        std::cout << "Code generation has failed. Compilation aborted!\n";
        return 1;
    }

    std::system("as -o build/out.o build/out.s");
    std::system("ld build/out.o -o build/out");

    return 0;
}