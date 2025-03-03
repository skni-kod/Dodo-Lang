#include "Parser.hpp"
#include "Generator.tpp"
#include "ParserVariables.hpp"
#include "SyntaxAnalysis/SyntaxAnalysis.hpp"
#include "GenerateCode.hpp"
#include "Lexer/Lexing.hpp"
#include "Misc/Options.hpp"


//uint64_t currentLine = 0;
//const std::string* currentFile = nullptr;

uint64_t GetCurrentLine() {
    return currentLine;
}

const char* ParserException::what() {
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
        throw ParserException();
    }
    throw __CodeGeneratorException();

}

Generator<LexerToken*> LexerTokenGenerator(std::vector<LexerFile>& lexed) {
    for (auto& file: lexed) {
        currentFile = &file.path;
        for (auto& line: file.lines) {
            currentLine = line.lineNumber;
            for (auto& token: line.tokens) {
                //lastToken = &token;
                co_yield &token;
            }
        }
    }
}


//void RunParsing(const std::vector<ProgramPage>& tokens) {
void RunParsing(std::vector<LexerFile>& lexed) {

    // Step 1: creating unprocessed structures
    {
        auto generator = LexerTokenGenerator(lexed);
        if (RunSyntaxAnalysis(generator)) {
            ParserError("Syntax analysis errors occurred!");
        }
    }

    // Step 2: processing type sizes, alignments and names
    CalculateTypeSizes();

    // Step 3: processing method and function prototypes

    // Step 4: processing method and function contents

    // Step 5: ... profit?


    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished type parsing with " << types.size() << " type definition(s)\n";
        if (Options::informationLevel > Options::InformationLevel::general) {
            std::cout << "INFO L3: Defined types:\n";
            for (const auto& n : types.map) {
                std::cout << n.second;
            }
        }
    }

    UpdateGlobalVariables();



    /*
    PrepareFunctionArguments();
    if (not parserFunctions.isKey("Main")) {
        ParserError("No main function found!");
    }

    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished function parsing with " << parserFunctions.size()
                  << " function definition(s)\n";
    }
    */
}