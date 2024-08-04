%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chordProg.tab.h"

extern FILE *yyin;
extern FILE *yyout;
int lineno = 1;

void yyerror(const char *msg);
%}

alpha       [a-zA-Z]
digit       [0-9]
alnum       {alpha}|{digit}
print       [ -~]

ID          {alpha}+{alnum}*
ICONST      "0"|{digit}+
FCONST      "0"|{digit}*"."{digit}+
CCONST      ('{print}')|(''{[nftrbv]})
STRING      "{print}*"
BCONST       (true|false)

%%

"//".*      { /* Ignore comment at line %d */ } 

"A"         { printf(" %d: A Chord - int\n", lineno); return INT; }
"Am"        { printf(" %d: Am Chord - char\n", lineno); return CHAR; }
"A#m"       { printf(" %d: A#m Chord - bool\n", lineno); return BOOL; }
"A7"        { printf(" %d: A7 Chord - float\n", lineno); return FLOAT; }
"Am7"       { printf(" %d: Am7 Chord - double\n", lineno); return DOUBLE; }
"B"         { printf(" %d: B Chord - if\n", lineno); return IF; }
"Bm"        { printf(" %d: Bm Chord - else\n", lineno); return ELSE; }
"C"         { printf(" %d: C Chord - while\n", lineno); return WHILE; }
"D"         { printf(" %d: D Chord - for\n", lineno); return FOR; }
"Dm"        { printf(" %d: Dm Chord - break\n", lineno); return BREAK; }
"D#m"       { printf(" %d: D#m Chord - continue\n", lineno); return CONTINUE; }
"E"         { printf(" %d: E Chord - void\n", lineno); return VOID; }
"F"         { printf(" %d: F Chord - return\n", lineno); return RETURN; }

"+"|"-"     {  printf(" %d: +|-\n", lineno);return ADDOP; }
"*"         {  printf(" %d: *\n", lineno) ;return MULOP; }
"/"         {  printf(" %d: /\n", lineno);return DIVOP; }
"++"|"--"   {  printf(" %d: ++|--\n", lineno);return INCR; }
"||"        {  printf(" %d: ||\n", lineno);return OROP; }
"&&"        {  printf(" %d: &&\n", lineno);return ANDOP; }
"!"         {  printf(" %d: !\n", lineno);return NOTOP; }
"=="|"!="   {  printf(" %d: == | != \n", lineno);return EQUOP; }
"<"         { printf(" %d: RELOP\n", lineno);return RELOP; }

"("         { printf(" %d: (\n", lineno);return LPAREN; }
")"         { printf(" %d: )\n", lineno);return RPAREN; }
"["         { printf(" %d: ]\n", lineno);return LBRACK; }
"]"         { printf(" %d: [\n", lineno);return RBRACK; }
"{"         { printf(" %d: {\n", lineno);return LBRACE; }
"}"         { printf(" %d: }\n", lineno);return RBRACE; }
";"         { printf(" %d: ;\n", lineno);return SEMI; }
"."         { printf(" %d: .\n", lineno);return DOT; }
","         { printf(" %d: ,\n", lineno);return COMMA; }
"="         { printf(" %d: =\n", lineno); return ASSIGN; }
"&"         { printf(" %d: &\n", lineno);return REFER; }
"->"        { printf(" %d: ->\n", lineno); return LARROW; }
"<-"        { printf(" %d: <-\n", lineno); return RARROW; }
"::"        { printf(" %d: ::\n", lineno); return DOUBLECOLON; }

{ICONST}    { printf(" %d: CONST\n", lineno);return ICONST; }
{BCONST}    { printf(" %d: BCONST\n", lineno);return BCONST; }
{ID}        { printf(" %d: ID\n", lineno);return ID; }

\n          { printf("\n"); lineno++; }
[ \t\r\f]+  /* eat up whitespace */

.           { yyerror("Unrecognized character"); }

%%
