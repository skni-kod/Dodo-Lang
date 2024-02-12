#include "lexicalToken.h"
//---------------------------------LINE OF PRORAM--------------------------------------------------------------------------------

//prints all tokens in line
void ProgramLine::line_print() const
{
	int size = line.size();
	for (int i = 0; i < size; i++)
		std::cout << line[i];
}

//adds token with none literalType type
void ProgramLine::add_token(int type, std::string value)
{
	line.push_back(LexicalToken(type, value));
}

//adding token with decision what literalType is
void ProgramLine::add_token(int type, std::string value, int literal_type)
{
	line.push_back(LexicalToken(type, value, literal_type));
}

//function that check numeric and float format when adding new lexical token
void ProgramLine::add_token(int type, std::string value, bool checkNumeric)
{
	int dots = 0;
	std::string hex = "0x";
	int size = value.length();
	char ch;

	//checing if it hex number
	if (value.find(hex) != -1)
	{
		if (size == 2)
		{
			line.push_back(LexicalToken(unexpeted_type, value));
			return;
		}
		for (int i = 2; i < size; i++)
		{
			ch = value[i];
			if (!isdigit(ch) && !((ch >= 65 && ch <= 69)|| (ch >= 95 && ch <=101) ))
			{
				line.push_back(LexicalToken(unexpeted_type, value));
				return;
			}
		}
		line.push_back(LexicalToken(7, value, hex_type));
		return;
	}

	//checking if it is float or integer
	for (int i = 0; i < size; i++)
	{
		ch = value[i];
		if (!isdigit(ch) && ch != '.')
		{
			line.push_back(LexicalToken(unexpeted_type, value));
			return;
		}
		if (ch == '.')
			dots++;

	}
	//0 dots means its integer
	if (dots == 0)
	{
		line.push_back(LexicalToken(7, value, numeric));
		return;
	}
	// one dot means its float or double
	else if (dots == 1)
	{
		line.push_back(LexicalToken(7, value, float_type));
		return;
	}
	//more than one dots - sth wrong but parser will decide
	else
	{
		line.push_back(LexicalToken(unexpeted_type, value));
		return;
	}
		
}

std::ostream& operator<<(std::ostream& os, const ProgramLine& l)
{
	int s = l.line.size();
	for (int i = 0; i < s;i++)
		os << l.line[i];
	
	return os;
}


//-----------------------------------LEXICAL TOKEN----------------------------------------------------------

//names to dispaly
std::string LexicalToken::names[9] = {"keyword", "operand", "identifier", "comma", "endline", "blockBegin", "blockEnd", "literalType", "unexpected" };
std::string LexicalToken::lnames[6] = {"none", "numeric", "character", "string_type", "float_type", "hex_type" };


std::ostream& operator<<(std::ostream& os, const LexicalToken& lt)
{
	os << "[" << lt.names[lt.type] << "," << lt.value;
	if (lt.literal != -1)
		os << "," << lt.lnames[lt.literal + 1];
	os << "]";
	return os;
}

//constructors
LexicalToken::LexicalToken(int type, std::string value)
{
	this->type = type;
	this->value = value;
    literal = -1;
}

LexicalToken::LexicalToken(int type, std::string value, int literal_type)
{
	this->type = type;
	this->value = value;
	this->literal = literal_type;
}

//getters

int LexicalToken::get_ltype()
{
	return this->literal;
}
int LexicalToken::get_type()
{
	return this->type;
}
std::string LexicalToken::get_value()
{
	return this->value;
}