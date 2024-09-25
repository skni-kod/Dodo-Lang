#include "ParserVariables.hpp"
#include "Parser.hpp"
#include "Generator.tpp"
#include "SyntaxAnalysis/AnalysisInternal.hpp"
#include "GenerateCode.hpp"

bool IsType(const std::string& token) {
    return parserTypes.isKey(token);
}

ParserType::ParserType(uint8_t type, uint8_t size) {
    this->type = type;
    this->size = size;
}

ParserType::ParserType(uint8_t type, uint8_t size, std::string name) {
    this->type = type;
    this->size = size;
    this->name = name;
}

std::ostream& operator<<(std::ostream& out, const VariableType& type) {
    switch (type.subtype) {
        case VariableType::Subtype::value:
            out << "value of ";
            break;
        case VariableType::Subtype::reference:
            out << "reference to ";
            break;
        case VariableType::Subtype::pointer:
            out << "pointer to ";
            break;
        default:
            if (doneParsing) {
                CodeGeneratorError("Invalid variable subtype!");
            }
            ParserError("Invalid variable subtype!");
    }
    switch (type.type) {
        case ParserType::Type::unsignedInteger:
            out << "unsigned integer variable ";
            break;
        case ParserType::Type::signedInteger:
            out << "signed integer variable ";
            break;
        case ParserType::Type::floatingPoint:
            out << "floating point variable ";
            break;
        default:
            if (doneParsing) {
                CodeGeneratorError("Invalid variable type!");
            }
            ParserError("Invalid variable type!");
    }
    out << "sized " << uint64_t(type.size) << " bytes";
    return out;
}


VariableType::VariableType(uint8_t size, uint8_t type, uint8_t subtype) : size(size), type(type), subtype(subtype) {}

VariableType::VariableType(const std::string& typeName, uint8_t subtype) {
    auto& type = parserTypes[typeName];
    this->subtype = subtype;
    this->size = type.size;
    this->type = type.type;
}

VariableType::VariableType(const ParserType& type, uint8_t subtype) {
    this->subtype = subtype;
    this->size = type.size;
    this->type = type.type;
}

bool VariableType::operator==(const VariableType& var) {
    if (size == var.size and type == var.type and subtype == var.subtype) {
        return true;
    }
    return false;
}

std::string VariableType::GetPrefix() const {
    std::string prefix;
    switch (type) {
        case ParserType::unsignedInteger:
            prefix += 'u';
            break;
        case ParserType::signedInteger:
            prefix += 'i';
            break;
        case ParserType::floatingPoint:
            prefix += 'f';
            break;
        default:
            CodeGeneratorError("Invalid type somehow");
    }
    prefix += std::to_string(size);
    switch (subtype) {
        case Subtype::value:
            return prefix + "-";
        case Subtype::reference:
            return prefix + "&-";
        case Subtype::pointer:
            return prefix + "*-";
        default:
            CodeGeneratorError("Invalid type somehow");
    }
    return "";
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

FunctionInstruction::FunctionInstruction(const FunctionInstruction& F) {
    type = F.type;
    sourceLine = F.sourceLine;
    sourceFile = F.sourceFile;
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

FunctionInstruction::FunctionInstruction(FunctionInstruction&& F) noexcept {
    type = F.type;
    sourceLine = F.sourceLine;
    sourceFile = F.sourceFile;
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
    if (val.empty()) {
        ParserError("Empty string passed to numeric conversion!");
    }

    if (val.front() == '-' and val.size() == 1) {
        isNegative = true;
    }

    // checking if it's a 0x..., 0b... or 0o...
    if (val.size() > 2 and val.front() == '0' and (val[1] == 'x' or val[1] == 'b' or val[1] == 'o' or val[1] == 'q')) {
        uint64_t base;
        switch (val[1]) {
            case 'x':
                base = 16;
                for (uint16_t n = 2; n < val.size(); n++) {
                    if ((val[n] < '0' or val[n] > '9') and (val[n] < 'A' or val[n] > 'F') and
                        (val[n] < 'a' or val[n] > 'f')) {
                        ParserError("Invalid hexadecimal number format!");
                    }
                    if (val[n] >= 'a') {
                        val[n] = val[n] - 'a' + '9' + 1;
                    }
                    if (val[n] >= 'A') {
                        val[n] = val[n] - 'A' + '9' + 1;
                        continue;
                    }
                }
                break;
            case 'o':
                base = 8;
                for (uint16_t n = 2; n < val.size(); n++) {
                    if (val[n] < '0' or val[n] > '7') {
                        ParserError("Invalid octal number format!");
                    }
                }
                break;
            case 'q':
                for (uint16_t n = 2; n < val.size(); n++) {
                    if (val[n] < '0' or val[n] > '3') {
                        ParserError("Invalid base 4 number format!");
                    }
                }
                base = 4;
                break;
            case 'b':
                for (uint16_t n = 2; n < val.size(); n++) {
                    if (val[n] != '0' and val[n] != '1') {
                        ParserError("Invalid binary number format!");
                    }
                }
                base = 2;
                break;
        }
        uint64_t sum = 0;
        for (uint16_t n = 2; n < val.size(); n++) {
            sum *= base;
            sum += val[n] - '0';
        }
        value = std::make_unique<std::string>(std::to_string(sum));
        return;
    }

    bool isFloatingPoint = false;
    for (auto& n: val) {
        if (n == '.') {
            if (isFloatingPoint == false) {
                isFloatingPoint = true;
            }
            else {
                ParserError("Multi dot floating point number!");
            }
        }
    }

    value = std::make_unique<std::string>(val);

    if (isFloatingPoint) {
        operationType = Value::floatingPoint;
        return;
    }

    if (isNegative) {
        operationType = Value::signedInteger;
        return;
    }

    operationType = Value::unsignedInteger;
}

void ParserCondition::SetOperand(const std::string& value) {
    if (value == "==") {
        type = Condition::equals;
        return;
    }
    if (value == "!=") {
        type = Condition::notEquals;
        return;
    }
    if (value == ">") {
        type = Condition::greater;
        return;
    }
    if (value == "<") {
        type = Condition::lesser;
        return;
    }
    if (value == ">=") {
        type = Condition::greaterEqual;
        return;
    }
    if (value == "<=") {
        type = Condition::lesserEqual;
        return;
    }
    ParserError("Invalid comparison operator!");
}
