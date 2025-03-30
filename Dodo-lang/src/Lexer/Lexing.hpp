#ifndef LEXING_HPP
#define LEXING_HPP

#include <cstdint>
#include <memory>
#include <string>
#include "LexingEnums.hpp"
#include "Options.hpp"

#include "TypeObject.hpp"


struct LexerToken {
#ifdef ENUM_VARIABLES
    Token::Type type = Token::Unknown;
    Type::TypeEnum literalType = Type::unsignedInteger;
#else
    uint8_t type = Token::Unknown;
    uint8_t literalType = Type::unsignedInteger;
#endif
    uint8_t isVerboseOperator = false;
    uint32_t characterNumber = 0;
    union {
        Operator::Type op;
        Keyword::KeywordType kw;
        uint64_t num = 0;
        uint64_t _unsigned;
        uint64_t unsigned64;
        uint32_t unsigned32;
        uint16_t unsigned16;
        uint8_t unsigned8;
        double _double;
        double float64;
        float float32;
        // no 16 bit implementation!
        int64_t _signed;
        int64_t signed64;
        int32_t signed32;
        int16_t signed16;
        int8_t signed8;
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

    [[nodiscard]] bool MatchOperator(Operator::Type type) const;
    [[nodiscard]] bool MatchOperator(Operator::Type type1, Operator::Type type2) const;
    [[nodiscard]] bool MatchOperator(Operator::Type type1, Operator::Type type2, Operator::Type type3) const;
    [[nodiscard]] bool MatchOperator(Operator::Type type1, Operator::Type type2, Operator::Type type3, Operator::Type type4) const;
    [[nodiscard]] bool MatchOperator(Operator::Type type1, Operator::Type type2, Operator::Type type3, Operator::Type type4, Operator::Type type5) const;
    [[nodiscard]] bool MatchOperator(Operator::Type type1, Operator::Type type2, Operator::Type type3, Operator::Type type4, Operator::Type type5, Operator::Type type6) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type1, Keyword::KeywordType type2) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type1, Keyword::KeywordType type2, Keyword::KeywordType type3) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type1, Keyword::KeywordType type2, Keyword::KeywordType type3, Keyword::KeywordType type4) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type1, Keyword::KeywordType type2, Keyword::KeywordType type3, Keyword::KeywordType type4, Keyword::KeywordType type5) const;
    [[nodiscard]] bool MatchKeyword (Keyword::KeywordType type1, Keyword::KeywordType type2, Keyword::KeywordType type3, Keyword::KeywordType type4, Keyword::KeywordType type5, Keyword::KeywordType type6) const;
    [[nodiscard]] bool MatchNumber (Type::TypeEnum type) const;
    [[nodiscard]] bool MatchNumber (Type::TypeEnum type1, Type::TypeEnum type2) const;

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
