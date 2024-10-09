#include "Parser.hpp"
#include "Generator.tpp"
#include "ParserVariables.hpp"
#include "SyntaxAnalysis/SyntaxAnalysis.hpp"
#include "GenerateCode.hpp"


uint64_t currentLine = 0;
const std::string* currentFile = nullptr;

const std::string* GetCurrentFile() {
    return currentFile;
}

uint64_t GetCurrentLine() {
    return currentLine;
}

const char* __ParserException::what() {
    return "Parser has encountered unexpected input";
}

void ParserError(const std::string& message) {
    if (currentFile == nullptr) {
        std::cout << "ERROR! Outside file!\n";
    }
    else {
        std::cout << "ERROR! " << *currentFile << " at line : " << currentLine + 1 << " : " << message << "\n";
    }
    if (not doneParsing) {
        throw __ParserException();
    }
    else {
        throw __CodeGeneratorException();
    }
}

Generator<const LexicalToken*> TokenRunGenerator(const std::vector<ProgramPage>& tokens) {
    currentLine = 0;
    currentFile = nullptr;
    for (const auto& file: tokens) {
        currentFile = &file.file_name;
        for (const auto& line: file.page) {
            currentLine = line.line_number;
            for (const auto& token: line.line) {
                lastToken = &token;
                co_yield &token;
            }
        }
    }
}

void RunParsing(const std::vector<ProgramPage>& tokens) {

    // Step 1: syntax analysis and creating initial structures
    {
        auto generator = TokenRunGenerator(tokens);
        if (RunSyntaxAnalysis(generator)) {
            ParserError("Syntax analysis errors occurred!");
        }
    }

    // Step 2: checking basic types (might not be required)

    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished type parsing with : " << parserTypes.size() << " type definition(s)\n";
    }

    // Step 3: checking complex types

    // ...

    // Step 4: preparing function arguments and checking if main exists
    PrepareFunctionArguments();
    if (not parserFunctions.isKey("main")) {
        ParserError("No main function found!");
    }

    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished function parsing with : " << parserFunctions.size()
                  << " function definition(s)\n";
    }
}