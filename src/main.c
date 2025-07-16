#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 

#include "ast.h" //ast should be included before parser
#include "parser.h"
#include "codegen.h"

extern FILE* yyin;

extern AST_node* program_node; 


int main(int argc, char*argv[])
{
    argc--;  argv++; 

    if (argc < 1)
        yyin = stdin;  
    else
    {
        if (!(yyin = fopen(argv[0], "r")))
        {
            perror(argv[0]); 
            return 1; 
        }
    }


    /* compiler init */ 
    parser_init(); /* parser */  
    code_gen_init(); /* codegen */  

    yyparse(); 
    code_gen_ir(program_node);
    /* debug */ 
    /* AST_tree_print(program_node, 0); */ 
    
    AST_tree_free(program_node); 
    code_gen_cleanup();  
    return 0; 
}
