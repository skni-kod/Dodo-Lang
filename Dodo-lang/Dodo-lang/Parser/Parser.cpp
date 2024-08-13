#include "Parser.hpp"
#include "Generator.tpp"

#include "ParserVariables.hpp"

#include "SyntaxAnalysis/SyntaxAnalysis.hpp"

uint64_t currentLine = 0;
const std::string* currentFile = nullptr;

const char* ParserException::what() {
    return "Parser has encountered unexpected input";
}

void ParserError(const std::string message) {
    if (currentFile == nullptr) {
        std::cout << "ERROR! Outside file!\n";
    }
    else {
        std::cout << "ERROR! " << *currentFile << " at line : " << currentLine + 1 << " : " << message << "\n";
    }
    throw ParserException();
}

Generator <const LexicalToken*> TokenRunGenerator(const std::vector<ProgramPage>& tokens) {
    currentLine = 0;
    currentFile = nullptr;
    for (const auto& file : tokens) {
        currentFile = &file.file_name;
        for (const auto& line : file.page) {
            currentLine = line.line_number;
            for (const auto& token : line.line) {
                co_yield &token;
            }
        }
    }
}

ASTTree RunParsing(const std::vector<ProgramPage>& tokens) {
    ASTTree tree;

    // Step 1: syntax analysis and creating initial structures
    {
        auto generator = TokenRunGenerator(tokens);
        if (RunSyntaxAnalysis(generator)) {
            ParserError("Syntax analysis errors occurred!");
        }
    }

    // Step 2: checking basic types (might not be required)

    if (flags::informationLevel > flags::informationLevel::minimal) {
        std::cout << "INFO L2: Finished type parsing with : " << parserTypes.size() << " type definition(s)\n";
    }

    // Step 3: checking complex types

    // ...

    // Step 4: checking functions and if main exists

    if (flags::informationLevel > flags::informationLevel::minimal) {
        std::cout << "INFO L2: Finished type parsing with : " << parserFunctions.size() << " function definition(s)\n";
    }

    // Step 5: creating tree from ready structures

    // ...

    return std::move(tree);
}