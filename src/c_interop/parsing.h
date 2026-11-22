// Functions used by bison/flex to create the output, defined in parsing.zig

#ifndef DODOC_PARSING_H
#define DODOC_PARSING_H

void yyerror(char* err);
extern int yyline_no;
extern int yycol_no;
extern int yycol_no_prev;

#endif //DODOC_PARSING_H
