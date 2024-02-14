#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

//this contails ProgramLine and LexicalToken
#include "lexicalToken.h"


/*
ABOUT LEXER:
- for now it only analize one file
- retruns vector of line object - one line have inside it all tokens that was in there  - more in LexicalToken.h
- his name is Bob. Bob still learns how to be a good lexer
-------------------------------------------------------------------------------------------------------------------------------------

WORKING EXAMPLE:
code written in dodo  (before lexical analize):

	i32 a = 0, b = 1;
	i32 c;
	c = a + b;

after lexical analysis:

	[keyword, i32][identifier, a][operand, =][literalType, 0][comma][identifier, b][operand, =][literalType, 1][endline]
	[keyword, i32][identifier c][endline]
	[literalType, c][operand, =][literalType, a][operand, +][literalType, b][endline]
--------------------------------------------------------------------------------------------------------------------------------------

HOW TO USE
A. FUNCTIONS
1. function to get instance of list of tokens
list_of_tokens* lt = list_of_tokens::get_instance();

2. pass main programm file - this function creates vector of all tokens and returns it
lt->analize_file(file_name);

3. vector declaration (need to include  <vector> library, if this library is not included then needed to include "LexicalToken.h")
std::vector<ProgramLine> list;

4. function to print all tokens that lexer found
lt->list_of_tokens_print();

5. function that takes all tokens list
std::vector<ProgramLine> get_list_of_tokens();

B. VECTOR AND OBJECTS
list of all tokens is inside of vector token_list, 
this vector conteins ProgramLine objects
ProgramLine has a number and vector with all tokens in line
token object has three parameters - value, type and literalType - check the LexicalToken.h for more info

*/

//singleton - most important function - analize_file
class list_of_tokens
{
private:
	list_of_tokens() {};



	//PARAMETERS
	//we are using them in functions - changing it here changes it in all functions
	//-------------------------------------------------------------------------------------------
	int number_of_operands = 27;
	int numer_of_keywords = 17;
	//------------------------------------------------------------------------------------------

	//function that checks if word is keyword or operand
	bool look_for_str_table(std::string str, std::string tab[], int R);

protected:
	static list_of_tokens* list_of_tokens_;

public:

    std::vector<ProgramLine> token_list;

	//disabling construcotrs
	list_of_tokens(list_of_tokens& lt) = delete;
	void operator=(const list_of_tokens&) = delete;
	
	//function that allows to get instance of list_of_tokens
	static list_of_tokens* get_instance();
	
	//destructor for deleting the instance of singleton
	~list_of_tokens();

	//return list of toknes
	std::vector<ProgramLine> get_list_of_tokens();

	//function that analizes program file and returs list of tokens - tokens are grouped in lines object - more in LexicalToken.h
	std::vector<ProgramLine>analize_file(std::fstream& file);

	//prints info about every line
	void list_of_tokens_print();

	//list of all keyword of dodo lang
	//adding new elements REMEBER about changing numer_of_keywords value - this variable is abouve
    // WHYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
	std::string list_of_keywords[26] =
	{
		"ret",
		"while",
		"dodo",
		"type",
		"f",
		"for",
		"else",
		"if",
		"bool",
		"struct",
		"if",
		"class",
		"true",
		"break",
		"bool",
		"char",
		"const"
	};

	//list of all operators in dodo language
	//	//adding new element REMEBER about changing numer_of_operands value - this variable is abouve
	std::string list_of_operands[27] =
	{
		"(",
		")",
		"=",
		"+",
		">=",
		">",
		"<=",
		"<",
		"==",
		"-",
		"*",
		"%",
		"/",
		"!",
		"&",
		"|",
		"+=",
		"-=",
		"*=",
		"/=",
		"\"",
		"\'",
		",",
		"{",
		"}",
		";",
        ":"
	};

	//list of operators for commenting
	//it havent been used yet;
	std::string comments_operands[3] =
	{
		"//",
		"/*",
		"*/"
	};
};




