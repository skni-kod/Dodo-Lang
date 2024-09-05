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
        case Type::valueChange:
            delete Variant.valueChangeInstruction;
            Variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            delete Variant.functionCallInstruction;
            Variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            delete Variant.ifInstruction;
            Variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            delete Variant.whileInstruction;
            Variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            delete Variant.doWhileInstruction;
            Variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            delete Variant.forInstruction;
            Variant.forInstruction = nullptr;
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
        case Type::valueChange:
            Variant.valueChangeInstruction = F.Variant.valueChangeInstruction;
            break;
        case Type::functionCall:
            Variant.functionCallInstruction = F.Variant.functionCallInstruction;
            break;
        case Type::ifStatement:
            Variant.ifInstruction = F.Variant.ifInstruction;
            break;
        case Type::whileStatement:
            Variant.whileInstruction = F.Variant.whileInstruction;
            break;
        case Type::doWhileStatement:
            Variant.doWhileInstruction = F.Variant.doWhileInstruction;
            break;
        case Type::forStatement:
            Variant.forInstruction = F.Variant.forInstruction;
            break;
    }
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
        case Type::valueChange:
            Variant.valueChangeInstruction = F.Variant.valueChangeInstruction;
            F.Variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            Variant.functionCallInstruction = F.Variant.functionCallInstruction;
            F.Variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            Variant.ifInstruction = F.Variant.ifInstruction;
            F.Variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            Variant.whileInstruction = F.Variant.whileInstruction;
            F.Variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            Variant.doWhileInstruction = F.Variant.doWhileInstruction;
            F.Variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            Variant.forInstruction = F.Variant.forInstruction;
            F.Variant.forInstruction = nullptr;
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
        case Type::valueChange:
            Variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            Variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            Variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            Variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            Variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            Variant.forInstruction = nullptr;
            break;
    }
}

void ParserValue::fillValue(std::string val) {
    std::string temp = val;
    if (temp.empty()) {
        ParserError("Empty string passed to numeric conversion!");
    }

    if (temp.front() == '-') {
        if (temp.size() == 1) {
            ParserError("A \"-\" string passed to numeric conversion!");
        }
        temp = temp.substr(1, temp.size() - 1);
        isNegative = true;
    }

    // checking if it's a 0x..., 0b... or 0o...
    if (temp.size() > 2 and temp.front() == 0 and (temp[1] == 'x' or temp[1] == 'b' or temp[1] == 'o')) {
        ParserError("Non decimals not yet supported!");
        // TODO: add non decimals
        return;
    }

    bool isFloatingPoint = false;
    for (auto& n : temp) {
        if (n == '.') {
            if (isFloatingPoint == false) {
                isFloatingPoint = true;
            }
            else {
                ParserError("Multi dot floating point number!");
            }
        }
    }

    if (isFloatingPoint) {
        // TODO: add floats
        ParserError("Floating point numbers not yet supported!");
        return;
    }

    // at this point val is a valid decimal, so just input it into the pointer
    value = std::make_unique<std::string>(val);
    operationType = Value::signedInteger;
}

void ParserCondition::SetOperand(const std::string &value) {
    if (value == "==") {
        type = Type::equals;
        return;
    }
    if (value == "!=") {
        type = Type::notEquals;
        return;
    }
    if (value == ">") {
        type = Type::greater;
        return;
    }
    if (value == "<") {
        type = Type::lesser;
        return;
    }
    if (value == ">=") {
        type = Type::greaterEqual;
        return;
    }
    if (value == "<=") {
        type = Type::lesserEqual;
        return;
    }
    ParserError("Invalid comparison operator!");
}
