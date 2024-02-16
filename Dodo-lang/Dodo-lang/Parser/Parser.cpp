#include "Parser.hpp"
#include "Generator.tpp"
#include "ParserVariables.hpp"
#include "ParserStages/TypeParser.hpp"
#include "ParserStages/ObjectDeclarationParser.hpp"

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

    // Pass 1 - parsing types
    {
        auto generator = TokenRunGenerator(tokens);
        ParseTypes(generator);
        if (flags::informationLevel > flags::informationLevel::minimal) {
            std::cout << "INFO L2: Finished type parsing with : " << parserTypes.size() << " type definition(s)\n";
        }
    }

    // Pass 2 - getting object declarations
    {
        auto generator = TokenRunGenerator(tokens);
        ParseObjectDeclarations(generator);

    }

    // Pass 3 - parsing all objects
    {
        auto generator = TokenRunGenerator(tokens);

        if (flags::informationLevel > flags::informationLevel::minimal) {
            std::cout << "INFO L2: Finished object parsing with : " << parserObjects.size() << " object definition(s)\n";
        }
    }


    return std::move(tree);
}
