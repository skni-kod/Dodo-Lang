#include "Lexing.hpp"

#include <iostream>
#include <fstream>
#include <queue>
#include <print>

#include "LexingInternal.hpp"
#include "Options.hpp"



std::queue <std::string> fileQueue;

bool IsSpecialCharacter(uint32_t code) {
    switch (code) {
        case '+':
        case '-':
        case '=':
        case '*':
        case '/':
        case '<':
        case '>':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case '&':
        case '^':
        case '%':
        case '#':
        case '!':
        case '|':
        case ':':
        case ';':
        case '.':
        case ',':
            return true;
        default:
            return false;
    }
}

// doesn't seem scalable, can't I make something better?
bool CanConstructFurther(const std::string& current) {
    switch (current.size()) {
        case 1:
            return true;
        case 2:
            // TODO: change this into switch case as it seems really slow
            if (current == ".!" or current == ".|" or current == ".&" or current == "!|" or current == "!&"
                or current == ".^"  or current == "=>" or current == ".="  or current == "!="  or current == ">>"
                or current == "<<"  or current == "=="  or current == "<="  or current == ">="  or current == "[]"
                or current == "++" or current == "--" or current == "//" or current == "*/" or current == "/*" or current == "::") {
                return true;
            }
            break;
        case 3:
            if (current == ".!=" or current == "!=>" or current == ".=>" or current == ".!|" or current == ".!&") {
                return true;
            }
            break;
        case 4:
            if (current == ".!=>") {
                return true;
            }
            break;
        default:
            break;
    }

    return false;
}

// checks if the pair is a double operator
bool DoubleOperator(const uint32_t first, const uint32_t second) {
    switch (first) {
        case Operator::Add:
        case Operator::Subtract:
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Power:
        case Operator::Modulo:
        case Operator::Not:
        case Operator::BinNot:
        case Operator::Or:
        case Operator::BinOr:
        case Operator::And:
        case Operator::BinAnd:
        case Operator::XOr:
        case Operator::BinXOr:
        case Operator::Imply:
        case Operator::BinImply:
        case Operator::NImply:
        case Operator::BinNImply:
        case Operator::ShiftRight:
        case Operator::ShiftLeft:
            if (second == Operator::Assign) {
                return true;
            }
            break;
        default:
            return false;
    }
    return false;
}

enum State {
    normal, string, character, singleComment, longComment, hexNumber, octNumber, binNumber, number, oper
};
uint8_t lexerState = State::normal;
bool doubleOperator = false;

LexerToken PushWrapper(const std::string& result, uint32_t characterNumber) {
    if (keywordsAndOperators.contains(result)) {
        return {keywordsAndOperators[result], characterNumber};
    }
    return {Token::Identifier, result, characterNumber};
}

LexerLine LexLine(std::string& line) {
    LexerLine output;
    // TODO: find a big/little endian friendly way for this
    struct Character {
        union {
            uint32_t code = 0;
            struct {
                // byte 1 is on the right actually as in 0x000000XX
                uint8_t byte1, byte2, byte3, byte4;
            };
            struct {
                char char1, char2, char3, char4;
            };
        };

        std::string toString() {
            if (byte2 != 0) {
                if (byte3 != 0) {
                    if (byte4 != 0) {
                        return {char4, char3, char2, char1};
                    }
                    return {char3, char2, char1};
                }
                return {char2, char1};
            }
            return {char1};
        }

        uint8_t isEscaped = false;
        uint8_t specialWhitespace = false;
        uint16_t whitespaceBefore = 0;
    };

    // getting a vector of characters after decoding the encoding
    std::vector <Character> characters;
    characters.reserve(line.length());
    uint16_t whitespaceBefore = 0;
    currentCharacter = 1;
    for (uint32_t n = 0; n < line.length(); currentCharacter++) {
        Character current;
        if (line[n] == '\\') {
            current.isEscaped = true;
            n++;
            if (n == line.length()) {
                LexerError(R"(Expected a character after '\'!)");
            }
        }

        // UTF-8 support, could add other encodings here too
        if ((line[n] & 0b10000000) != 0) {
            if ((line[n] & 0b11100000) == 0b11000000) {
                // 2 byte
                if (n > line.length() - 2) {
                    LexerError("Unfinished UTF-8 character!");
                }
                current.byte1 = line[n + 1];
                current.byte2 = line[n];
                n += 2;
            }
            else if ((line[n] & 0b11110000) == 0b11100000) {
                // 3 byte
                if (n > line.length() - 3) {
                    LexerError("Unfinished UTF-8 character!");
                }
                current.byte3 = line[n];
                current.byte2 = line[n + 1];
                current.byte1 = line[n + 2];
                n += 3;
            }
            else if ((line[n] & 0b11111000) == 0b11110000) {
                // 4 byte
                if (n > line.length() - 4) {
                    LexerError("Unfinished UTF-8 character!");
                }
                current.byte4 = line[n];
                current.byte3 = line[n + 1];
                current.byte2 = line[n + 2];
                current.byte1 = line[n + 3];
                n += 4;
            }
            else {
                LexerError("Invalid UTF-8 character!");
            }
        }
        else {
            // 1 byte
            current.code = static_cast <unsigned char>(line[n]);
            n += 1;
        }



        if (IsSpecialCharacter(current.code)) {
            current.specialWhitespace = true;
        }

        // skipping Unicode whitespace characters
        // look at the first result of googling utf8 whitespace characters list on stackoverflow
        switch (current.code) {
            case 0x0009:
            case 0x000A:
            case 0x000B:
            case 0x000C:
            case 0x000D:
            case 0x0020:
            case 0x0085:
            case 0x00A0:
            case 0x1680:
            case 0x180E:
            case 0x2000:
            case 0x2001:
            case 0x2002:
            case 0x2003:
            case 0x2004:
            case 0x2005:
            case 0x2006:
            case 0x2007:
            case 0x2008:
            case 0x2009:
            case 0x200A:
            case 0x200B:
            case 0x200C:
            case 0x200D:
            case 0x2028:
            case 0x2029:
            case 0x202F:
            case 0x205F:
            case 0x2060:
            case 0x3000:
            case 0xFEFF:
                whitespaceBefore++;
                continue;
            default:
                current.whitespaceBefore = whitespaceBefore;
                whitespaceBefore = 0;
                characters.push_back(current);
                break;
        }
    }

    if (characters.empty()) {
        return {};
    }

    if (Options::informationLevel > Options::InformationLevel::general) {
        std::cout << "INFO L3: Line: " << currentLine << " after character splitting:\nINFO L3: ";
        for (auto& n : characters) {
            std::cout << std::string(n.whitespaceBefore, ' ');
            std::cout << n.toString();
        }
        std::cout << "\n";
    }

    // removing whitespace from first character to use later
    characters[0].whitespaceBefore = false;


    // now a go through the characters without context to create something
    std::string result;
    bool isNegative = false;
    bool hasDot = false;
    bool hasFloatSign = false;
    uint8_t base = 0;

    for (int64_t n = 0; n < characters.size(); n++) {
        auto& current = characters[n];
        switch (lexerState) {
            case State::normal:
                // normal state of the lexer
                if ((current.whitespaceBefore or current.specialWhitespace) and not result.empty()) {
                    output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                    result.clear();
                    if (output.tokens.back().type == Token::Operator) {
                        n -= 2;
                        lexerState = State::oper;
                        continue;
                    }
                }
                if (current.specialWhitespace) {
                    lexerState = oper;
                    n--;
                    continue;
                }
                if (current.code == '"') {
                    lexerState = string;
                    if (not result.empty()) {
                        output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                        result.clear();
                    }
                    continue;
                }
                if (current.code == '\'') {
                    lexerState = State::character;
                    if (not result.empty()) {
                        output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                        result.clear();
                    }
                    continue;
                }
                if (current.code >= '0' and current.code <= '9' and result.empty()) {
                    lexerState = State::number;
                    if (not result.empty()) {
                        output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                        result.clear();
                    }
                    n--;
                    isNegative = false;
                    hasDot = false;
                    hasFloatSign = false;
                    base = 0;
                    continue;
                }

                result += current.toString();
                if (n == characters.size() - 1) {
                    output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                }
                break;
            case State::oper:
                if (IsSpecialCharacter(current.code) and CanConstructFurther(result + current.toString())) {
                    result += current.toString();
                    if (n == characters.size() - 1) {
                        if (doubleOperator and result == ";") {
                            doubleOperator = false;
                            output.tokens.emplace_back(Token::Operator, static_cast <uint64_t>(Operator::BracketClose), 0);
                        }
                        if (result == "//") {
                            lexerState = State::singleComment;
                            continue;
                        }
                        if (result == "/*") {
                            lexerState = State::longComment;
                            continue;
                        }
                        output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                        result.clear();
                        lexerState = normal;
                    }
                }
                else if (not result.empty()) {
                    if (result == "//") {
                        lexerState = State::singleComment;
                        continue;
                    }
                    if (result == "/*") {
                        lexerState = State::longComment;
                        continue;
                    }
                    if (doubleOperator and result == ";") {
                        doubleOperator = false;
                        output.tokens.emplace_back(Token::Operator, static_cast <uint64_t>(Operator::BracketClose), 0);
                    }
                    if ((result[0] == '.' or result[0] == '-') and result.size() == 1 and current.code >= '0' and current.code <= '9') {
                        n -= 2;
                        lexerState = State::number;
                        result.clear();
                        isNegative = false;
                        hasDot = false;
                        hasFloatSign = false;
                        base = 0;
                        continue;
                    }
                    output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                    // minimum index here is 1
                    n--;
                    result.clear();
                    lexerState = State::normal;
                }
                else {
                    lexerState = normal;
                }
                // if this ended with a double operator
                if (result.empty() and output.tokens.size() >= 3 and output.tokens.back().type == Token::Operator
                    and output.tokens[output.tokens.size() - 2].type == Token::Operator
                    and DoubleOperator(output.tokens[output.tokens.size() - 2].op, output.tokens.back().op)) {

                    auto operation = output.tokens[output.tokens.size() - 2];
                    output.tokens.erase(output.tokens.size() - 2 + output.tokens.begin());
                    if (doubleOperator) {
                        LexerError("Use of another double operator before ending last one's area");
                    }
                    doubleOperator = true;
                    output.tokens.emplace_back(output.tokens[output.tokens.size() - 2]);
                    output.tokens.emplace_back(std::move(operation));
                    output.tokens.emplace_back(Token::Operator, static_cast <uint64_t>(Operator::BracketOpen), 0);
                }
                break;
            case State::string:
                result += std::string(current.whitespaceBefore, ' ');
                if (current.code == '"' and not current.isEscaped) {
                    lexerState = State::normal;
                    output.tokens.emplace_back(Token::String, result, n - result.size());
                    result.clear();
                }
                else {
                    if (current.isEscaped) {
                        switch (current.code) {
                            case 'a':
                                result += '\a';
                            break;
                            case 'b':
                                result += '\b';
                            break;
                            case 't':
                                result += '\t';
                            break;
                            case 'n':
                                result += '\n';
                            break;
                            case 'v':
                                result += '\v';
                            break;
                            case 'f':
                                result += '\f';
                            break;
                            case 'r':
                                result += '\r';
                            break;
                            case 'e':
                                result += 0x1b;
                            break;
                            case '0':
                                result += '\0';
                            break;
                            case '"':
                                result += '"';
                            break;
                            default:
                                LexerError("Invalid escaped character in string!");
                            break;
                        }
                    }
                    else {
                        result += current.toString();
                    }

                }
                break;
            case State::character:
                if (current.code == '\'') {
                    if (result.empty()) {
                        output.tokens.emplace_back(Token::Number, static_cast <uint64_t>(0), n);
                    }
                    else {
                        output.tokens.emplace_back(Token::Number, static_cast <uint64_t>(characters[n - 1].code), n);
                    }
                    lexerState = normal;
                    result.clear();
                }
                else if (not result.empty()) {
                    LexerError("A character definition can only contain 1 character!");
                }
                else {
                    // no value is actually needed
                    result += ' ';
                }
                break;
            case State::singleComment:
                break;
            case State::longComment:
                if (not result.empty()) {
                    if (current.code == '/') {
                        lexerState = normal;
                    }
                    result.clear();
                }
                else if (current.code == '*') {
                    result += '*';
                }
                break;
            case State::number: {
                bool parse = false;
                if ((current.whitespaceBefore > 0 and not result.empty())
                    or (current.specialWhitespace and current.code != '.' and current.code != '-')) {
                    parse = true;
                }

                if (not parse) {
                    switch (current.code) {
                        case 'A':
                            result += 'a';
                        break;
                        case 'B':
                            result += 'b';
                        break;
                        case 'C':
                            result += 'c';
                        break;
                        case 'D':
                            result += 'd';
                        break;
                        case 'E':
                            result += 'e';
                        break;
                        case 'F':
                            result += 'f';
                        break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            result += current.toString();
                        break;
                        case '-':
                            if (not result.empty()) {
                                if (result.size() == 1 and result[0] == '-') {
                                    // double negative negates itself, let's hope it's correct in every case
                                    result.clear();
                                }
                                else {
                                    if (result.size() > isNegative) {
                                        // will try to parse the number, if it wasn't correct it will throw a nice error
                                        parse = true;
                                        n--;
                                    }
                                }
                            }
                            isNegative = true;
                        result += current.toString();
                        break;
                        case '.':
                            if (hasDot) {
                                LexerError("Double dotted numbers are not allowed!");
                            }
                            hasDot = true;
                            result += current.toString();
                            break;
                        case 'f':
                            if (base != 10) {
                                result += current.toString();
                            }
                            else {
                                if (hasFloatSign) {
                                    LexerError("Only one float sign is allowed!");
                                }
                                hasFloatSign = true;
                                result += current.toString();
                            }

                        break;
                        case 'b':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                            base = 2;
                            result.pop_back();
                            break;
                        case 't':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 3;
                        result.pop_back();
                        break;
                        case 'q':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 4;
                        result.pop_back();
                        break;
                        case 'p':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 5;
                        result.pop_back();
                        break;
                        case 'h':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 6;
                        result.pop_back();
                        break;
                        case 's':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 7;
                        result.pop_back();
                        break;
                        case 'o':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 8;
                        result.pop_back();
                        break;
                        case 'n':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 9;
                        result.pop_back();
                        break;
                        case 'd':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 10;
                        result.pop_back();
                        break;
                        case 'e':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 11;
                        result.pop_back();
                        break;
                        case 'w':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 12;
                        result.pop_back();
                        break;
                        case 'x':
                            if (result.size() != 1 + isNegative or base != 0 or result[isNegative] != '0') {
                                LexerError("Number base specifiers are only allowed at second position!");
                            }
                        base = 16;
                        result.pop_back();
                        break;
                        default:
                            LexerError("Invalid character in number!");
                    }
                }

                if (parse or n == characters.size() - 1) {
                    if (base == 0) {
                        base = 10;
                    }
                    if (base != 10 and (hasDot or hasFloatSign)) {
                        LexerError("Only base 10 floats supported!");
                    }

                    if (hasFloatSign) {
                        if (result.back() != 'f') {
                            LexerError("Float sign can only be at the end!");
                        }
                        result.pop_back();
                    }

                    // TODO: what if a signed integer is needed where an unsigned one is parsed?
                    if (hasDot or (hasFloatSign and base == 10)) {
                        output.tokens.emplace_back(Token::Number, std::strtod(result.c_str(), nullptr), n - result.length() - hasFloatSign + 1);
                    }
                    else if (isNegative) {
                        output.tokens.emplace_back(Token::Number, static_cast <int64_t>(std::strtoll(result.c_str(), nullptr, base)), n - result.length());
                    }
                    else {
                        output.tokens.emplace_back(Token::Number, static_cast <uint64_t>(std::strtoull(result.c_str(), nullptr, base)), n - result.length());
                    }


                    result.clear();
                    lexerState = normal;
                    if (current.specialWhitespace and current.code != '.' and current.code != '-') {
                        n--;
                    }
                }

            }
                break;
            default:
                LexerError("Internal bug - invalid lexer token reconstruction state!");
        }
    }

    if (lexerState == singleComment) {
        lexerState = normal;
    }

    if (output.tokens.size() == 3
        and output.tokens[0].type == Token::Keyword and output.tokens[0].kw == Keyword::Import
        and output.tokens[1].type == Token::String
        and output.tokens[2].type == Token::Keyword and output.tokens[2].kw == Keyword::End) {

        fileQueue.push(*output.tokens[1].text);
        output.tokens.clear();
    }

    if (Options::informationLevel > Options::InformationLevel::general) {
        std::cout << "INFO L3: After reconstruction:\nINFO L3: ";
        for (const auto& n : output.tokens) {
            std::cout << n << " ";
        }
        std::cout << "\n";
    }
    return output;
}

LexerFile LexFile(const std::string& filePath) {
    LexerFile output;
    output.path = filePath;
    currentFile = &output.path;

    auto input = std::ifstream(filePath);
    if (not input.is_open()) {
        LexerError("Cannot open file: \"" + filePath + "\"!");
    }

    uint64_t lineNumer = 1;
    for (std::string line; std::getline(input, line); currentLine = ++lineNumer) {
        auto lexedLine = LexLine(line);
        if (not lexedLine.tokens.empty()) {
            lexedLine.lineNumber = lineNumer;
            output.lines.emplace_back(std::move(lexedLine));
        }
    }

    return output;
}

std::unordered_map<std::string, uint64_t> lexedFiles;

std::vector <LexerFile> RunLexer(const std::string& startFile) {
    std::vector<LexerFile> output;
    fileQueue.push(startFile);
    while (not fileQueue.empty()) {
        if (not lexedFiles.contains(fileQueue.front())) {
            output.emplace_back(LexFile(fileQueue.front()));
            lexedFiles.emplace(fileQueue.front(), output.size());
        }
        fileQueue.pop();
    }

    return output;
}