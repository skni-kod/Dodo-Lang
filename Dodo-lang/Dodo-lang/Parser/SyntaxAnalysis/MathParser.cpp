#include "AnalysisInternal.hpp"

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

ParserValue ParseMathInternal(const std::vector<const LexicalToken*>& tokens, std::pair<uint32_t, uint32_t> range) {
    uint32_t size = range.second - range.first;

    // a single constant or variable value
    if (size == 1) {
        if (tokens[range.first]->type == LexicalToken::Type::identifier) {
            ParserValue value;
            value.nodeType = ParserValue::Node::variable;
            value.value = std::make_unique<std::string>(tokens[range.first]->value);
            return std::move(value);
        }
        else  if (tokens[range.first]->type == LexicalToken::Type::literal) {
            ParserValue value;
            value.nodeType = ParserValue::Node::constant;
            value.value = std::make_unique<std::string>(tokens[range.first]->value);
            return std::move(value);
        }
        ParserError("Unexpected token when attempting to extract value!");
    }

    // a negative value of variable or constant
    if (size == 2) {
        if (tokens[range.first]->type != LexicalToken::Type::operand or tokens[range.first]->value != "-") {
            ParserError("Unexpected two token pair in mathematical expression: " + tokens[range.first]->value + " "
            + tokens[range.first + 1]->value + "!");
        }
        if (tokens[range.first + 1]->type == LexicalToken::Type::identifier) {
            ParserValue value;
            value.nodeType = ParserValue::Node::operation;
            value.operationType = ParserValue::Operation::subtraction;
            value.left = std::make_unique<ParserValue>();
            value.left->nodeType = ParserValue::Node::constant;
            value.left->value = std::make_unique<std::string>("0");
            value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first + 1, range.second}));
            return std::move(value);
        }
        else if (tokens[range.first + 1]->type == LexicalToken::Type::literal) {
            if (not IsNumeric(tokens[range.first + 1])) {
                ParserError("Invalid literal type in negative sign operation: " + tokens[range.first + 1]->value + "!");
            }
            ParserValue value;
            value.nodeType = ParserValue::Node::constant;
            if (tokens[range.first]->value[0] == '-') {
                value.value = std::make_unique<std::string>(tokens[range.first]->value.substr(1, tokens[range.first]->value.size() - 1));
            }
            else {
                value.value = std::make_unique<std::string>("-" + tokens[range.first]->value);
            }
            return std::move(value);
        }
    }

    if (size > 2 and tokens[range.first]->value == "(" and tokens[range.second - 1]->value == ")") {
        return ParseMathInternal(tokens, {range.first + 1, range.second - 1});
    }

    // TODO: add complex expression handling here

    return {};
}

ParserValue ParseMath(const std::vector<const LexicalToken*>& tokens) {
    return std::move(ParseMathInternal(tokens, std::make_pair<uint32_t, uint32_t>(0, tokens.size())));
}

ParserValue ParseMath(Generator<const LexicalToken*> &generator) {
    const LexicalToken* current = generator();
    std::vector <const LexicalToken*> tokens;
    while (current->type != LexicalToken::Type::comma and current->type != LexicalToken::Type::expressionEnd) {
        tokens.push_back(current);
        current = generator();
    }
    return std::move(ParseMath(tokens));
}


