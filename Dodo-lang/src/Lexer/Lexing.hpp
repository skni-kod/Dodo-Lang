#ifndef LEXING_HPP
#define LEXING_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <generator>
#include "Misc/Options.hpp"

#include "TypeObject.hpp"


namespace Keyword {
    enum KeywordType {
        None, Primitive, TypeSI, TypeUI, TypeFP, Type, Void, Operator, Return, Import, End, After,
        Extern, Syscall, Public, Private, Protected, Let, Mut, Const, Comma, Dot, Member,
        Break, Continue, Switch, If, While, Else, Case, Do, For
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
        // enum order affects order of operations
        // lower code means being before the one with higher
        Address, Dereference,
        Not, BinNot,
        // first kinda like PEMDAS
        Increment, Decrement, Power, Multiply, Divide, Modulo, Add, Subtract,
        ShiftRight, ShiftLeft,
        // boolean order similar to C/C++ but expanded for more things
        NAnd, BinNAnd, And, BinAnd,
        XOr, BinXOr,
        NOr, BinNOr, Or, BinOr,
        NImply, Imply, BinNImply, BinImply,
        // assign and compare last so that they are evaluated after everything
        Assign, Lesser, Greater, Equals, LesserEqual, GreaterEqual, NotEqual,
        // these are special cases that do not use order of operations
        Macro, BracketOpen, BracketClose, Bracket, Brace,
        BraceOpen, BraceClose, IndexOpen, IndexClose, Index

    };
}

struct LexerToken {
    // TODO: make this enums
    uint8_t type = Token::Unknown;
    uint8_t literalType = Type::unsignedInteger;
    uint8_t isVerboseOperator = false;
    uint32_t characterNumber = 0;
    union {
        Operator::Type op;
        Keyword::KeywordType kw;
        uint64_t num = 0;
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
    // for putting in a value from somewhere with correct line number
    LexerToken(const LexerToken& value, uint32_t characterNumber);
    // for getting operator and keyword values from the map
    LexerToken(const std::string& key, uint32_t characterNumber);

    [[nodiscard]] bool MatchOperator(uint64_t type) const;
    [[nodiscard]] bool MatchKeyword (uint64_t type) const;
    [[nodiscard]] bool MatchNumber (uint64_t type) const;

    bool operator==(const LexerToken& other) const;

    LexerToken(LexerToken&& other) noexcept ;
    LexerToken(LexerToken& other);
    LexerToken& operator=(const LexerToken& other);
    LexerToken(const LexerToken& other);


    ~LexerToken();
};

std::ostream& operator<<(std::ostream& out, const LexerToken& token);

struct LexerLine {
    uint64_t lineNumber = 0;
    std::vector <LexerToken> tokens;
};

struct LexerFile {
    fs::path path;
    std::vector <LexerLine> lines;
};

std::vector <LexerFile> RunLexer();

class LexerException : public std::exception {
public:
    const char* what();
};

void LexerError(const std::string& message);

inline uint32_t currentCharacter = 0;
inline uint64_t currentLine = 0;
inline const fs::path* currentFile = nullptr;

#endif //LEXING_HPP
