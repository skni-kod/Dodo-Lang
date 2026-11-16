%{
#include<stdio.h>
#include<stdlib.h>
void lexer_print(const char* data, int len);
%}

%%

[a-zA-Z]+ { lexer_print(yytext, yyleng); }

%%

void lexer_print(const char* data, int len) {
    printf("\nFound %i: %s\n", len, data);
    return;
}
