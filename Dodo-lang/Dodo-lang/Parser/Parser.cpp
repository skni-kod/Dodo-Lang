#include "Parser.hpp"
#include "Generator.tpp"
#include "ParserVariables.hpp"
#include "ParserStages/TypeParser.hpp"

uint64_t currentLine = 0;

const char* ParserException::what() {
    return "Parser has encountered unexpected input";
}

void ParserError(const std::string message) {
    std::cout << "ERROR! At line : " << currentLine << " : " << message << "\n";
    throw ParserException();
}

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

    // Pass 1
    {
        auto generator = TokenRunGenerator(tokens);
        ParseTypes(generator);
        if (flags::informationLevel > flags::informationLevel::minimal) {
            std::cout << "Finished type parsing with : " << parserTypes.size() << " type definition(s)\n";
        }
    }


    return std::move(tree);
}
