#ifndef LEXING_HPP
#define LEXING_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "TypeObject.hpp"


namespace Keyword {
    enum Type {
        Primitive, TypeSI, TypeUI, TypeFP, Type, Void, Operator, Return, Import, End, After,
        Extern, Syscall, Public, Private, Protected, Let, Mut, Const, Comma
    };
}

struct LexerToken;

namespace Token {
    enum Type {
        Identifier, Operator, Number, String, Keyword, Unknown
    };
}

namespace Operator {
    enum Type {
        Assign, Add, Subtract, Multiply, Divide, Power, Modulo, NOr, BinNOr,
        Macro, Not, BinNot, Or, BinOr, And, BinAnd, XOr, BinXOr, Imply, NImply,
        BinImply, BinNImply, Lesser, Greater, Equals, LesserEqual,
        GreaterEqual, NotEqual, BracketOpen, BracketClose,
        BraceOpen, BraceClose, IndexOpen, IndexClose, Index,
        Increment, Decrement, ShiftRight, ShiftLeft
    };
}

struct LexerToken {
    uint8_t type = Token::Unknown;
    uint8_t literalType = Type::unsignedInteger;
    uint8_t isVerboseOperator = false;
    uint32_t characterNumber = 0;
    union {
        uint64_t op = 0;
        uint64_t kw;
        uint64_t num;
        uint64_t _unsigned;
        double _double;
        int64_t _signed;
        std::string* text;
        std::string* string;
    };
    LexerToken() = default;
    // for operands, keywords ad unsigned values
    LexerToken(uint8_t type, uint64_t value, uint32_t characterNumber, uint8_t isVerboseOperator = false);
    // for signed integer values
    LexerToken(uint8_t type, int64_t value, uint32_t characterNumber);
    // for floating point values
    LexerToken(uint8_t type, double value, uint32_t characterNumber);
    // for identifier and string field values
    LexerToken(uint8_t type, const std::string& text, uint32_t characterNumber);
    // for getting operator and keyword values from the map
    LexerToken(const std::string& key, uint32_t characterNumber);


    ~LexerToken();
};

struct LexerLine {
    uint64_t lineNumber = 0;
    std::vector <LexerToken> tokens;
};

struct LexerFile {
    std::string path;
    std::vector <LexerLine> lines;
};

std::vector <LexerFile> RunLexer(const std::string& startFile);

class LexerException : public std::exception {
public:
    const char* what();
};

void LexerError(const std::string& message);

inline uint32_t currentCharacter = 0;
inline uint64_t currentLine = 0;
inline const std::string* currentFile = nullptr;

#endif //LEXING_HPP
