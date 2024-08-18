#include "ParserVariables.hpp"
#include "Parser.hpp"
#include "Generator.tpp"

bool IsType(const std::string& token) {
    return parserTypes.isKey(token);
}

ParserType::ParserType(uint8_t type, uint8_t size) : type(type), size(size) {}

ParserValue::ParserValue(Generator<const LexicalToken*> &generator) {
    // not implemented as of now
    const LexicalToken* current = generator();
    while (current->type != LexicalToken::Type::comma and current->type != LexicalToken::Type::expressionEnd) {
        current = generator();
    }
}

ReturnInstruction::ReturnInstruction(Generator<const LexicalToken*> &generator) {
    expression = ParserValue(generator);
}

FunctionInstruction::~FunctionInstruction() {
    switch (type) {
        case Type::declaration:
            delete Variant.declarationInstruction;
            Variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            delete Variant.returnInstruction;
            Variant.returnInstruction = nullptr;
            break;
    }
}

FunctionInstruction::FunctionInstruction(const FunctionInstruction &F) {
    type = F.type;
}

FunctionInstruction::FunctionInstruction(FunctionInstruction &&F) {
    type = F.type;
    switch (type) {
        case Type::declaration:
            F.Variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            F.Variant.returnInstruction = nullptr;
            break;
    }
}
