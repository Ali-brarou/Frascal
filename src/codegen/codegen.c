#include "codegen.h"

void code_gen_init(Codegen_ctx *ctx)
{
    memset(ctx, 0, sizeof(Codegen_ctx)); 
    //init llvm 
    ctx->module = LLVMModuleCreateWithName("main_module"); 
    ctx->builder = LLVMCreateBuilder(); 

    //set up the symbol table 
    ctx->global_sym_tab = st_create(); 

    //define builtins functions 
    builtins_init(ctx->module); 
    builtins_add_to_symtab(ctx->global_sym_tab); 


    /* printf 
     * return type : int
     * argument : (char* , ...)
     * isVarArg : true 
     */
    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    ctx->printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, true); 
    ctx->printf_ref = LLVMAddFunction(ctx->module, "printf", ctx->printf_type); 

}

void code_gen_cleanup(Codegen_ctx *ctx)
{
    //cleanup llvm 
    LLVMDisposeBuilder(ctx->builder); 
    LLVMDisposeModule(ctx->module); 

    //free the symbol table
    st_free(ctx->global_sym_tab); 
}

St_entry* find_var(Codegen_ctx* ctx, char* name)
{
    St_entry* var = st_find_var(ctx->current_sym_tab, name); 
    if (var)
        return var; 

    /* if it's the main function dont research in the global sym tab */ 
    if (ctx->current_sym_tab == ctx->global_sym_tab)
        return NULL; 

    return st_find_var(ctx->global_sym_tab, name); 
}

/* for now search only the global table */ 
St_entry* find_fun(Codegen_ctx* ctx, char* name, Type** args, size_t args_count)
{
    return st_find_fun(ctx->global_sym_tab, name, args, args_count); 
}


void code_gen_populate_st(Codegen_ctx* ctx, AST_node* decls)
{
    if (!decls)
        return; 
    LL_FOR_EACH(((AST_declarations_node*)decls) -> var_decls_list, ll_node)
    {
        AST_var_declaration_node* decl_node = (AST_var_declaration_node*)ll_node -> data; 
        AST_id_node* id_node = (AST_id_node*)(decl_node -> id_node); 

        Type* decl_type = code_gen_resolve_type(ctx, decl_node->id_type); 

        LLVMValueRef id_alloca = LLVMBuildAlloca(ctx->builder, type_to_llvm_type(decl_type), id_node->id_str);
        if (st_insert_var(ctx->current_sym_tab, id_node->id_str, decl_type, id_alloca)
                                                == ST_ALREADY_DECLARED)
        {
            fprintf(stderr, "Error : variable %s declared twice\n", id_node -> id_str); 
            exit(3); 
        }
    }
}

void code_gen_ir(Codegen_ctx *ctx, AST_node* program_node)
{
    assert(program_node != NULL); 
    // user defined types 
    code_gen_new_types(ctx, ((AST_program_node*)program_node)->new_types); 
    
    //code generating subprograms 
    code_gen_subprograms(ctx, ((AST_program_node*)program_node)->subprograms); 
    
    //creating a main function without arguments
    LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_function = LLVMAddFunction(ctx->module, "main", ret_type);

    //creating entry basic block 
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_function, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);

    //allocate variables in the stack
    //* it's the main function there is no local symtoble so make it point to the gloable table *//  
    ctx->current_sym_tab = ctx->global_sym_tab; 
    code_gen_populate_st(ctx, ((AST_program_node*)program_node)->declarations); 

    //statements ir generation
    code_gen_stmt(ctx, ((AST_program_node*)program_node)->statements); 

    //main function return 
    LLVMBuildRet(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, false));
    //verify the main module
    char* error = NULL; 
    if (LLVMVerifyModule(ctx->module, LLVMAbortProcessAction, &error))
    {
        fprintf(stderr, "Error : %s\n", error); 
        exit(3); 
    }
    LLVMDisposeMessage(error); 
    //print the final ir to a file  
    LLVMPrintModuleToFile(ctx->module, "out.ll", NULL); 

}
