%{
#include <stdio.h>
#include <stdlib.h>
#define YY_NO_UNPUT
int yylex(void);
void yyerror(char* err);
%}

%union {
    long long_value;
}

%start  input

%token  <long_value>    LITERAL
%type   <long_value>    exp
%left   PLUS


%%

input:	/* empty */
		| exp	{ printf("Exp: %i\n", $1); }
		;

exp:	LITERAL        	{ $$ = $1; }
		| exp PLUS exp	{ $$ = $1 + $3; }
		;

%%

void yyerror(char* err) {}