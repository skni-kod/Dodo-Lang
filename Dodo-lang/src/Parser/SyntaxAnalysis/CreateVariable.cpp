#include "AnalysisInternal.hpp"

std::pair<std::string, ParserVariable> CreateVariable(Generator<const LexicalToken*>& generator, const std::string& firstToken, bool isGlobal) {
    ParserVariable var;
    if (firstToken == "mut") {
        var.isMutable = true;
    }
    
    // now the type identifier
    auto* current = generator();
    if (current->type != LexicalToken::Type::identifier) {
        ParserError("Expected a type identifier after variable mutability prefix!");
    }
    var.typeName = current->value;

    current = generator();
    if (current->type != LexicalToken::Type::identifier) {
        ParserError("Expected an identifier after variable type!");
    }
    std::string name = current->value;

    current = generator();
    if (current->type != LexicalToken::Type::operand and current->type != LexicalToken::Type::expressionEnd) {
        ParserError("Expected an operand after variable identifier!");
    }
    if (isGlobal) {
        var.type.subtype += ParserVariable::Subtype::globalValue;
    }
    
    switch (current->value[0]) {
        case ';':
            var.expression = ParserValue(ParserValue::Node::constant, 0, 0, false, nullptr, nullptr, std::make_unique<std::string>("0"));
            return std::pair<std::string, ParserVariable> (std::move(name), std::move(var));
        case '*':
            var.type.subtype += ParserVariable::pointer;
            current = generator();
            if (current->value != "=" and current->value != ";") {
                ParserError(R"(Expected a '=' or an ';' after pointer declaration!)");
            }
            if (current->value == ";") {
                var.expression = ParserValue(ParserValue::Node::constant, 0, 0, false, nullptr, nullptr, std::make_unique<std::string>("0"));
                return std::pair<std::string, ParserVariable> (std::move(name), std::move(var));
            }
            break;
        case '=':
            break;
        default:
            ParserError("Unexpected operand after variable identifier!");
    }
    
    // now getting the value since there is one
    var.expression = ParseMath(generator);
    return std::pair<std::string, ParserVariable> (std::move(name), std::move(var));
}

void UpdateGlobalVariables() {
    for (auto& n : globalVariables.map) {
        if (not parserTypes.isKey(n.second.typeName)) {
            ParserError("Invalid typename \"" + n.second.typeName + "\" in global variable named \"" + n.first + "\"!");
        }
        auto& type = parserTypes[n.second.typeName];
        n.second.type.type = type.type;
        n.second.type.size = type.size;
    }
}

