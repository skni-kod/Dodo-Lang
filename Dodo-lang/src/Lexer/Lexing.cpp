#include "Lexing.hpp"

#include <cstring>
#include <iostream>
#include <fstream>
#include <queue>

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
    switch (current[0]) {
        case ';':
        case ':':
        case ',':
        case '(':
        case ')':
        case '{':
        case '}':
            return false;
        case '+':
        case '-':
            if (current.size() == 1) {
                return true;
            }
        return false;
        case '.':
            if (current.size() == 1) {
                return true;
            }
            if (current[1] >= '0' and current[1] <= '9') {
                return false;
            }
            // TODO: add binary things here
            return false;
        default:
            return false;
    }
    return false;
}

// checks if the pair is a double operator
bool DoubleOperator(uint32_t first, uint32_t second) {
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
            if (second == Operator::Equals) {
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

LexerLine LexLine(std::string& line) {
    LexerLine output;
    // TODO: find a big/little endian friendly way for this
    struct Character {
        union {
            uint32_t code = 0;
            struct {
                // byte 1 is on the right actually as in 0x000000XX
                char byte1, byte2, byte3, byte4;
            };
            struct {
                char char1, char2, char3, char4;
            };
        };

        std::string toString() {
            if (byte2 > 0) {
                if (byte3 > 0) {
                    if (byte4 > 0) {
                        return {char4, char3, char2, char1};
                    }
                    return {char3, char2, char1};
                }
                return {char2, char1};
            }
            return {char1};
        }

        uint8_t isEscaped = false;
        uint8_t whitespaceBefore = false;
    };

    // getting a vector of characters after decoding the encoding
    std::vector <Character> characters;
    characters.reserve(line.length());
    bool whitespaceBefore = false;
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
                current.byte2 = line[n];
                current.byte1 = line[n] + 1;
                n += 2;
            }
            else if ((line[n] & 0b11110000) == 0b11100000) {
                // 3 byte
                if (n > line.length() - 3) {
                    LexerError("Unfinished UTF-8 character!");
                }
                current.byte3 = line[n];
                current.byte2 = line[n] + 1;
                current.byte1 = line[n] + 2;
                n += 3;
            }
            else if ((line[n] & 0b11111000) == 0b11110000) {
                // 4 byte
                if (n > line.length() - 4) {
                    LexerError("Unfinished UTF-8 character!");
                }
                current.byte4 = line[n];
                current.byte3 = line[n] + 1;
                current.byte2 = line[n] + 2;
                current.byte1 = line[n] + 3;
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

        current.whitespaceBefore = whitespaceBefore;
        whitespaceBefore = false;

        if (IsSpecialCharacter(current.code)) {
            current.whitespaceBefore = true;
            whitespaceBefore = true;
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
                whitespaceBefore = true;
                continue;
            default:
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
            if (n.whitespaceBefore) {
                std::cout << " ";
            }
            std::cout << n.toString();
        }
        std::cout << "\n";
    }

    // removing whitespace from first character to use later
    characters[0].whitespaceBefore = false;


    // now a go through the characters without context to create something
    std::string result;
    std::vector <std::string> results;

    uint32_t stateSeq = 0;

    // TODO: make this work on tokens vector and do a lot of thinking current
    for (uint32_t n = 0; n < characters.size(); n++) {
        auto& current = characters[n];
        switch (lexerState) {
            case State::normal:
                // normal state of the lexer
                if (current.whitespaceBefore) {
                    results.push_back(result);
                    result.clear();
                }
                if (IsSpecialCharacter(current.code)) {
                    lexerState = oper;
                }

                result += current.toString();
                break;
            case State::oper:
                if (CanConstructFurther(result + current.toString())) {

                }
                else {
                    // minimum index here is 1
                    n--;
                    results.push_back(result);
                    lexerState = State::normal;
                }
                break;
            case State::string:

                break;
            case State::character:

                break;
            case State::singleComment:
                break;
            case State::longComment:

                break;
            case State::hexNumber:

                break;
            case State::octNumber:

                break;
            case State::binNumber:

                break;
            default:
                LexerError("Internal bug - invalid lexer token reconstruction state!");
        }
    }

    if (lexerState != longComment) {
        lexerState = normal;
    }

    if (Options::informationLevel > Options::InformationLevel::general) {
        std::cout << "INFO L3: After initial construction:\nINFO L3: ";
        for (auto& n : results) {
            std::cout << "\"" << n << "\" ";
        }
        std::cout << "\n";
    }
    return output;
}

LexerFile LexFile(const std::string& filePath) {
    LexerFile output;
    output.path = filePath;

    auto input = std::ifstream(filePath);
    if (not input.is_open()) {
        LexerError("Cannot open file: \"" + filePath + "\"!");
    }

    uint64_t lineNumer = 1;
    for (std::string line; std::getline(input, line); lineNumer++) {
        currentLine = lineNumer;
        auto lexedLine = LexLine(line);
        if (not lexedLine.tokens.empty()) {
            lexedLine.lineNumber = lineNumer;
            output.lines.push_back(lexedLine);
        }
    }

    return output;
}

std::vector <LexerFile> RunLexer(const std::string& startFile) {
    std::vector<LexerFile> output;
    fileQueue.push(startFile);
    while (not fileQueue.empty()) {
        output.emplace_back(LexFile(fileQueue.front()));
        fileQueue.pop();
    }

    return output;
}

const char* LexerException::what() {
    return "Lexer has encountered unexpected input";
}

void LexerError(const std::string& message) {
    std::cout << "ERROR! In file: " << (currentFile != nullptr ? '"' + *currentFile + '"': "unknown")
    << ", at line: " << currentLine << ", character: " << currentCharacter << "\nMessage:" << message << "\n";
    throw LexerException();
}

LexerToken::~LexerToken() {
    if ((type == Token::Identifier or type == Token::String) and text != nullptr) {
        free(text);
    }
}

LexerToken::LexerToken(const uint8_t type, uint64_t value, const uint32_t characterNumber, uint8_t isVerboseOperator) {
    this->type = type;
    this->_unsigned = type;
    this->characterNumber = characterNumber;
    this->isVerboseOperator = isVerboseOperator;
}

LexerToken::LexerToken(const uint8_t type, int64_t value, const uint32_t characterNumber) {
    this->type = type;
    this->_signed = type;
    this->characterNumber = characterNumber;
}

LexerToken::LexerToken(const uint8_t type, double value, const uint32_t characterNumber) {
    this->type = type;
    this->_double = type;
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