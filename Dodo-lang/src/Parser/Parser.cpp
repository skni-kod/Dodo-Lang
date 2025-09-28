#include "Parser.hpp"

#include <AnalysisInternal.hpp>
#include <iostream>

#include "Generator.tpp"
#include "SyntaxAnalysis/SyntaxAnalysis.hpp"
#include "GenerateCode.hpp"
#include "Lexing.hpp"
#include "Options.hpp"


uint64_t GetCurrentLine() {
    return currentLine;
}

const char* ParserException::what() {
    return "Parser has encountered unexpected input";
}

uint64_t passedStringCounter = 0;

// TODO: make this faster
void AddString(std::string* string) {
    if (passedStrings.contains(*string)) return;
    if (string->length() < 32)
        passedStrings.emplace(*string, passedStringCounter++);
    else
        passedLongStrings.emplace_back(string, passedStringCounter++);
}

uint64_t FindString(std::string* string) {
    if (passedStrings.contains(*string)) return passedStrings[*string];
    for (auto& n : passedLongStrings) {
        if (*n.first == *string)
            return n.second;
    }
    ParserError("Could not find a passed string!");
    return 0;
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
    ResolveParameterTypes();

    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished type parsing with: " << types.size() << " type definition(s)\n";
        if (Options::informationLevel > Options::InformationLevel::general) {
            std::cout << "INFO L3: Defined types:\n";
            for (const auto& n : types) {
                std::cout << n.second;
            }
        }
    }

    // Step 3: processing global variables
    CheckGlobalVariables();

    if (Options::informationLevel > Options::InformationLevel::minimal) {
        std::cout << "INFO L2: Finished global variable checks with: " << globalVariables.size() << " instance(s)\n";
        if (Options::informationLevel > Options::InformationLevel::general and not globalVariables.empty()) {
            std::cout << "INFO L3: Global variables:\n";
            for (const auto& n : globalVariables) {
                std::cout << n.second;
            }
        }
    }

    // Step 4: adding parent type pointers to methods
    for (auto& n : types) {
        for (auto& m : n.second.methods) {
            m.parentType = &n.second;
        }
    }
}