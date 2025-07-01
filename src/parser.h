/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_IDENTIFIER = 258,            /* T_IDENTIFIER  */
    T_INTEGER = 259,               /* T_INTEGER  */
    T_FLOAT = 260,                 /* T_FLOAT  */
    T_BOOL = 261,                  /* T_BOOL  */
    T_CHAR = 262,                  /* T_CHAR  */
    T_ARRAY = 263,                 /* T_ARRAY  */
    T_TYPEINT = 264,               /* T_TYPEINT  */
    T_TYPEFLOAT = 265,             /* T_TYPEFLOAT  */
    T_TYPEBOOL = 266,              /* T_TYPEBOOL  */
    T_TYPECHAR = 267,              /* T_TYPECHAR  */
    T_PLUS = 268,                  /* T_PLUS  */
    T_MINUS = 269,                 /* T_MINUS  */
    T_MULT = 270,                  /* T_MULT  */
    T_DIV = 271,                   /* T_DIV  */
    T_IDIV = 272,                  /* T_IDIV  */
    T_MOD = 273,                   /* T_MOD  */
    T_AND = 274,                   /* T_AND  */
    T_OR = 275,                    /* T_OR  */
    T_NOT = 276,                   /* T_NOT  */
    T_ASSIGN = 277,                /* T_ASSIGN  */
    T_EQ = 278,                    /* T_EQ  */
    T_NEQ = 279,                   /* T_NEQ  */
    T_LT = 280,                    /* T_LT  */
    T_GT = 281,                    /* T_GT  */
    T_LE = 282,                    /* T_LE  */
    T_GE = 283,                    /* T_GE  */
    T_COLON = 284,                 /* T_COLON  */
    T_COMMA = 285,                 /* T_COMMA  */
    T_LPAREN = 286,                /* T_LPAREN  */
    T_RPAREN = 287,                /* T_RPAREN  */
    T_AT = 288,                    /* T_AT  */
    T_ALGO = 289,                  /* T_ALGO  */
    T_FUNC = 290,                  /* T_FUNC  */
    T_PROC = 291,                  /* T_PROC  */
    T_BEGIN = 292,                 /* T_BEGIN  */
    T_END = 293,                   /* T_END  */
    T_RETURN = 294,                /* T_RETURN  */
    T_TDO = 295,                   /* T_TDO  */
    T_TDNT = 296,                  /* T_TDNT  */
    T_IF = 297,                    /* T_IF  */
    T_ELSE = 298,                  /* T_ELSE  */
    T_ENDIF = 299,                 /* T_ENDIF  */
    T_THEN = 300,                  /* T_THEN  */
    T_WHILE = 301,                 /* T_WHILE  */
    T_ENDWHILE = 302,              /* T_ENDWHILE  */
    T_FOR = 303,                   /* T_FOR  */
    T_FROM = 304,                  /* T_FROM  */
    T_TO = 305,                    /* T_TO  */
    T_DO = 306,                    /* T_DO  */
    T_ENDFOR = 307,                /* T_ENDFOR  */
    T_REPEAT = 308,                /* T_REPEAT  */
    T_UNTILL = 309,                /* T_UNTILL  */
    UMINUS = 310                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "parser.y"


    AST_node* node; /* used for non-terminal symbols for the grammar */

    int tok; 
    char* str; //for identifiers 
    Const_value val; 

#line 123 "parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
