#include "ErrorHandling.hpp"

CompilationStage::Type CurrentCompilationStage = CompilationStage::setup;
std::string LastMessage{};

// TODO: add accurate information about current lines/tokens being processed
const char* CompilerException::what() {
    switch (CurrentCompilationStage) {
        case CompilationStage::setup:
            LastMessage = "Error: in compiler setup: " + LastMessage;
            break;
        case CompilationStage::lexing:
            LastMessage = "Error: in lexer: " + LastMessage;
            break;
        case CompilationStage::parsing:
            LastMessage = "Error: in parser: " + LastMessage;
            break;
        case CompilationStage::bytecode:
            LastMessage = "Error: in intermediate code generator: " + LastMessage;
            break;
        case CompilationStage::assembly:
            LastMessage = "Error: in assembly code generator: " + LastMessage;
            break;
        case CompilationStage::output:
            LastMessage = "Error: in output: " + LastMessage;
            break;
    default:
        return "Double error: Invalid stage set in exception handling!";
    }
    if (LastMessage.back() != '!')
        LastMessage += "!";
    return (LastMessage = LastMessage + "\n").c_str();
}

CompilerException::CompilerException(const std::string message) : message((LastMessage = message).c_str()) { }

void SetCompilationStage(CompilationStage::Type stage) {
    CurrentCompilationStage = stage;
}
