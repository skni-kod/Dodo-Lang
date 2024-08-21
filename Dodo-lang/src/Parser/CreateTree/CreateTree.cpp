#include "Parser.hpp"
#include "CreateTree.hpp"
#include "ParserVariables.hpp"


void ASTError(const std::string& message) {
    std::cout << "ERROR! AST Tree construction failed : " << message << "\n";
    throw ParserException();
}

ASTTree CreateTree() {
    ASTTree tree;
    if (!parserFunctions.isKey("main")) {
        ASTError("No function \"main\" found!");
    }

    return tree;
}