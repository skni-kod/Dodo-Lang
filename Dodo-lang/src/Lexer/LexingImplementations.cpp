#include <iostream>

#include "Lexing.hpp"
#include "LexingInternal.hpp"

const char* LexerException::what() {
    return "Lexer has encountered unexpected input";
}

void LexerError(const std::string& message) {
    std::print(std::cout, "ERROR! In file: {}, at line: {}, character: {}\nMessage: {}\n",
        (currentFile != nullptr ? '"' + *currentFile + '"': "unknown"), currentLine, currentCharacter, message);
    throw LexerException();
}

LexerToken::~LexerToken() {
    if ((type == Token::Identifier or type == Token::String) and text != nullptr) {
        delete text;
        text = nullptr;
    }
}

LexerToken::LexerToken(const uint8_t type, uint64_t value, const uint32_t characterNumber, uint8_t isVerboseOperator) {
    this->type = type;
    this->_unsigned = value;
    this->characterNumber = characterNumber;
    this->literalType = Value::unsignedInteger;
    this->isVerboseOperator = isVerboseOperator;
}

LexerToken::LexerToken(const uint8_t type, int64_t value, const uint32_t characterNumber) {
    this->type = type;
    this->_signed = value;
    this->literalType = Value::signedInteger;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const LexerToken& value, const uint32_t characterNumber) {
    this->type = value.type;
    this->_unsigned = value._unsigned;
    this->literalType = value.literalType;
    this->isVerboseOperator = value.isVerboseOperator;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const uint8_t type, const double value, const uint32_t characterNumber) {
    this->type = type;
    this->_double = value;
    this->literalType = Value::floatingPoint;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const uint8_t type, const std::string& text, const uint32_t characterNumber) {
    this->type = type;
    this->text = new std::string(text);
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const std::string& key, const uint32_t characterNumber) {
    auto result = keywordsAndOperators[key];
    this->type = result.type;
    this->_unsigned = result._unsigned;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(LexerToken&& other)  noexcept {
    type = other.type;
    literalType = other.literalType;
    isVerboseOperator = other.isVerboseOperator;
    _unsigned = other._unsigned;
    other._unsigned = 0;
}

LexerToken::LexerToken(LexerToken& other) {
    type = other.type;
    literalType = other.literalType;
    isVerboseOperator = other.isVerboseOperator;
    _unsigned = other._unsigned;
    if ((other.type == Token::Identifier or other.type == Token::String) and other.text != nullptr) {
        text = new std::string(*other.text);
    }
}

LexerToken& LexerToken::operator=(const LexerToken& other) {
    type = other.type;
    literalType = other.literalType;
    isVerboseOperator = other.isVerboseOperator;
    _unsigned = other._unsigned;
    if ((other.type == Token::Identifier or other.type == Token::String) and other.text != nullptr) {
        text = new std::string(*other.text);
    }
    return *this;
}

// TODO: that is stupid, find a way to fix it, i'm not even copying these values so yeah
// shared_ptr?
LexerToken::LexerToken(const LexerToken& other) {
    type = other.type;
    literalType = other.literalType;
    isVerboseOperator = other.isVerboseOperator;
    _unsigned = other._unsigned;
    if (other.type == Token::Identifier and other.text != nullptr) {
        text = new std::string(*other.text);
    }
}

std::ostream& operator<<(std::ostream& out, const LexerToken& token) {
    switch (token.type) {
        case Token::Identifier:
            return out << "Identifier: \"" << *token.text << "\",";
        case Token::String:
            return out << "String: \"" << *token.text << "\",";
        case Token::Keyword:
            return out << "Keyword No: " << token._unsigned << ",";
        case Token::Operator:
            return out << "Operator No: " << token._unsigned << ",";
        case Token::Number:
            switch (token.literalType) {
                case Type::floatingPoint:
                    return out << "Float: " << token._double;
                case Type::unsignedInteger:
                    return out << "Integer: " << token._unsigned;
                case Type::signedInteger:
                    return out << "Integer: " << token._signed;
                default:
                    break;
            }
        default:
            break;
    }
    return out;
}