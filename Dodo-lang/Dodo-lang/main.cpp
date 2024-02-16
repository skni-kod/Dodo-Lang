#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "lexical_analysis.h"
#include <memory>
#include "Parser/Parser.hpp"

int main(int argc, char* argv[])
{
	//Compiling process:
	/*
	1. Read program file
	
	2. Lexical analysis

	3. Parsing + syntax error detecting
		- creating AST 

	4. Compiling
		- using assembly

	5. Run program and create exe
	*/


	//in future change to name from argv
	//another file FirstProgram.txt
	std::string file_name = "CheckLexer.txt";
	
	//opening file
	std::fstream plik;
	plik.open(file_name, std::fstream::in);
	
	//passing file to lexer and starting lexical analise
	std::unique_ptr<list_of_tokens> lt(list_of_tokens::get_instance());

	//display original file:
	//* - the switch to turn on/off below code
	std::string line;
	int licznik = 1;

	while (getline(plik, line))
	{
		std::cout << licznik << ". " << line << std::endl;
		licznik++;
	}

	plik.clear();
	//*/

	std::cout << "\n\n\n" << std::endl;
	std::cout << "lexing..." << std::endl;

	//do lexical analize.
	std::cout << "LINKER ERRORS: " << std::endl;
	lt->lexical_analize(file_name);
	std::cout << "\n\n\n" << std::endl;


	//below is for checking if the lexing procces was correct
	std::cout << "LEXER OUTPUT: " << std::endl;
	lt->list_of_tokens_print();


	//LEXER HOW TO USE
	//lt->f_token_list;// - list divided into files, lines and tokens f_token_list[0][0][0] - first file, first line inside this file, first token inside line
	//lt->token_list;// - all lexical info divided into lines and tokens
	//lt->f_size; //numer of files that was used
	//lt->size; //number of lines inside token_list
	//lt->f_token_list[0].p_size; // - number of lines inside first file
	//lt->f_token_list[0][0].l_size; //-number of tokens inside first line in first file

	plik.close();

	std::cout << "INFO L1: Lexing done!\nINFO L1: Parsing:\n";
    try {
        RunParsing(lt->token_list);
    }
    catch (ParserException& e){
        std::cout << "Parsing has failed. compilation aborted!\n";
        return 1;
    }
    std::cout << "INFO L1: Parsing completed successfully!\nINFO L1: Generating code:\n";


	return 0;
}