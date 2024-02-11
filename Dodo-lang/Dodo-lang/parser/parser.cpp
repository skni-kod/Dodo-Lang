#include "parser.hpp"
#include "generator.tpp"

uint64_t currentLine = 0;

Generator <const LexicalToken*> TokenRunGenerator(const std::vector<ProgramLine>& tokens) {
    for (auto& line : tokens) {
        currentLine = line.line_number;
        for (auto& token : line.line) {
            co_yield &token;
        }
    }
}

ASTTree RunParsing(const std::vector<ProgramLine>& tokens) {
    ASTTree tree;
    auto generator = TokenRunGenerator(tokens);
    while (generator) {
        std::cout << generator()->value << "\n";
    }


    return std::move(tree);
}
