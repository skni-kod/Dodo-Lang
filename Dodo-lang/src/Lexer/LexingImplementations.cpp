#include <iostream>

#include "LexicalToken.hpp"
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
            switch (token.kw) {
                case Keyword::Primitive:
                    return out << "Keyword: primitive,";
                case Keyword::TypeSI:
                    return out << "Keyword: signed integer,";
                case Keyword::TypeUI:
                    return out << "Keyword: unsigned integer,";
                case Keyword::TypeFP:
                    return out << "Keyword: floating point,";
                case Keyword::Type:
                    return out << "Keyword: type,";
                case Keyword::Void:
                    return out << "Keyword: void,";
                case Keyword::Operator:
                    return out << "Keyword: operator,";
                case Keyword::Return:
                    return out << "Keyword: return,";
                case Keyword::Import:
                    return out << "Keyword: import,";
                case Keyword::End:
                    return out << "Keyword: end,";
                case Keyword::After:
                    return out << "Keyword: after,";
                case Keyword::Extern:
                    return out << "Keyword: extern,";
                case Keyword::Syscall:
                    return out << "Keyword: syscall,";
                case Keyword::Public:
                    return out << "Keyword: public,";
                case Keyword::Private:
                    return out << "Keyword: private,";
                case Keyword::Protected:
                    return out << "Keyword: protected,";
                case Keyword::Let:
                    return out << "Keyword: let,";
                case Keyword::Mut:
                    return out << "Keyword: mut,";
                case Keyword::Const:
                    return out << "Keyword: const,";
                case Keyword::Comma:
                    return out << "Keyword: comma,";
                case Keyword::Dot:
                    return out << "Keyword: dot,";
                case Keyword::Member:
                    return out << "Keyword: member,";
                default:
                    LexerError("Internal bug - invalid keyword in printing!");
            }
        case Token::Operator:
            switch (token.op) {
                case Operator::Assign:
                    return out << "Operator: =,";
                case Operator::Add:
                    return out << "Operator: +,";
                case Operator::Subtract:
                    return out << "Operator: -,";
                case Operator::Multiply:
                    return out << "Operator: *,";
                case Operator::Divide:
                    return out << "Operator: /,";
                case Operator::Power:
                    return out << "Operator: ^,";
                case Operator::Modulo:
                    return out << "Operator: %,";
                case Operator::NOr:
                    return out << "Operator: NOR,";
                case Operator::BinNOr:
                    return out << "Operator: binary NOR,";
                case Operator::NAnd:
                    return out << "Operator: NAND,";
                case Operator::BinNAnd:
                    return out << "Operator: binary NAND,";
                case Operator::Macro:
                    return out << "Operator: macro,";
                case Operator::Not:
                    return out << "Operator: NOT,";
                case Operator::BinNot:
                    return out << "Operator: binary NOT,";
                case Operator::Or:
                    return out << "Operator: OR,";
                case Operator::BinOr:
                    return out << "Operator: binary OR,";
                case Operator::And:
                    return out << "Operator: AND,";
                case Operator::BinAnd:
                    return out << "Operator: binary AND,";
                case Operator::XOr:
                    return out << "Operator: XOR,";
                case Operator::BinXOr:
                    return out << "Operator: binary XOR,";
                case Operator::Imply:
                    return out << "Operator: IMPLY,";
                case Operator::NImply:
                    return out << "Operator: NIMPLY,";
                case Operator::BinImply:
                    return out << "Operator: binary IMPLY,";
                case Operator::BinNImply:
                    return out << "Operator: binary NIMPLY,";
                case Operator::Lesser:
                    return out << "Operator: <,";
                case Operator::Greater:
                    return out << "Operator: >,";
                case Operator::Equals:
                    return out << "Operator: ==,";
                case Operator::LesserEqual:
                    return out << "Operator: <=,";
                case Operator::GreaterEqual:
                    return out << "Operator: >=,";
                case Operator::NotEqual:
                    return out << "Operator: !=,";
                case Operator::BracketOpen:
                    return out << "Operator: (,";
                case Operator::BracketClose:
                    return out << "Operator: ),";
                case Operator::BraceOpen:
                    return out << "Operator: {,";
                case Operator::BraceClose:
                    return out << "Operator: },";
                case Operator::IndexOpen:
                    return out << "Operator: [,";
                case Operator::IndexClose:
                    return out << "Operator: ],";
                case Operator::Index:
                    return out << "Operator: index,";
                case Operator::Increment:
                    return out << "Operator: ++,";
                case Operator::Decrement:
                    return out << "Operator: --,";
                case Operator::ShiftRight:
                    return out << "Operator: shift right,";
                case Operator::ShiftLeft:
                    return out << "Operator: shift left,";
                default:
                    LexerError("Internal bug - invalid operator in printing!");
            }
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

std::generator<const LexerToken*> TokenGenerator(std::vector <LexerFile>& files) {
    for (const auto& file : files) {
        currentFile = &file.path;
        for (const auto& line : file.lines) {
            currentLine = line.lineNumber;
            for (const auto& token : line.tokens) {
                currentCharacter = token.characterNumber;
                co_yield &token;
            }
        }
    }
}

bool LexerToken::MatchOperator(const uint64_t type) const {
    if (this->type != Token::Operator) {
        return false;
    }
    if (op == type) {
        return true;
    }
    return false;
}
bool LexerToken::MatchKeyword (const uint64_t type) const {
    if (this->type != Token::Keyword) {
        return false;
    }
    if (kw == type) {
        return true;
    }
    return false;
}
bool LexerToken::MatchNumber (const uint64_t type) const {
    if (this->type != Token::Number) {
        return false;
    }
    if (literalType == type) {
        return true;
    }
    return false;
}
