#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Support.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>

#include "ast.h"
#include "symboltable.h"
#include "builtins.h"

typedef struct Codegen_ctx_s {
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    Symbol_table* global_sym_tab; 
    Symbol_table* current_sym_tab; 
    Type* current_fn_ret_type;  
    bool current_block_terminated; 
    LLVMTypeRef printf_type; 
    LLVMValueRef printf_ref; 
} Codegen_ctx; 

void code_gen_ir(Codegen_ctx *ctx, AST_node* program_node);

void code_gen_init(Codegen_ctx *ctx);
void code_gen_cleanup(Codegen_ctx *ctx);

/* subprograms */ 
void code_gen_subprograms(Codegen_ctx *ctx, AST_node* subprograms); 

/* statements */ 
void code_gen_stmt(Codegen_ctx *ctx, AST_node* stmt); 

/* expression */ 
LLVMValueRef code_gen_exp(Codegen_ctx *ctx, AST_node* exp); 
LLVMValueRef code_gen_lval(Codegen_ctx *ctx, AST_node* lval); 
#define code_gen_rval(c, r) code_gen_exp(c, r)

/* type */ 
LLVMValueRef code_gen_promote(Codegen_ctx *ctx, LLVMValueRef value, Type* val_type, Type* dest_type);
Type* code_gen_resolve_type(Codegen_ctx* ctx, AST_node* type); 

St_entry* find_var(Codegen_ctx *ctx, char * name);
St_entry* find_fun(Codegen_ctx *ctx, char * name, Type** args, size_t args_count);
void code_gen_populate_st(Codegen_ctx *ctx, AST_node* decls);

#endif
