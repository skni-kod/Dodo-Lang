#include "ParserVariables.hpp"
#include "Parser.hpp"
#include "Generator.tpp"
#include "SyntaxAnalysis/AnalysisInternal.hpp"

bool IsType(const std::string& token) {
    return parserTypes.isKey(token);
}

ParserType::ParserType(uint8_t type, uint8_t size)  {
    this->type = type;
    this->size = size;
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
    switch (type) {
        case Type::declaration:
            Variant.declarationInstruction = F.Variant.declarationInstruction;
            break;
        case Type::returnValue:
            Variant.returnInstruction = F.Variant.returnInstruction;
            break;
    }
    // bypassing these damned copy operators, this thing does not copy anywhere and it's a big pain to write all this
    // TODO: replace with reinterpret cast and getters or something
}

FunctionInstruction::FunctionInstruction(FunctionInstruction &&F) noexcept {
    type = F.type;
    switch (type) {
        case Type::declaration:
            Variant.declarationInstruction = F.Variant.declarationInstruction;
            F.Variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            Variant.returnInstruction = F.Variant.returnInstruction;
            F.Variant.returnInstruction = nullptr;
            break;
    }
}

void FunctionInstruction::DeleteAfterCopy() {
    switch (type) {
        case Type::declaration:
            Variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            Variant.returnInstruction = nullptr;
            break;
    }
}
