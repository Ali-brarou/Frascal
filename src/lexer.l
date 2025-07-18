/* the lexer should be case-insensetive */
%option caseless 
/* disable yywrap: no need to check another file */
%option noyywrap 
/* flex maintains the number of the current line */
%option yylineno
%{
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 

#include "ast.h" //ast should be include before parser.h
#include "parser.h"

#define TOKEN(t)    (yylval.tok = t)
#define SAVE_ID     yylval.str = strdup(yytext)
#define SAVE_INT    yylval.val.ival = atoi(yytext) 
#define SAVE_FLOAT  yylval.val.fval = strtof(yytext, NULL) 
#define SAVE_TRUE   yylval.val.bval = true
#define SAVE_FALSE  yylval.val.bval = false 
#define SAVE_CHAR   yylval.val.cval = yytext[1]

%}

WHITESPACE [ \n\t\r]
DIGIT [0-9]
ID [a-z_][a-z0-9_]*

%%

{WHITESPACE} ; /* whitespaces should be ignored */ 

"//".*|"(*"(.|\n)*"*)" ; /* ignore comments */ 

{DIGIT}+                    {SAVE_INT; return T_INTEGER;}
{DIGIT}+"."{DIGIT}*         {SAVE_FLOAT; return T_FLOAT;} 
"VRAI"                      {SAVE_TRUE; return T_BOOL;}
"FAUX"                      {SAVE_FALSE; return T_BOOL;}
'.'                         {SAVE_CHAR; return T_CHAR;}

"+"             {return TOKEN(T_PLUS);}
"-"             {return TOKEN(T_MINUS);}
"*"             {return TOKEN(T_MULT);}
"/"             {return TOKEN(T_DIV);}
"DIV"           {return TOKEN(T_IDIV);}
"MOD"           {return TOKEN(T_MOD);}
"et"            {return TOKEN(T_AND);}
"ou"            {return TOKEN(T_OR);}
"non"           {return TOKEN(T_NOT);}
":="            {return TOKEN(T_ASSIGN);} 
"="             {return TOKEN(T_EQ);} 
"!="            {return TOKEN(T_NEQ);}
">"             {return TOKEN(T_GT);}
">="            {return TOKEN(T_GE);}
"<"             {return TOKEN(T_LT);}
"<="            {return TOKEN(T_LE);}
":"             {return TOKEN(T_COLON);}
","             {return TOKEN(T_COMMA);}
"("             {return TOKEN(T_LPAREN);}
")"             {return TOKEN(T_RPAREN);}
"@"             {return TOKEN(T_AT);}
"si"            {return TOKEN(T_IF);}
"sinon"         {return TOKEN(T_ELSE);}
"finsi"         {return TOKEN(T_ENDIF);}
"alors"         {return TOKEN(T_THEN);}
"tant que"      {return TOKEN(T_WHILE);}
"fin tant que"  {return TOKEN(T_ENDWHILE);}
"pour"          {return TOKEN(T_FOR);}
"de"            {return TOKEN(T_DE);}
"a"             {return TOKEN(T_TO);}
"faire"         {return TOKEN(T_DO);}
"fin pour"      {return TOKEN(T_ENDFOR);}
"repeter"       {return TOKEN(T_REPEAT);}
"jusqu'a"       {return TOKEN(T_UNTILL);}
"ALGO"          {return TOKEN(T_ALGO);}
"fonction"      {return TOKEN(T_FUNC);}
"procedure"     {return TOKEN(T_PROC);}
"ecrire"        {return TOKEN(T_PRINT);}
"debut"         {return TOKEN(T_BEGIN);}
"fin"           {return TOKEN(T_END);}
"retourner"     {return TOKEN(T_RETURN);}
"TDNT"          {return TOKEN(T_TDNT);}
"TDOL"           {return TOKEN(T_TDOL);}
"TDOG"           {return TOKEN(T_TDOG);}
"entier"        {return TOKEN(T_TYPEINT);}
"reel"          {return TOKEN(T_TYPEFLOAT);}
"booleen"       {return TOKEN(T_TYPEBOOL);}
"caractere"     {return TOKEN(T_TYPECHAR);}
"tableau"       {return TOKEN(T_ARRAY);}

{ID}+           {SAVE_ID; return T_IDENTIFIER;}
           
          
.       fprintf(stderr, "\033[31mError : Unrecognized character \"%c\" at ligne %d\n", yytext[0], yylineno); exit(1); 

%%
