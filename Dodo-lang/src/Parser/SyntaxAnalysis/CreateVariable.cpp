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
    var.typeOrName = current->value;

    current = generator();
    if (current->type != LexicalToken::Type::identifier) {
        if (current->type == LexicalToken::Type::operand and current->value == "*") {
            while (current->value == "*") {
                var.type.subtype++;
                current = generator();
            }
        }
        else {
            ParserError("Expected an identifier after variable type!");
        }
    }
    std::string name = current->value;

    current = generator();
    if (current->type != LexicalToken::Type::operand and current->type != LexicalToken::Type::expressionEnd) {
        ParserError("Expected an operand after variable identifier!");
    }
    if (isGlobal) {
        name = "glob." + name;
    }
    
    switch (current->value[0]) {
        case ';':
            var.expression = ParserValue(ParserValue::Node::constant, 0, 0, false, nullptr, nullptr, std::make_unique<std::string>("0"));
            return {std::move(name), std::move(var)};
        case '=':
            break;
        default:
            ParserError("Unexpected operand after variable identifier!");
    }
    
    // now getting the value since there is one
    var.expression = ParseMath(generator);

    return {std::move(name), std::move(var)};
}

void UpdateGlobalVariables() {
    for (auto& n : globalVariables.map) {
        if (not parserTypes.isKey(n.second.typeOrName)) {
            ParserError("Invalid typename \"" + n.second.typeOrName + "\" in global variable named \"" + n.first + "\"!");
        }
        auto& type = parserTypes[n.second.typeOrName];
        n.second.type.type = type.type;
        n.second.type.size = type.size;
        n.second.typeOrName = VariableType(type).getPrefix() + n.first;
    }
}

