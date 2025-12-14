#ifndef LEXING_HPP
#define LEXING_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <concepts>

#include "ErrorHandling.hpp"
#include "LexingEnums.hpp"
#include "Options.hpp"

#include "TypeObject.hpp"


template<typename T>
concept TokenEnum = (std::same_as<T, Operator::Type>
                  or std::same_as<T, Keyword::KeywordType>
                  or std::same_as<T, Type::TypeEnum>
                  or std::same_as<T, Token::Type>
                  );

struct LexerToken {
#ifdef PACKED_ENUM_VARIABLES
    Token::Type type : 4 = Token::Unknown;
    Type::TypeEnum literalType : 3 = Type::unsignedInteger;
#else
    uint8_t type : 4 = Token::Unknown;
    uint8_t literalType : 3 = Type::unsignedInteger;
#endif
    uint8_t isVerboseOperator : 1 = false;
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

    /// <summary>
    /// Checks if the token matches the given token enum
    /// </summary>
    /// <param name="matched">One of allowed types enums to match against</param>
    /// <returns>True if matches</returns>
    template <TokenEnum T>
    [[nodiscard]] bool is(T matched) const {
        if constexpr (std::same_as<T, Operator::Type>)
            return this->type == Token::Operator and this->op == matched;
        if constexpr (std::same_as<T, Keyword::KeywordType>)
            return this->type == Token::Keyword and this->kw == matched;
        if constexpr (std::same_as<T, Type::TypeEnum>)
            return this->type == Token::Number and this->literalType == matched;
        if constexpr (std::same_as<T, Token::Type>)
            return this->type == matched;
        Unimplemented();
    }

    /// <summary>
    /// Checks if any of provided token enums can be matched to the token
    /// </summary>
    /// <param name="matched">Variable amount of parameters to check</param>
    /// <returns>True if any match</returns>
    template <TokenEnum... Ts>
    [[nodiscard]] bool anyOf (Ts... matched) const {
        return false or (is(matched) or ...);
    }

    bool operator==(const LexerToken& other) const;

    LexerToken(LexerToken&& other) noexcept ;
    LexerToken(LexerToken& other);
    LexerToken& operator=(const LexerToken& other);
    LexerToken(const LexerToken& other);


    ~LexerToken();
};

std::ostream& PrintOperatorSymbol(Operator::Type op, std::ostream& out);

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
void ReEscapeCharacters(std::string* text);

inline uint32_t currentCharacter = 0;
inline uint64_t currentLine = 0;
inline const fs::path* currentFile = nullptr;

#endif //LEXING_HPP
