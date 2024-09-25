#pragma once

#include <iostream>
#include <vector>
#include <ostream>

// TODO: Rewrite to make this comfortable to use

/*
ABOUT THIS FILE:
it contains two classes:
- LexicalToken,
- ProgramLine
----------------------------------------------------------------------------------------------------------

ABOUT LexicalToken:
- this is simple objects - it has type and value
- all types are in tokenType enum
- value is string that contains the name of token like "for" or "while" or "variableName"
- every word in program will be change into token (exepts of commnets)
- have variable literal - it contains information about type of literal such as char, string, float, numeric and hex,
this types are not the real program types, they are used for checking if the format is correct (numeric can means i32, long etc),
type none is using when this is not literal types
- unexpected type means that there is probably mistake in literal format, but it is left to parser to decide
----------------------------------------------------------------------------------------------------------

ABOUT ProgramLine:
- have number of line
- contains all tokens from one line of program file
- contains number of tokens in line
exampe:
code below is treated as two diffrent lexer lines

i32 a = 2
+ 4;

About ProgramPage:
- contains all info about one included file
#imp "library" - that instruction will be adding another local file
*/


class LexicalToken {
public:

    enum Type {
        keyword,
        operand,
        identifier,
        comma,
        expressionEnd,
        blockBegin,
        blockEnd,
        literal,
        unexpected,
        fileBegin
    };

    //types are described at the end of that file
    uint8_t type;
    int literalValue;

    //names of token types - use for display
    static std::string names[11];

    //names of literal types - use for display
    static std::string lnames[6];

    //literal type is set to -1
    LexicalToken(int type, std::string value);

    //allows to set literal type
    LexicalToken(int type, std::string value, int literal_type);

    //friends
    friend std::ostream& operator<<(std::ostream& os, const LexicalToken& dt);

    //getters
    int get_ltype();

    int get_type();

    std::string get_value();

    //value
    std::string value;
};

class ProgramLine {
public:

    //vector of all tokens in one line
    std::vector<LexicalToken> line;

    //number of line -
    int line_number = 0;

    int l_size = 0;

    // function to print line with all info
    //displays also types of numeric
    void line_print() const;

    //overriding << operator
    friend std::ostream& operator<<(std::ostream& os, const ProgramLine& l);

    //add token when it's not literal
    void add_token(int type, std::string value);

    //add literal token with literal - uses when adding strings and chars
    void add_token(int type, std::string value, int literal_type);

    //add literal token with numeric type check
    void add_token(int type, std::string value, bool checkNumeric);


    //operators
    LexicalToken& operator[](int);

};

//this conteins all info about one file
class ProgramPage {

public:
    //tells how many lines are inside
    int p_size;

    //tells the name of the file
    std::string file_name;

    //list of all lines inside of one page
    std::vector<ProgramLine> page;

    //add line to vector
    void add_line(ProgramLine p_line);

    friend std::ostream& operator<<(std::ostream& os, const ProgramPage& p);

    //operators
    ProgramLine& operator[](int);

};

//it contains all token types
enum tokenType {
    keyword = 0, //part of the language
    operand = 1,
    identifier = 2, //variable names, functions names, object names etc
    comma = 3,
    endline = 4, //;
    blockBegin = 5, //{
    blockEnd = 6, //}
    literal = 7, //number, string, char, bool, etc.
    unexpeted_type = 8, //if sth was expected to be literal but it turns out to it has wrong format
    file_begin = 9, //tells that here is the begin of some file
};

//for now the lexer understands this literar types, numeric is signedInteger, character is char, string_type is string
//this types do not reprezents the actual program types - numeric can means int, long, long long etc
//this types are only for checking if the number format is correct
enum literalType {
    nonee = -1, //this is when token is not literal, why does this exist though since there's an unexpected token type?
    numeric = 0, // 213123
    character = 1, // '...'
    string_type = 2, // "..."
    float_type = 3, // 12.43
    hex_type = 4, // 0x...
    binary_type, // 0b...
    octal_type // 0o...
};