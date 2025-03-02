#include "AnalysisInternal.hpp"

bool IsExpressionEndToken(const LexerToken* token) {
    switch (token->type) {
        case Token::Keyword:
            switch (token->kw) {
                case Keyword::End:
                case Keyword::Comma:
                    return true;
                default:
                    return false;
            }
        case Token::Operator:
            switch (token->kw) {
                case Operator::BraceClose:
                case Operator::BracketClose:
                case Operator::IndexClose:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

const LexerToken* ParseExpression(Generator <const LexerToken*>& generator, std::vector <ParserTreeValue>& valueArray, std::vector <const LexerToken*> tokens) {
    // first of all let's
    const LexerToken* last = nullptr;
    uint16_t bracketLevel = 0, braceLevel = 0, indexLevel = 0;
    for (uint32_t n = 0; n < tokens.size(); n++) {
        if (bracketLevel == 0 and braceLevel == 0 and indexLevel == 0 and IsExpressionEndToken(tokens[n])) {
            if (n != tokens.size() - 1) {
                ParserError("Invalid expression!");
            }
            last = tokens.back();
            tokens.pop_back();
        }

        if (tokens[n]->type == Token::Operator) {
            switch (tokens[n]->op) {
                case Operator::BraceOpen:
                    braceLevel++;
                break;
                case Operator::BracketOpen:
                    bracketLevel++;
                break;
                case Operator::IndexOpen:
                    indexLevel++;
                break;
                case Operator::BraceClose:
                    if (braceLevel == 0) {
                        ParserError("Invalid braces!");
                    }
                    braceLevel--;
                    break;
                case Operator::BracketClose:
                    if (bracketLevel == 0) {
                        ParserError("Invalid brackets!");
                    }
                    bracketLevel--;
                break;
                case Operator::IndexClose:
                    if (indexLevel == 0) {
                        ParserError("Invalid index brackets!");
                    }
                    indexLevel--;
                break;
                default:
                    break;
            }
        }

        if (n == tokens.size() - 1) {
            tokens.push_back(generator());
        }
    }

    // now the entire expression bounds are done and it can be parsed into a tree


    return last;
}




// DEPRECATED
ParserValue ParseMathInternal(const std::vector<const LexicalToken*>& tokens, std::pair<uint32_t, uint32_t> range) {
    uint32_t size = range.second - range.first;
    if (size == 1 and not IsNumeric(tokens[range.first]) and tokens[range.first]->value.size() > 1  and tokens[range.first]->value.front() == '\"' and tokens[range.first]->value.back() == '\"') {
        // a string
        
        ParserValue value;
        value.nodeType = ParserValue::Node::constant;
        value.valueType = ParserValue::ValueType::string;
        value.value = std::make_unique<std::string>(std::to_string(passedStrings.size()));
        if (tokens[range.first]->value != "syscall") {
            passedStrings.push_back(tokens[range.first]->value);
        }
        return value;
    }

    // a single constant or variable value
    if (size == 1) {
        if (tokens[range.first]->type == LexicalToken::Type::identifier) {
            ParserValue value;
            value.nodeType = ParserValue::Node::variable;
            value.value = std::make_unique<std::string>(tokens[range.first]->value);
            return value;
        }
        else if (tokens[range.first]->type == LexicalToken::Type::literal) {
            ParserValue value;
            value.nodeType = ParserValue::Node::constant;
            value.fillValue(tokens[range.first]->value);
            return value;
        }
        ParserError("Unexpected token when attempting to extract value!");
    }

    // a negative value of variable or constant
    if (size == 2) {
        if (tokens[range.first + 1]->type == LexicalToken::Type::identifier) {
            if (tokens[range.first]->value == "-") {
                ParserValue value;
                value.nodeType = ParserValue::Node::operation;
                value.operationType = ParserValue::Operation::subtraction;
                value.left = std::make_unique<ParserValue>();
                value.left->nodeType = ParserValue::Node::constant;
                value.left->fillValue("0");
                value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first + 1, range.second}));
                return std::move(value);
            }
            else if (tokens[range.first]->value == "&") {
                // address extraction
                ParserValue value;
                value.nodeType = ParserValue::Node::operation;
                value.operationType = ParserValue::Operation::getAddress;
                value.value = std::make_unique<std::string>(tokens[range.first + 1]->value);
                return std::move(value);
            }
            else if (tokens[range.first]->value == "*") {
                // address extraction
                ParserValue value;
                value.nodeType = ParserValue::Node::operation;
                value.operationType = ParserValue::Operation::getValue;
                value.value = std::make_unique<std::string>(tokens[range.first + 1]->value);
                return std::move(value);
            }
        }
        else if (tokens[range.first + 1]->type == LexicalToken::Type::literal and tokens[range.first]->value == "-") {
            if (not IsNumeric(tokens[range.first + 1])) {
                ParserError("Invalid literal type in negative sign operation: " + tokens[range.first + 1]->value + "!");
            }
            ParserValue value;
            value.nodeType = ParserValue::Node::constant;
            if (tokens[range.first + 1]->value.front() == '-') {
                value.fillValue(tokens[range.first]->value.substr(1, tokens[range.first]->value.size() - 1));
            }
            else {
                value.fillValue("-" + tokens[range.first + 1]->value);
            }
            return std::move(value);
        }

        ParserError("Unexpected two token pair in mathematical expression: " + tokens[range.first]->value + " "
                        + tokens[range.first + 1]->value + "!");
    }

    if (size > 2 and tokens[range.first]->value == "(" and tokens[range.second - 1]->value == ")") {
        bool isAllBracketed = true;
        uint64_t bracketLevel = 1;
        for (uint64_t n = range.first + 1; n < range.second - 1; n++) {
            if (tokens[n]->value == "(") {
                bracketLevel++;
                continue;
            }
            if (tokens[n]->value == ")") {
                bracketLevel--;
                if (bracketLevel == 0) {
                    isAllBracketed = false;
                    break;
                }
                continue;
            }
        }
        if (isAllBracketed) {
            return ParseMathInternal(tokens, {range.first + 1, range.second - 1});
        }
    }

    // value returning function call
    // TODO: add argument parsing
    if (size > 2 and (tokens[range.first]->type == LexicalToken::Type::identifier or tokens[range.first]->value == "syscall") and
        tokens[range.first + 1]->value == "(" and tokens[range.second - 1]->value == ")") {
        ParserValue value;
        value.nodeType = ParserValue::Node::operation;
        value.operationType = ParserValue::Operation::functionCall;
        value.value = std::make_unique<std::string>(tokens[range.first]->value);
        // left side is the next node, right is the argument itself
        ParserValue* current = &value;
        uint64_t first = range.first + 2;
        int64_t bracketLevel = 0;
        for (uint64_t n = range.first + 1; n < range.second; n++) {
            if (tokens[n]->value == "(") {
                bracketLevel++;
                continue;
            }
            else if (tokens[n]->value == ")") {
                bracketLevel--;
            }
            if (tokens[n]->value == "," or (tokens[n]->value == ")" and bracketLevel == 0)) {
                // time to parse whatever was there
                // this means there was nothing
                if (n == first) {
                    first = n + 1;
                    continue;
                }
                current->right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {first, n}));
                first = n + 1;
                // if not the last found argument add another
                if (tokens[n]->value == ",") {
                    current->left = std::make_unique<ParserValue>();
                    current = current->left.get();
                    current->nodeType = ParserValue::Node::operation;
                    current->operationType = ParserValue::Operation::functionCall;
                }
            }
        }
        return std::move(value);
    }

    // CoMpLeX expression parsing

    // bracket check definitions to avoid repeating most of the code
    uint32_t bracketLevel = 0;
#define MATH_CHECK_BRACKET                          \
    if (tokens[n]->value == ")") {                  \
        bracketLevel++;                             \
        continue;                                   \
    }                                               \
    if (tokens[n]->value == "(") {                  \
        if (bracketLevel == 0) {                    \
            ParserError("Invalid closing bracket!");\
        }                                           \
        bracketLevel--;                             \
        continue;                                   \
    }                                               \
    if (bracketLevel) {                             \
        continue;                                   \
    }
#define MATH_CHECK_BRACKET_AFTER                    \
    if (bracketLevel != 0) {                        \
        ParserError("Invalid bracket pairs!");      \
    }

    // example: 1 + 5 - 6 + 5 * 5 / 6 + 3
    // -> (1 + 5 - 6 + 5 * 5 / 6) + (3)
    // -> ((1 + 5 - 6) + (5 * 5 / 6)) + (3)
    // -> (((1) + (5 - 6)) + (5 * 5 / 6)) + (3)
    for (int64_t n = range.second - 1; n >= range.first; n--) {
        MATH_CHECK_BRACKET
        if (tokens[n]->value == "+") {
            ParserValue value;
            value.nodeType = ParserValue::Node::operation;
            value.operationType = ParserValue::Operation::addition;
            value.left = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first, n}));
            value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {n + 1, range.second}));
            return std::move(value);
        }
    }
    MATH_CHECK_BRACKET_AFTER

    // (((1) + (5 - 6)) + (5 * 5 / 6)) + (3)
    // -> (((1) + ((5) - (6))) + (5 * 5 / 6)) + (3)
    for (int64_t n = range.second - 1; n >= range.first; n--) {
        MATH_CHECK_BRACKET
        if (tokens[n]->value == "-") {
            ParserValue value;
            value.nodeType = ParserValue::Node::operation;
            value.operationType = ParserValue::Operation::subtraction;
            value.left = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first, n}));
            value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {n + 1, range.second}));
            return std::move(value);
        }
    }
    MATH_CHECK_BRACKET_AFTER

    // (((1) + ((5) - (6))) + (5 * 5 / 6)) + (3)
    // -> (((1) + ((5) - (6))) + ((5) * (5 / 6))) + (3)
    for (int64_t n = range.second - 1; n >= range.first; n--) {
        MATH_CHECK_BRACKET
        if (tokens[n]->value == "*") {
            ParserValue value;
            value.nodeType = ParserValue::Node::operation;
            value.operationType = ParserValue::Operation::multiplication;
            value.left = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first, n}));
            value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {n + 1, range.second}));
            return std::move(value);
        }
    }
    MATH_CHECK_BRACKET_AFTER

    // (((1) + ((5) - (6))) + ((5) * (5 / 6))) + (3)
    // -> (((1) + ((5) - (6))) + ((5) * ((5) / (6)))) + (3)
    for (int64_t n = range.second - 1; n >= range.first; n--) {
        MATH_CHECK_BRACKET
        if (tokens[n]->value == "/") {
            ParserValue value;
            value.nodeType = ParserValue::Node::operation;
            value.operationType = ParserValue::Operation::division;
            value.left = std::make_unique<ParserValue>(ParseMathInternal(tokens, {range.first, n}));
            value.right = std::make_unique<ParserValue>(ParseMathInternal(tokens, {n + 1, range.second}));
            return std::move(value);
        }
    }
    MATH_CHECK_BRACKET_AFTER

    return {};
}

ParserValue ParseMath(const std::vector<const LexicalToken*>& tokens) {
    return std::move(ParseMathInternal(tokens, std::make_pair<uint32_t, uint32_t>(0, tokens.size())));
}

ParserValue ParseMath(Generator<const LexicalToken*>& generator) {
    const LexicalToken* current = generator();
    std::vector<const LexicalToken*> tokens;
    int64_t bracketLevel = 0;
    while ((current->type != LexicalToken::Type::comma and current->type != LexicalToken::Type::expressionEnd) or
           bracketLevel != 0) {
        if (current->value == "(") {
            bracketLevel++;
        }
        if (current->value == ")") {
            bracketLevel--;
        }
        tokens.push_back(current);
        current = generator();
    }
    return std::move(ParseMath(tokens));
}

ParserValue ParseMath(Generator<const LexicalToken*>& generator, std::vector<const LexicalToken*> front, bool addBraces,
                      uint64_t bracketLevel) {
    const LexicalToken* current = generator();
    LexicalToken frontBrace = {LexicalToken::Type::operand, "("};
    LexicalToken backBrace = {LexicalToken::Type::operand, ")"};
    if (addBraces) {
        front.push_back(&frontBrace);
    }
    while ((current->value != "," and current->value != ";" and current->value != ")") or
           bracketLevel != 0) {
        if (current->value == "(") {
            bracketLevel++;
        }
        if (current->value == ")") {
            if (bracketLevel == 0) {
                break;
            }
            bracketLevel--;
        }
        front.push_back(current);
        current = generator();
    }
    if (addBraces) {
        front.push_back(&backBrace);
    }
    return std::move(ParseMath(front));
}


