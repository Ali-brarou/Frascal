#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h> 
#include <llvm-c/Analysis.h> 
#include <llvm-c/Support.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h> 

#include "ast.h"
#include "symboltable.h"

void gen_ir(AST_node* program_node);   

void code_gen_init(void); 
void code_gen_cleanup(void); 

#endif
