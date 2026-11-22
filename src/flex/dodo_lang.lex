%{
#include <stdio.h>
#include <stdlib.h>
#include "../bison/bison.h"
#include "../c_interop/parsing.h"
void lexer_print(const char* data, int len);

#define YY_USER_ACTION    \
yycol_no_prev = yycol_no; \
yycol_no += yyleng;

%}

%%




[a-zA-Z]+   { lexer_print(yytext, yyleng); }

[0-9]+      { yylval.long_value = atoi(yytext); return LITERAL; }

"+"         { return PLUS; }

[\n]        { yyline_no++; yycol_no = 1; }

%%

void lexer_print(const char* data, int len) {
    printf("\nFound on %i, %i characters: %s\n", yyline_no, len, data);
    return;
}
