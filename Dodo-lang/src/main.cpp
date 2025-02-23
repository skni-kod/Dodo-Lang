#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "LexicalAnalysis.hpp"
#include <memory>
#include "Parser/Parser.hpp"
#include "CodeGenerator/GenerateCode.hpp"
#include "Lexer/Lexing.hpp"

int main(int argc, char* argv[]) {

    // TODO: argument parsing here
    const std::string fileName = "dodotest.dodo";

    // new lexing here
    std::vector<LexerFile> lexed;
    try {
        lexed = std::move(RunLexer(fileName));
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