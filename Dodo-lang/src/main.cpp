#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "LexicalAnalysis.hpp"
#include <memory>
#include "Parser/Parser.hpp"
#include "CodeGenerator/GenerateCode.hpp"
#include "Lexer/Lexing.hpp"

// to fix any simple issues without delving into the lexer itself
void FixLexerOutput(std::vector<ProgramPage>& tokens) {
    for (auto& m: tokens) {
        for (auto& m2: m.page) {
            for (auto& n: m2.line) {
                if (n.value == ",") {
                    n.type = LexicalToken::Type::comma;
                }
                else if (n.value == ";") {
                    n.type = LexicalToken::Type::expressionEnd;
                }
                else if (n.value == "{") {
                    n.type = LexicalToken::Type::blockBegin;
                }
                else if (n.value == "}") {
                    n.type = LexicalToken::Type::blockEnd;
                }
                else if (n.type == LexicalToken::Type::literal and n.value.size() > 1 and n.value.front() == '\"' and n.value.back() == '\"') {
                    // it's a string, since the lexer ands the line after it an expression end needs to be added
                    if (&n == &m2.line.back()) {
                        m2.add_token(LexicalToken::Type::expressionEnd, ";");
                    }
                    else {
                        // add a comma after this
                        for (uint64_t k = 0; k < m2.line.size(); k++) {
                            if (&m2.line[k] == &n) {
                                m2.line.insert(m2.line.begin() + k + 1, {LexicalToken::Type::comma, ","});
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {

    // TODO: take from arguments
    const std::string fileName = "dodotest.dodo";

    // DO NOT USE THIS IF YOU VALUE YOUR SANITY
    std::fstream plik;
    plik.open(fileName, std::fstream::in);
    std::unique_ptr<list_of_tokens> lt(list_of_tokens::get_instance());
    lt->lexical_analize(fileName);
    FixLexerOutput(lt->f_token_list);
    lt->list_of_tokens_print();
    plik.close();
    // END OF DO NOT USE

    // new lexing here
    std::vector<LexerFile> lexed;
    try {
        lexed = std::move(RunLexer(fileName));
    }
    catch (LexerException& e) {
        std::cout << "Lexing has failed. Compilation aborted!\n";
        return 1;
    }

    // later macro system here


    std::cout << "INFO L1: Lexing done!\nINFO L1: Parsing:\n";
    try {
        RunParsing(lt->f_token_list);
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