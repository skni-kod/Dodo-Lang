%{
#include <stdio.h>
#include <stdlib.h>
#include "../bison/bison.h"
void lexer_print(const char* data, int len);
int yyline_no;

/*UTF8_1  [\x00-\x7F]
UTF8_2  [\xB0-\xDF]
UTF8_3  [\xE0-\xEF]
UTF8_4  [\xF0-\xEF]
UTF8_N  [\x80-\xCF]
UTF8    [UTF8_1]/[UTF8_2][UTF8_N]/[UTF8_3][UTF8_N][UTF8_N]/[UTF8_4][UTF8_N][UTF8_N][UTF8_N]
WSPC    [\s\t\n]
CHAR    [a-zA-Z]
ISYM    [_]
NUM     [0-9]
WSPC    [\t\s]
*/

%}

%%




[a-zA-Z]+   { lexer_print(yytext, yyleng); }

[0-9]+      { yylval.long_value = atoi(yytext); return LITERAL; }

"+"         { return PLUS; }

[\n]        { yyline_no++; }

%%

void lexer_print(const char* data, int len) {
    printf("\nFound on %i, %i characters: %s\n", yyline_no, len, data);
    return;
}
