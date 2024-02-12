#pragma once
#include <iostream>
#include <vector>
#include <ostream>

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
- have variable literalType - it contains information about type of literalType such as char, string, float, numeric and hex,
this types are not the real program types, they are used for checking if the format is correct (numeric can means i32, long etc),
type none is using when this is not literalType types
- unexpected type means that there is probably mistake in literalType format, but it is left to parser to decide
----------------------------------------------------------------------------------------------------------

ABOUT ProgramLine:
- have number of line
- contains all tokens from one line of program file
- contains number of tokens in line
exampe:
code below is treated as two diffrent lexer lines

i32 a = 2
+ 4;

FUTURE PLANS:
- add page object that will be containing all of the lines from one file of program
#inc <library> - that instruction will be adding another page 
*/


class LexicalToken
{

public:
	//types are described at the end of that file
	int type;
	int literal;
private:
    //names of token types - use for display
	static std::string names[9];

	//names of literalType types - use for display
	static std::string lnames[6];

public:
	//literalType type is set to -1
	LexicalToken(int type, std::string value);

	//allows to set literalType type
	LexicalToken(int type, std::string value, int literal_type);

	//friens
	friend std::ostream& operator<<(std::ostream& os, const LexicalToken& dt);

	//getters
	int get_ltype();
	int get_type();
	std::string get_value();

    //value
    std::string value;
};

class ProgramLine
{
public:

	//vector of all tokens in one line
	std::vector<LexicalToken> line;
	
	//number of line - 
	int line_number =0;
	
	// function to print line with all info
	//displays also types of numeric
	void line_print() const;

	//overriding << operator
	friend std::ostream& operator<<(std::ostream& os, const ProgramLine& l);
	
	//add token when it's not literalType
	void add_token(int type, std::string value);

	//add literalType token with literalType - uses when adding strings and chars
	void add_token(int type, std::string value, int literal_type);

	//add literalType token with numeric type check
	void add_token(int type, std::string value, bool checkNumeric);

};


//TODO
//class page{};  - this gonna be used for multiple files lexing

//it contains all token types
enum tokenType
{
	keyword = 0, //part of the language
	operand = 1,
	identifier = 2, //variable names, functions names, object names etc
	comma = 3,  
	endline = 4, //;
	blockBegin = 5, //{
	blockEnd = 6, //}
	literal = 7, //number, string, char, bool, etc.
	unexpeted_type  = 8 //if sth was expected to be literalType but it turns out to it has wrong format
};

//for now the lexer understands this literar types, numeric is integer, character is char, string_type is string
//this types do not reprezents the actual program types - numeric can means int, long, long long etc
//this types are only for checking if the number format is correct
//in the future the hex type will be added
enum literalType
{
	none = -1, //this is when token is not literalType
	numeric = 0, // 213123
	character = 1, // '...'
	string_type = 2, // "..."
	float_type = 3, // 12.43
	hex_type = 4 // 0x...
};