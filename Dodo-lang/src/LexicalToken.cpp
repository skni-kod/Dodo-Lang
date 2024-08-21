#include "LexicalToken.hpp"
//---------------------------------LINE OF PRORAM--------------------------------------------------------------------------------

//prints all tokens in line
void ProgramLine::line_print() const
{
	int size = line.size();
	for (int i = 0; i < size; i++)
		std::cout << line[i];
}

//adds token with none literal type
void ProgramLine::add_token(int type, std::string value)
{
	line.push_back(LexicalToken(type, value));
}

//adding token with decision what literal is
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
			if (!isdigit(ch) && !((ch >= 65 && ch <= 69) || (ch >= 95 && ch <= 101)))
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

LexicalToken& ProgramLine::operator[](int l_ind)
{
	int i = l_ind;

	if (abs(l_ind) > l_size)
		i = l_ind % l_size;

	if (i < 0)
		i = l_size - i;

	return line[i];
}


//-----------------------------------LEXICAL TOKEN----------------------------------------------------------

//names to dispaly
std::string LexicalToken::names[11] = { "keyword", "operand", "identifier", "comma", "expressionEnd", "blockBegin", "blockEnd", "literal", "unexpected", "fileBegin", "fileEnd"};
std::string LexicalToken::lnames[6] = { "none", "numeric", "character", "string_type", "float_type", "hex_type" };


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
    literalValue = -1;
}

LexicalToken::LexicalToken(int type, std::string value, int literal_type)
{
	this->type = type;
	this->value = value;
	this->literalValue = literal_type;
}

//getters

int LexicalToken::get_ltype()
{
	return this->literalValue;
}
int LexicalToken::get_type()
{
	return this->type;
}
std::string LexicalToken::get_value()
{
	return this->value;
}

//----------------------------PROGRAM PAGE------------------------------------------------


void ProgramPage::add_line(ProgramLine p_line)
{
	page.push_back(p_line);
}

ProgramLine& ProgramPage::operator[](int p_ind)
{
	int i = p_ind;

	if (abs(p_ind) > p_size)
		i = p_ind % p_size;

	if (i < 0)
		i = p_size - i;

	return page[i];
}


std::ostream& operator<<(std::ostream& os, const ProgramPage& p)
{
	os << "FILE NAME: " << p.file_name << std::endl;

	for (int i = 0; i < p.p_size; i++)
	{
		os << p.page[i] << std::endl;
	}

	return os;
}