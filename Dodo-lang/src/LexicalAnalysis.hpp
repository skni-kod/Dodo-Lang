#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

//this contails ProgramLine and LexicalToken
#include "LexicalToken.hpp"


/*
ABOUT LEXER:
- for now it only analize one file
- creates vector of line object - one line have inside it all tokens that was in there  - more in LexicalToken.h
- his name is Bob. Bob still learns how to be a good lexer
- "Bob" isn't being developed and will be replaced soon

- IMPORTANT VARIABLES:
	- f_token_list - conteins information created in lexical process, this info is divided into pages, lines and tokens
		f_token_list[0][0][0] - first file, first line in this file, first token in this line
	- token_list - contains information created in lexical process, this info is divided into lines and tokens
		token_list[0][0] - first line, first token in this line
	- f_size - number of analized files
	- singleSize - number of lines inside token_list
-------------------------------------------------------------------------------------------------------------------------------------

WORKING EXAMPLE:
code written in dodo  (before lexical analize):

	i32 a = 0, b = 1;
	i32 c;
	c = a + b;

after lexical analysis:

	[keyword, i32][identifier, a][operand, =][literal, 0][comma][identifier, b][operand, =][literal, 1][expressionEnd]
	[keyword, i32][identifier c][expressionEnd]
	[literal, c][operand, =][literal, a][operand, +][literal, b][expressionEnd]
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
token object has three parameters - value, type and literal - check the LexicalToken.h for more info

*/

//contains info about file when adding it to list of lexing files
//this information is used to display proper error information when file was not found
struct added_file_info
{
	std::string file_name; //name of file
	std::string master_file; //name of file when was used #imp instruction
	int number_line; //number of line in file when was used #imp instruction
};

//singleton - most important function - lexiucal_analize
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

	//used in function to add new files if needs
	std::vector<added_file_info> file_names;

protected:
	static list_of_tokens* list_of_tokens_;

public:

	//number of pages inside f_token_list
	int f_size = 0;

	//number of lines inside token_list
	int size;


	//if set to true means that Bob didn't find some files
	bool was_errors;

	//this contains all info after lexing
	std::vector<ProgramLine> token_list;
	std::vector<ProgramPage> f_token_list;

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
	std::vector<ProgramLine>analize_file(std::fstream& file, std::string name);

	void lexical_analize(const std::string start_file_name);

	//prints info about every line
	void list_of_tokens_print();

	//list of all keyword of dodo lang
	//adding new elements REMEBER about changing numer_of_keywords value - this variable is abouve
	// WHYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
	//because reasons - USE STD::ARRAY<T> PLEASE
	std::string list_of_keywords[17] =
	{
		"return",
		"while",
		"let",
		"type",
		"mut",
		"for",
		"else",
		"if",
		"protected",
		"struct",
		"if",
		"class",
		"true",
		"break",
		"private",
		"public",
		"interrupt"
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
		"[",
		"]",
        ":",
		";",
		",",
		"}",
        "{"
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




