%debug
%glr-parser /* glr can handle if stmt grammar that LR(1) couldn't handle*/ 
%{
#include "types.h"
#include "ast.h"
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 
AST_node* program_node = NULL; //main program node

extern int yylex(); 
void yyerror(const char *s) {fprintf(stderr, "\033[31mError: %s\n", s); exit(2);} 

#define YYMAXDEPTH 10000 /*bigger stack size*/ 
//int yydebug = 1; /*enable debugging*/ 
%}

%union {

    AST_node* node; /* used for non-terminal symbols for the grammar */

    int tok; 
    char* str; //for identifiers 
    Const_value val; 
}

%token <str> T_IDENTIFIER 
%token <val> T_INTEGER T_FLOAT T_BOOL T_CHAR T_ARRAY
%token <tok> T_TYPEINT T_TYPEFLOAT T_TYPEBOOL T_TYPECHAR
%token <tok> T_PLUS T_MINUS T_MULT T_DIV T_IDIV T_MOD T_AND T_OR T_NOT
%token <tok> T_ASSIGN T_EQ T_NEQ T_LT T_GT T_LE T_GE T_COLON T_COMMA T_LPAREN T_RPAREN T_AT
%token <tok> T_ALGO T_FUNC T_PROC T_BEGIN T_END T_RETURN T_TDO T_TDNT
%token <tok> T_IF T_ELSE T_ENDIF T_THEN T_WHILE T_ENDWHILE T_FOR T_FROM T_TO T_DO
%token <tok> T_ENDFOR T_REPEAT T_UNTILL 


// defining non terminals
%type <node> program optional_statements statements statement assignment expression const_value id_ref if_stmt elif_branches optional_elif_branches optional_else_branch fun_declaration var_declaration declaration declarations new_type_decls array_type_decl TDO optional_TDO TDNT optional_TDNT


//precedences 
//used something similair to c 
//https://en.cppreference.com/w/c/language/operator_precedence 
%left T_COMMA
%right T_ASSIGN
%left T_OR
%left T_AND
%left T_EQ T_NEQ
%left T_LT T_GT T_LE T_GE 
%left T_PLUS T_MINUS
%left T_MULT T_DIV T_MOD T_IDIV 
%right UMINUS
%right T_NOT 
%left T_LPAREN T_RPAREN


%start program

%define parse.error verbose

%%
    program : optional_TDNT optional_TDO T_BEGIN optional_statements T_END {program_node = ast_program_create($2, $4);}

    optional_TDNT: TDNT {$$ = $1;}
                | /*empty*/ {$$ = NULL;}

    TDNT: T_TDNT new_type_decls {$$ = $2;}
        | T_TDNT {$$ = NULL;} /*empty tdnt */ 

    new_type_decls: array_type_decl

    array_type_decl: id_ref T_EQ T_ARRAY T_FROM T_INTEGER T_TYPEINT

    optional_TDO: TDO {$$ = $1;}
                | /*empty*/ {$$ = NULL;}
    
    TDO: T_TDO declarations {$$ = $2;}
        | T_TDO {$$ = NULL;} /*empty tdo */ 
            
    declarations: declarations declaration {ast_decls_node_insert($1, $2);}
                | declaration {$$ = ast_decls_node_create($1);} 

    declaration: var_declaration {$$ = $1;}
                | fun_declaration {$$ = $1;}

    fun_declaration: id_ref T_COLON T_PROC {$$ = ast_fun_decl_node_create($1);}
                | id_ref T_COLON T_FUNC {$$ = ast_fun_decl_node_create($1);}

    var_declaration: id_ref T_COLON T_TYPEINT {$$ = ast_var_decl_node_create(VAL_INT, $1);}
                | id_ref T_COLON T_TYPEFLOAT {$$ = ast_var_decl_node_create(VAL_FLOAT, $1);}
                | id_ref T_COLON T_TYPEBOOL {$$ = ast_var_decl_node_create(VAL_BOOL, $1);}
                | id_ref T_COLON T_TYPECHAR {$$ = ast_var_decl_node_create(VAL_CHAR, $1);}

    id_ref: T_IDENTIFIER {$$ = ast_id_node_create($1);}

    const_value: T_INTEGER  {$$ = ast_const_node_create(VAL_INT, $1);}
                | T_FLOAT   {$$ = ast_const_node_create(VAL_FLOAT, $1);}
                | T_BOOL    {$$ = ast_const_node_create(VAL_BOOL, $1);}
                | T_CHAR    {$$ = ast_const_node_create(VAL_CHAR, $1);}



    optional_statements : statements {$$ = $1;}
                | /*empty*/ {$$ = NULL;}

    statements: statements statement {ast_statements_node_insert($1, $2);}
                | statement     {$$ = ast_statements_node_create($1);} 
    
    statement: assignment {$$ = $1;}
                |  if_stmt {$$ = $1;}

    assignment: id_ref T_ASSIGN expression {$$ = ast_assign_node_create($1, $3);}

    if_stmt: T_IF expression T_THEN optional_statements optional_elif_branches optional_else_branch T_ENDIF {$$ = ast_if_node_create($2, $4, $5, $6);}

    optional_elif_branches: elif_branches {$$ = $1;}
                | /*empty*/ {$$ = NULL;}

    elif_branches: elif_branches T_ELSE T_IF expression T_THEN optional_statements {
                    ast_elif_node_insert($1, ast_branch_node_create($4, $6)); 
                    }
                | T_ELSE T_IF expression T_THEN optional_statements {
                    $$ = ast_elif_node_create(ast_branch_node_create($3, $5)); 
                    }

    optional_else_branch: T_ELSE optional_statements {$$ = $2;}
                | /*empty*/ {$$ = NULL;}


    
    expression: id_ref {$$ = $1;} 
                | const_value {$$ = $1;}
                | expression T_PLUS expression {$$ = ast_op_node_create(OP_ADD, $1, $3);}
                | expression T_MINUS expression {$$ = ast_op_node_create(OP_SUB, $1, $3);}
                | expression T_MULT expression {$$ = ast_op_node_create(OP_MUL, $1, $3);}
                | expression T_DIV expression {$$ = ast_op_node_create(OP_DIV, $1, $3);}
                | expression T_IDIV expression {$$ = ast_op_node_create(OP_IDIV, $1, $3);}
                | expression T_MOD expression {$$ = ast_op_node_create(OP_MOD, $1, $3);}
                | expression T_GT expression {$$ = ast_op_node_create(OP_GREATER, $1, $3);}
                | expression T_LT expression {$$ = ast_op_node_create(OP_LESS, $1, $3);}
                | expression T_GE expression {$$ = ast_op_node_create(OP_GREATER_EQUAL, $1, $3);}
                | expression T_LE expression {$$ = ast_op_node_create(OP_LESS_EQUAL, $1, $3);}
                | expression T_EQ expression {$$ = ast_op_node_create(OP_EQUAL, $1, $3);}
                | expression T_NEQ expression {$$ = ast_op_node_create(OP_NOT_EQUAL, $1, $3);}
                | expression T_OR expression {$$ = ast_op_node_create(OP_OR, $1, $3);}
                | expression T_AND expression {$$ = ast_op_node_create(OP_AND, $1, $3);}
                | T_NOT expression {$$ = ast_op_node_create(OP_NOT, $2, NULL);}
                | T_MINUS expression %prec UMINUS {$$ = ast_op_node_create(OP_UMIN, $2, NULL);} 
                | T_LPAREN expression T_RPAREN {$$ = $2;}

        

%%
