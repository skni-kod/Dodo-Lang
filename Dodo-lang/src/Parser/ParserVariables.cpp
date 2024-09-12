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
            delete variant.declarationInstruction;
            variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            delete variant.returnInstruction;
            variant.returnInstruction = nullptr;
            break;
        case Type::valueChange:
            delete variant.valueChangeInstruction;
            variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            delete variant.functionCallInstruction;
            variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            delete variant.ifInstruction;
            variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            delete variant.whileInstruction;
            variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            delete variant.doWhileInstruction;
            variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            delete variant.forInstruction;
            variant.forInstruction = nullptr;
            break;
    }
}

FunctionInstruction::FunctionInstruction(const FunctionInstruction &F) {
    type = F.type;
    switch (type) {
        case Type::declaration:
            variant.declarationInstruction = F.variant.declarationInstruction;
            break;
        case Type::returnValue:
            variant.returnInstruction = F.variant.returnInstruction;
            break;
        case Type::valueChange:
            variant.valueChangeInstruction = F.variant.valueChangeInstruction;
            break;
        case Type::functionCall:
            variant.functionCallInstruction = F.variant.functionCallInstruction;
            break;
        case Type::ifStatement:
            variant.ifInstruction = F.variant.ifInstruction;
            break;
        case Type::whileStatement:
            variant.whileInstruction = F.variant.whileInstruction;
            break;
        case Type::doWhileStatement:
            variant.doWhileInstruction = F.variant.doWhileInstruction;
            break;
        case Type::forStatement:
            variant.forInstruction = F.variant.forInstruction;
            break;
    }
}

FunctionInstruction::FunctionInstruction(FunctionInstruction &&F) noexcept {
    type = F.type;
    switch (type) {
        case Type::declaration:
            variant.declarationInstruction = F.variant.declarationInstruction;
            F.variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            variant.returnInstruction = F.variant.returnInstruction;
            F.variant.returnInstruction = nullptr;
            break;
        case Type::valueChange:
            variant.valueChangeInstruction = F.variant.valueChangeInstruction;
            F.variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            variant.functionCallInstruction = F.variant.functionCallInstruction;
            F.variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            variant.ifInstruction = F.variant.ifInstruction;
            F.variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            variant.whileInstruction = F.variant.whileInstruction;
            F.variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            variant.doWhileInstruction = F.variant.doWhileInstruction;
            F.variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            variant.forInstruction = F.variant.forInstruction;
            F.variant.forInstruction = nullptr;
            break;
    }
}

void FunctionInstruction::DeleteAfterCopy() {
    switch (type) {
        case Type::declaration:
            variant.declarationInstruction = nullptr;
            break;
        case Type::returnValue:
            variant.returnInstruction = nullptr;
            break;
        case Type::valueChange:
            variant.valueChangeInstruction = nullptr;
            break;
        case Type::functionCall:
            variant.functionCallInstruction = nullptr;
            break;
        case Type::ifStatement:
            variant.ifInstruction = nullptr;
            break;
        case Type::whileStatement:
            variant.whileInstruction = nullptr;
            break;
        case Type::doWhileStatement:
            variant.doWhileInstruction = nullptr;
            break;
        case Type::forStatement:
            variant.forInstruction = nullptr;
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
