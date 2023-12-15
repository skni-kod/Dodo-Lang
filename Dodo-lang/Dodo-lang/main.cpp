#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "lexical_analysis.h"

using namespace std;

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
	string file_name = "CheckLexer.txt";
	
	//opening file
	fstream plik;
	plik.open(file_name, fstream::in);
	
	//passing file to lexer and starting lexical analise
	list_of_tokens* lt = list_of_tokens::get_instance();

	//display original file:
	//* - the switch to turn on/off below code
	string line;
	int licznik = 1;
	while (getline(plik, line))
	{
		std::cout << licznik << ". " << line << std::endl;
		licznik++;
	}

	plik.clear();
	plik.seekg(0);
	//*/

	cout << "lexing..." << endl;

	//getting the list of tokens - function below returns vector of lines
	lt->analize_file(plik);

	//below is for checking if the lexing procces was correct
	//lt->list_of_tokens_print();

	plik.close();

	cout << "parsing..." << endl;
	//parsing process
	//checing if there are erros
		//YES) displaying erros and stops compilation process
		// NO) countinue
	

	return 0;
}