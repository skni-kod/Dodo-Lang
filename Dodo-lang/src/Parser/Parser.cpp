#include "Parser.hpp"
#include "Generator.tpp"
#include "ParserVariables.hpp"
#include "SyntaxAnalysis/SyntaxAnalysis.hpp"
#include "GenerateCode.hpp"
#include "Lexer/Lexing.hpp"


//uint64_t currentLine = 0;
//const std::string* currentFile = nullptr;

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
    throw __CodeGeneratorException();

}

Generator<const LexerToken*> LexerTokenGenerator(std::vector<LexerFile>& lexed) {
    for (const auto& file: lexed) {
        currentFile = &file.path;
        for (const auto& line: file.lines) {
            currentLine = line.lineNumber;
            for (const auto& token: line.tokens) {
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

    // Step 2: processing types without their methods
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

bool IsNumeric(const LexicalToken* token) {
    if (token->literalValue == literalType::numeric or
        token->literalValue == literalType::character or
        token->literalValue == literalType::float_type or
        token->literalValue == literalType::hex_type or
        token->literalValue == literalType::binary_type or
        token->literalValue == literalType::octal_type) {
        return true;
        }
    return false;
}