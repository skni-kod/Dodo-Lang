#include "lexical_token.h"
//---------------------------------LINE OF PRORAM--------------------------------------------------------------------------------

//prints all tokens in line
void line_of_program::line_print() const
{
	int size = line.size();
	for (int i = 0; i < size; i++)
		std::cout << line[i];
}

//adds token with none literal type
void line_of_program::add_token(int type, std::string value)
{
	line.push_back(lexical_token(type, value));
}

//adding token with decision what literal_type is
void line_of_program::add_token(int type, std::string value, int literal_type)
{
	line.push_back(lexical_token(type, value, literal_type));
}

//function that check numeric and float format when adding new lexical token
void line_of_program::add_token(int type, std::string value, bool checkNumeric)
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
			line.push_back(lexical_token(unexpeted_type,value));
			return;
		}
		for (int i = 2; i < size; i++)
		{
			ch = value[i];
			if (!isdigit(ch) && !((ch >= 65 && ch <= 69)|| (ch >= 95 && ch <=101) ))
			{
				line.push_back(lexical_token(unexpeted_type, value));
				return;
			}
		}
		line.push_back(lexical_token(7, value, hex_type));
		return;
	}

	//checking if it is float or integer
	for (int i = 0; i < size; i++)
	{
		ch = value[i];
		if (!isdigit(ch) && ch != '.')
		{
			line.push_back(lexical_token(unexpeted_type, value));
			return;
		}
		if (ch == '.')
			dots++;

	}
	//0 dots means its integer
	if (dots == 0)
	{
		line.push_back(lexical_token(7, value, numeric));
		return;
	}
	// one dot means its float or double
	else if (dots == 1)
	{
		line.push_back(lexical_token(7, value, float_type));
		return;
	}
	//more than one dots - sth wrong but parser will decide
	else
	{
		line.push_back(lexical_token(unexpeted_type, value));
		return;
	}
		
}

std::ostream& operator<<(std::ostream& os, const line_of_program& l)
{
	int s = l.line.size();
	for (int i = 0; i < s;i++)
		os << l.line[i];
	
	return os;
}


//-----------------------------------LEXICAL TOKEN----------------------------------------------------------

//names to dispaly
std::string lexical_token::names[9] = { "keyword","operand", "identifier", "comma", "endline", "blockBegin", "blockEnd", "literal", "unexpected" };
std::string lexical_token::lnames[6] = { "none", "numeric", "character", "string_type","float_type","hex_type" };


std::ostream& operator<<(std::ostream& os, const lexical_token& lt)
{
	os << "[" << lt.names[lt.type] << "," << lt.value;
	if (lt.literal_type != -1)
		os << "," << lt.lnames[lt.literal_type + 1];
	os << "]";
	return os;
}

//constructors
lexical_token::lexical_token(int type, std::string value)
{
	this->type = type;
	this->value = value;
	literal_type = -1;
}

lexical_token::lexical_token(int type, std::string value, int literal_type)
{
	this->type = type;
	this->value = value;
	this->literal_type = literal_type;
}

//getters

int lexical_token::get_ltype()
{
	return this->literal_type;
}
int lexical_token::get_type()
{
	return this->type;
}
std::string lexical_token::get_value()
{
	return this->value;
}



