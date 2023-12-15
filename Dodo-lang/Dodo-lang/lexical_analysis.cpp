#include "lexical_analysis.h"

list_of_tokens* list_of_tokens::list_of_tokens_ = nullptr;

bool list_of_tokens::look_for_str_table(std::string str, std::string tab[], int R)
{

	bool ret = false;

	for (int i = 0; i < R; i++)
	{
		if (str == tab[i])
			return true;
	}

	return false;
}

list_of_tokens* list_of_tokens::get_instance()
{
	if (list_of_tokens_ == nullptr)
		list_of_tokens_ = new list_of_tokens();

	return list_of_tokens_;
}

list_of_tokens::~list_of_tokens()
{
	if (list_of_tokens_ != nullptr)
		delete list_of_tokens_;
}

std::vector<line_of_program> list_of_tokens::get_list_of_tokens()
{
	return token_list;
}

std::vector<line_of_program> list_of_tokens::analize_file(std::fstream& file)
{
	std::string line;
	std::string word;
	int words_num;
	int counter = 0;
	char ch;
	std::string operand;
	std::string help;
	bool probably_operand;
	bool word_under_construction;
	bool literal_under_construction;
	bool found_sth = false;
	int size;
	bool big_comment = false;
	int index, index2;
	while (std::getline(file, line))
	{
		size = line.length();
		word_under_construction = false;
		literal_under_construction = false;
		probably_operand = false;
		//this for analize one line of program and looks for tokens

		line_of_program l;
		words_num = 0;
		for (int i = 0; i < size; i++)
		{
			ch = line[i];

			//it look for end of multiline comment
			//if not break for
			//if true sets index after end of multiline comment
			if (big_comment)
			{
				index = line.find("*/");
				//std::cout <<"INDEX" << index << std::endl;
				if (index != -1)
				{
					i = index + 2;
					//std::cout << counter + 1 << ". End of big comment" << std::endl;
					big_comment = false;
				}
				else
				{
					//std::cout << "Dziala" << std::endl;
					break;
				}
			}

			//checking if comment
			//if true break for
			if ((ch == '/' && i < size - 1) && (line[i + 1] == '/' || line[i + 1] == '*'))
			{
				help = ch;
				help += line[i + 1];

				if (help == "/*")
				{
					big_comment = true;
				}

				break;
			}


			//constructing word - until find blank space or operand
			//or constructing numeric literal - it's starts with number
			if ((isalpha(ch) || ch == '_') || word_under_construction || literal_under_construction || isdigit(ch))
			{
				//sets what kind of build it is
				if(isalpha(ch) && !literal_under_construction)
					word_under_construction = true;
				else if (isdigit(ch) && !word_under_construction)
					literal_under_construction = true;

				//check if it is operand
				for (int j = 0; j < number_of_operands; j++)
				{
					help = ch;
					if (help == list_of_operands[j])
					{
						probably_operand = true;

						if (help == "." && literal_under_construction)
							probably_operand = false;

						break;
					}
						
				}
				//if it is not operand and it is not white space then add char to creating word
				if(!probably_operand && (ch!=' ' && ch!='\t'))
					word += ch;
				else
				{

					//this oppened when lexer found word that is not operand or comment

					//checking if it is a numeric literal
					if (literal_under_construction)
					{
						l.add_token(7, word, true);
						word = "";
						words_num++;
					}

					//checking if it is keyword
					for (int k = 0; k < numer_of_keywords; k++)
					{
						if (word == list_of_keywords[k])
						{
							l.add_token(keyword, word);
							words_num++;
							word = "";
							break;
						}
					}
					//if it is not keyword and it is not literal then it is identifier
					if (word != "")
					{
						l.add_token(identifier, word);
						words_num++;
						word = "";
					}
					
					word_under_construction = false;
					literal_under_construction = false;
					
					if (ch != ' ' && ch != '\t')
						operand = ch;
				}
			}
			
			
			//checking if the ch is operand
			//and dealing with operand at the end of the keyword or identifier
			if (!word_under_construction || !isalpha(ch))
			{
				help = ch;
				
				//checs if it's structural operand
				switch (ch)
				{
				case ';':
					l.add_token(endline, help);
					words_num++;
					help = "";
					break;
				case '{':
					l.add_token(blockBegin, help);
					words_num++;
					help = "";
					break;
				case '}':
					l.add_token(blockEnd, help);
					words_num++;
					help = "";
					break;
				case ',':
					l.add_token(comma, help);
					words_num++;
					help = "";
					break;
				
				case '\'':
					index2 = 0;
					index2 = line.find("\'", i + 1);
					if (index2 != -1)
					{
						help = line.substr(i, index2 - i + 1);
						//adding character literal
						l.add_token(7, help, character);
						i = index2 + 1;
					}
					else
					{
						help = line.substr(i, size - i + 1);
						//probably missing second '
						l.add_token(8, help);
						help = "";
					}
					words_num++;
					help = "";
					break;
				
				case '\"':
					index2 = line.find("\"", i + 1);
					if (index2 != -1)
					{
						help = line.substr(i, index2 - i + 1);
						//adding string literal
						l.add_token(7, help, string_type);
						i = index2 + 1;
					}
					else
					{
						help = line.substr(i, size - i + 1);
						//probably missing second "
						l.add_token(8, help);
						help = "";
					}

					words_num++;
					help = "";
					break; 
				}

				if (help == "")
					break;
		
				operand = ch;
				probably_operand = false;
				
				//check if its in list of operands
				probably_operand  = look_for_str_table(operand, list_of_operands, number_of_operands);
				
				//if it is the last character in line add it
				if (probably_operand && i == size - 1)
				{
					l.add_token(1, operand);
					words_num++;
				}

				//if it is not the last character in line check if it is two characters operand
				if (probably_operand && i <size - 1)
				{
					help = operand;
					help += line[i + 1];

					probably_operand = look_for_str_table(help, list_of_operands, number_of_operands);

					//add operand - if it has two charactes avoind checking the next operand
					if (probably_operand)
					{
						l.add_token(1, help);
						words_num++;
						i++;
					}
					else
					{
						l.add_token(1, operand);
						words_num++;
					}
				}
			}
		}

		if (word_under_construction)
		{
			if (look_for_str_table(word, list_of_keywords, numer_of_keywords))
				l.add_token(keyword, word);
			else
				l.add_token(identifier, word);
		}

		if (literal_under_construction)
			l.add_token(7, word, true);
		
		word = "";

		l.line_number = counter;
		//if line is important add it to list of lines
		if(words_num!=0)
			token_list.push_back(l);
	
		//line number couter
		counter++;

	}
	return token_list;
}

void list_of_tokens::list_of_tokens_print()
{
	int size = token_list.size();
	for (int i = 0; i < size; i++)
		std::cout << token_list[i].line_number <<". " << token_list[i] << std::endl;
}

