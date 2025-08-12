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

    Codegen_ctx codegen_ctx; 

    /* compiler init */ 
    code_gen_init(&codegen_ctx); /* codegen */  

    yyparse(); 
    code_gen_ir(&codegen_ctx, program_node);
    /* debug */ 
    /* AST_tree_print(program_node, 0); */ 
    
    AST_tree_free(program_node); 
    code_gen_cleanup(&codegen_ctx);  
    return 0; 
}
