#include "Lexing.hpp"

#include <iostream>
#include <fstream>
#include <queue>
#include <print>

#include "LexingInternal.hpp"
#include "Options.hpp"

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
        case '~':
            return true;
        default:
            return false;
    }
}

void ReEscapeCharacters(std::string* text) {
    // ALWAYS also change the one below
    for (uint64_t n = 0; n < text->size(); n++) {
        switch ((*text)[n]) {
            case '\a':
                *text = text->substr(0, n) + "\\a" + text->substr(n + 1);
                break;
            case '\b':
                *text = text->substr(0, n) + "\\b" + text->substr(n + 1);
                break;
            case '\t':
                *text = text->substr(0, n) + "\\t" + text->substr(n + 1);
                break;
            case '\n':
                *text = text->substr(0, n) + "\\n" + text->substr(n + 1);
                break;
            case '\v':
                *text = text->substr(0, n) + "\\v" + text->substr(n + 1);
                break;
            case '\f':
                *text = text->substr(0, n) + "\\f" + text->substr(n + 1);
                break;
            case '\r':
                *text = text->substr(0, n) + "\\r" + text->substr(n + 1);
                break;
            case '\e':
                *text = text->substr(0, n) + "\\e" + text->substr(n + 1);
                break;
            case '\0':
                *text = text->substr(0, n) + "\\0" + text->substr(n + 1);
                break;
            case '\"':
                *text = text->substr(0, n) + "\\\"" + text->substr(n + 1);
                break;
            default:
                break;
        }
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
                or current == "++" or current == "--" or current == "//" or current == "*/" or current == "/*"
                or current == "::" or current == "()" or current == "{}") {
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


LexerToken PushWrapper(const std::string& result, uint32_t characterNumber) {
    if (keywordsAndOperators.contains(result)) {
        return {keywordsAndOperators[result], characterNumber};
    }
    if (result == "false") {
        return {Token::Number, 0, characterNumber, true};
    }
    if (result == "true") {
        return {Token::Number, 1, characterNumber, true};
    }
    return {Token::Identifier, result, characterNumber};
}

// state variables
uint8_t lexerState = State::normal;
bool doubleOperator = false;
uint32_t doubleOperatorBracket = 0;
std::string result;
bool isNegative = false;
bool hasDot = false;
bool hasFloatSign = false;
uint8_t base = 0;

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
                //if (n == characters.size() - 1) {
                //    output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                //}
                break;
            case State::oper:
                if (result == "/") {
                    if (current.code == '/') {
                        lexerState = State::singleComment;
                        result.clear();
                        continue;
                    }
                    if (current.code == '*') {
                        lexerState = State::longComment;
                        result.clear();
                        continue;
                    }
                }

                if (IsSpecialCharacter(current.code) and CanConstructFurther(result + current.toString())) {
                    result += current.toString();
                    if (n == characters.size() - 1) {
                        if (doubleOperator and (result == ";" or (result == ")" and doubleOperatorBracket == 0))) {
                            doubleOperator = false;
                            output.tokens.emplace_back(Token::Operator, static_cast <uint64_t>(Operator::BracketClose), 0);
                        }
                        if ((output.tokens.empty()
                                or (not output.tokens.empty() and not output.tokens.front().MatchKeyword(Keyword::Operator)))
                                and (result == "()" or result == "{}" or result == "[]")){

                            output.tokens.emplace_back(PushWrapper({result[0]}, n - result.size()));
                            result.erase(result.begin());
                                }
                        output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                        if (result == "(") {
                            doubleOperatorBracket++;
                        }
                        else if (result == ")") {
                            doubleOperatorBracket--;
                        }
                        result.clear();
                        lexerState = normal;
                    }
                }
                else if (not result.empty()) {
                    if (doubleOperator and (result == ";" or (result == ")" and doubleOperatorBracket == 0))) {
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
                    if ((output.tokens.empty()
                        or (not output.tokens.empty() and not output.tokens.front().MatchKeyword(Keyword::Operator)))
                        and (result == "()" or result == "{}" or result == "[]")){

                        output.tokens.emplace_back(PushWrapper({result[0]}, n - result.size()));
                        result.erase(result.begin());
                    }
                    output.tokens.emplace_back(PushWrapper(result, n - result.size() + 1));
                    // minimum index here is 1
                    n--;
                    if (result == "(") {
                        doubleOperatorBracket++;
                    }
                    else if (result == ")") {
                        doubleOperatorBracket--;
                    }
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
                    doubleOperatorBracket = 0;
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
                        // ALWAYS update above
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
                                result += '\x1b';
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
                    n--;
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
                        case '_':
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
                    // if (current.specialWhitespace and current.code != '.' and current.code != '-') {
                    //     n--;
                    // }
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
        and output.tokens[0].MatchKeyword(Keyword::Import)
        and output.tokens[1].type == Token::String
        and output.tokens[2].MatchKeyword(Keyword::End)) {
        if (auto pos = output.tokens[1].text->find_last_of('/'); pos == std::string::npos) {
            Options::inputFiles.emplace(*output.tokens[1].text);
        }
        else {
            std::cout << "Warning: only file names are supported in import, the rest of the path: \"" << *output.tokens[1].text << "\" was ignored!\n";
            Options::inputFiles.emplace(output.tokens[1].text->substr(pos + 1));
        }

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

LexerFile LexFile(const fs::path& fileName) {
    LexerFile output;
    // finding the file
    for (auto& n : Options::importDirectories) {
        for (auto& m : fs::recursive_directory_iterator(n)) {
            if (m.is_regular_file() and m.path().filename() == fileName.filename()) {
                output.path = fs::canonical(m);
                currentFile = &output.path;
                break;
            }
        }
        if (output.path != "") {
            break;
        }
    }

    if (output.path == "") {
        LexerError("Cannot find file: \"" + fileName.string() + "\"!");
    }

    auto input = std::ifstream(absolute(output.path));
    if (not input.is_open()) {
        LexerError("Cannot open file: \"" + fileName.string() + "\"!");
    }

    uint64_t lineNumber = 1;
    for (std::string line; std::getline(input, line); currentLine = ++lineNumber) {
        if (auto lexedLine = LexLine(line); not lexedLine.tokens.empty()) {
            lexedLine.lineNumber = lineNumber;
            output.lines.emplace_back(std::move(lexedLine));
        }
    }

    return output;
}

std::unordered_map<std::string, uint64_t> lexedFiles;

std::vector <LexerFile> RunLexer() {
    std::vector<LexerFile> output;
    while (not Options::inputFiles.empty()) {
        if (not lexedFiles.contains(Options::inputFiles.front().filename())) {
            output.emplace_back(LexFile(Options::inputFiles.front()));
            lexedFiles.emplace(Options::inputFiles.front().filename(), output.size());
        }
        Options::inputFiles.pop();
    }

    return output;
}