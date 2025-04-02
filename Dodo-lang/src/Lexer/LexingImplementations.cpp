#include <GenerateCode.hpp>
#include <iostream>
#include <generator>

#include "Lexing.hpp"
#include "LexingInternal.hpp"


const char* LexerException::what() {
    return "Lexer has encountered unexpected input";
}

void LexerError(const std::string& message) {
    std::print(std::cout, "ERROR! In file: {}, at line: {}, character: {}\nMessage: {}\n",
        (currentFile != nullptr ? '"' + currentFile->string() + '"': "unknown"), currentLine, currentCharacter, message);
    throw LexerException();
}

LexerToken::~LexerToken() {
    if ((type == Token::Identifier or type == Token::String) and text != nullptr) {
        delete text;
        text = nullptr;
    }
}

LexerToken::LexerToken(const uint8_t type, const uint64_t value, const uint32_t characterNumber, const uint8_t isVerboseOperator) {
    this->type = static_cast <Token::Type>(type);
    this->_unsigned = value;
    this->characterNumber = characterNumber;
    this->literalType = Type::unsignedInteger;
    this->isVerboseOperator = isVerboseOperator;
}

LexerToken::LexerToken(const uint8_t type, const int64_t value, const uint32_t characterNumber) {
    this->type = static_cast <Token::Type>(type);
    this->_signed = value;
    this->literalType = Type::signedInteger;
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
    this->type = static_cast <Token::Type>(type);
    this->_double = value;
    this->literalType = Type::floatingPoint;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const uint8_t type, const std::string& text, const uint32_t characterNumber) {
    this->type = static_cast <Token::Type>(type);
    this->text = new std::string(text);
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const std::string& key, const uint32_t characterNumber) {
    const auto result = keywordsAndOperators[key];
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

bool LexerToken::operator==(const LexerToken& other) const {
    if (type != other.type) {
        return false;
    }
    switch (type) {
        case Token::Identifier:
            return *text == *other.text;
        case Token::Operator:
            return op == other.op;
        case Token::Keyword:
            return kw == other.kw;
        case Token::Number:
            return literalType == other.literalType and _unsigned == other._unsigned;
        case Token::String:
            return *text == *other.text;
        default:
        return false;
    }
}

std::ostream& PrintOperatorSymbol(const Operator::Type op, std::ostream& out) {
    switch (op) {
        case Operator::Assign:
            return out << "=";
        case Operator::Add:
            return out << "+,";
        case Operator::Subtract:
            return out << "-";
        case Operator::Multiply:
            return out << "*";
        case Operator::Divide:
            return out << "/";
        case Operator::Power:
            return out << "^^";
        case Operator::Modulo:
            return out << "%";
        case Operator::NOr:
            return out << "!|";
        case Operator::BinNOr:
            return out << ".!|";
        case Operator::NAnd:
            return out << "!&";
        case Operator::BinNAnd:
            return out << ".!&";
        case Operator::Macro:
            return out << "#";
        case Operator::Not:
            return out << "!";
        case Operator::BinNot:
            return out << ".!";
        case Operator::Or:
            return out << "|";
        case Operator::BinOr:
            return out << ".|";
        case Operator::And:
            return out << "&";
        case Operator::BinAnd:
            return out << ".&";
        case Operator::XOr:
            return out << "^";
        case Operator::BinXOr:
            return out << ".^";
        case Operator::Imply:
            return out << "=>";
        case Operator::NImply:
            return out << "!=>";
        case Operator::BinImply:
            return out << ".=>";
        case Operator::BinNImply:
            return out << ".=>,";
        case Operator::Lesser:
            return out << "<";
        case Operator::Greater:
            return out << ">";
        case Operator::Equals:
            return out << "==";
        case Operator::LesserEqual:
            return out << "<=";
        case Operator::GreaterEqual:
            return out << ">=";
        case Operator::NotEqual:
            return out << "!=";
        case Operator::BracketOpen:
            return out << "(";
        case Operator::BracketClose:
            return out << ")";
        case Operator::BraceOpen:
            return out << "{";
        case Operator::BraceClose:
            return out << "}";
        case Operator::IndexOpen:
            return out << "[";
        case Operator::IndexClose:
            return out << "]";
        case Operator::Index:
            return out << "[]";
        case Operator::Increment:
            return out << "++";
        case Operator::Decrement:
            return out << "--";
        case Operator::ShiftRight:
            return out << ">>";
        case Operator::ShiftLeft:
            return out << "<<";
        case Operator::Bracket:
            return out << "()";
        case Operator::Brace:
            return out << "{}";
        case Operator::Address:
            return out << "address of";
        case Operator::Dereference:
            return out << "dereference";
        default:
            LexerError("Internal bug - invalid operator in printing!");
        return out;
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
                case Keyword::Break:
                    return out << "Keyword: break,";
                case Keyword::Continue:
                    return out << "Keyword: continue,";
                case Keyword::Switch:
                    return out << "Keyword: switch,";
                case Keyword::If:
                    return out << "Keyword: if,";
                case Keyword::While:
                    return out << "Keyword: while,";
                case Keyword::Else:
                    return out << "Keyword: else,";
                case Keyword::Case:
                    return out << "Keyword: case,";
                case Keyword::Do:
                    return out << "Keyword: do,";
                case Keyword::For:
                    return out << "Keyword: for,";
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
                case Operator::Bracket:
                    return out << "Operator: bracket,";
                case Operator::Brace:
                    return out << "Operator: brace,";
                case Operator::Address:
                    return out << "Operator: address of,";
                case Operator::Dereference:
                    return out << "Operator: dereference pointer,";
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

bool LexerToken::MatchOperator(const Operator::Type type) const {
    if (this->type == Token::Operator) {
        if (this->op == type) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchOperator(const Operator::Type type1, const Operator::Type type2) const {
    if (this->type == Token::Operator) {
        if (this->op == type1) {
            return true;
        }
        if (this->op == type2) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchOperator(const Operator::Type type1, const Operator::Type type2, const Operator::Type type3) const {
    if (this->type == Token::Operator) {
        if (this->op == type1) {
            return true;
        }
        if (this->op == type2) {
            return true;
        }
        if (this->op == type3) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchOperator(const Operator::Type type1, const Operator::Type type2, const Operator::Type type3, const Operator::Type type4) const {
    if (this->type == Token::Operator) {
        if (this->op == type1) {
            return true;
        }
        if (this->op == type2) {
            return true;
        }
        if (this->op == type3) {
            return true;
        }
        if (this->op == type4) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchOperator(const Operator::Type type1, const Operator::Type type2, const Operator::Type type3, const Operator::Type type4, const Operator::Type type5) const {
    if (this->type == Token::Operator) {
        if (this->op == type1) {
            return true;
        }
        if (this->op == type2) {
            return true;
        }
        if (this->op == type3) {
            return true;
        }
        if (this->op == type4) {
            return true;
        }
        if (this->op == type5) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchOperator(const Operator::Type type1, const Operator::Type type2, const Operator::Type type3, const Operator::Type type4, const Operator::Type type5, const Operator::Type type6) const {
    if (this->type == Token::Operator) {
        if (this->op == type1) {
            return true;
        }
        if (this->op == type2) {
            return true;
        }
        if (this->op == type3) {
            return true;
        }
        if (this->op == type4) {
            return true;
        }
        if (this->op == type5) {
            return true;
        }
        if (this->op == type6) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type1, const Keyword::KeywordType type2) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type1) {
            return true;
        }
        if (this->kw == type2) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type1, const Keyword::KeywordType type2, const Keyword::KeywordType type3) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type1) {
            return true;
        }
        if (this->kw == type2) {
            return true;
        }
        if (this->kw == type3) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type1, const Keyword::KeywordType type2, const Keyword::KeywordType type3, const Keyword::KeywordType type4) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type1) {
            return true;
        }
        if (this->kw == type2) {
            return true;
        }
        if (this->kw == type3) {
            return true;
        }
        if (this->kw == type4) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type1, const Keyword::KeywordType type2, const Keyword::KeywordType type3, const Keyword::KeywordType type4, const Keyword::KeywordType type5) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type1) {
            return true;
        }
        if (this->kw == type2) {
            return true;
        }
        if (this->kw == type3) {
            return true;
        }
        if (this->kw == type4) {
            return true;
        }
        if (this->kw == type5) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchKeyword (const Keyword::KeywordType type1, const Keyword::KeywordType type2, const Keyword::KeywordType type3, const Keyword::KeywordType type4, const Keyword::KeywordType type5, const Keyword::KeywordType type6) const {
    if (this->type == Token::Keyword) {
        if (this->kw == type1) {
            return true;
        }
        if (this->kw == type2) {
            return true;
        }
        if (this->kw == type3) {
            return true;
        }
        if (this->kw == type4) {
            return true;
        }
        if (this->kw == type5) {
            return true;
        }
        if (this->kw == type6) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchNumber (const Type::TypeEnum type) const {
    if (this->type == Token::Number) {
        if (this->literalType == type) {
            return true;
        }
    }
    return false;
}
bool LexerToken::MatchNumber (const Type::TypeEnum type1, const Type::TypeEnum type2) const {
    if (this->type == Token::Number) {
        if (this->literalType == type1) {
            return true;
        }
        if (this->literalType == type2) {
            return true;
        }
    }
    return false;
}