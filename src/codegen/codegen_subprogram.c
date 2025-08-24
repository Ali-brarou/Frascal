#include <codegen.h> 

static void code_gen_function(Codegen_ctx *ctx, AST_node* function); 
static Type* create_function_type(Codegen_ctx *ctx, AST_function_node* function, Type** param_types, size_t params_count); 
static LLVMValueRef create_function(Codegen_ctx *ctx, 
                            AST_function_node* fn, 
                            Type** param_types, 
                            LLVMTypeRef* llvm_param_types, 
                            size_t params_count); 

void code_gen_subprograms(Codegen_ctx *ctx, AST_node* subprograms)
{
    if (!subprograms)
        return;

    AST_subprograms_node* node = (AST_subprograms_node*)subprograms;
    LL_FOR_EACH(node->functions_list, ll_node)
    {
        AST_node* fn_node = ll_node->data;
        code_gen_function(ctx, fn_node);
    }
}

static inline Type* create_function_type(Codegen_ctx *ctx, 
                                  AST_function_node* fn, 
                                  Type** param_types, 
                                  size_t params_count)
{
    ctx->current_fn_ret_type = code_gen_resolve_type(ctx, fn->ret_type);
    return type_function_create(ctx->current_fn_ret_type, param_types, params_count);

}

static LLVMValueRef create_function(Codegen_ctx *ctx, 
                            AST_function_node* fn, 
                            Type** param_types, 
                            LLVMTypeRef* llvm_param_types, 
                            size_t params_count) 
{
    char* fun_name = ((AST_id_node*)fn->id_node)->id_str;
    Type* func_type = create_function_type(ctx, fn, param_types, params_count); 
    LLVMTypeRef llvm_func_type
        = LLVMFunctionType(type_to_llvm_type(ctx->current_fn_ret_type), 
                           llvm_param_types, 
                           params_count, 
                           0);
    LLVMValueRef func_ref= LLVMAddFunction(ctx->module, fun_name, llvm_func_type);
    if (st_insert_fun(ctx->global_sym_tab, fun_name, func_type, func_ref, llvm_func_type)
            == ST_ALREADY_DECLARED)
    {
            fprintf(stderr, "Error : function %s defined twice\n", fun_name);
            exit(3);
    }
    return func_ref; 
}

static void code_gen_function(Codegen_ctx *ctx, AST_node* function)
{
    if (!function)
        return;
    AST_function_node* fn = (AST_function_node*)function;
    AST_params_node* params = (AST_params_node*)fn->params;
    size_t params_count = 0;
    Type** param_types = NULL;
    char** param_names = NULL;
    LLVMTypeRef* llvm_param_types = NULL;

    if (params != NULL)
    {
        params_count = LL_size(params->params_list);
        param_types = malloc(params_count * sizeof(Type*));
        param_names = malloc(params_count * sizeof(char*));
        llvm_param_types = malloc(params_count * sizeof(LLVMTypeRef));

        size_t index = 0;
        /* iterate over every paramater */
        LL_FOR_EACH(params->params_list, ll_node)
        {
            AST_param_node* param = ll_node->data;

            param_names[index] = ((AST_id_node*)param->id_node)->id_str;
            Type* param_type = code_gen_resolve_type(ctx, param->id_type);
            param_types[index] = param_type;
            llvm_param_types[index] = type_to_llvm_type(param_types[index]);

            index++;
        }
    }

    /* create function type and insert it into the global symbol table */
    LLVMValueRef func_ref = create_function(ctx, 
                                            fn, 
                                            param_types, 
                                            llvm_param_types, 
                                            params_count); 

    /* create a local symbol table */
    ctx->current_sym_tab = st_create();

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func_ref, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);

    LLVMValueRef param_alloca;
    for (size_t i = 0; i < params_count; i++)
    {
        param_alloca = LLVMBuildAlloca(ctx->builder, llvm_param_types[i], param_names[i]);
        LLVMBuildStore(ctx->builder, LLVMGetParam(func_ref, i), param_alloca);
        st_insert_var(ctx->current_sym_tab, param_names[i], param_types[i], param_alloca);
    }

    /* populate local sym table */
    code_gen_populate_st(ctx, fn->declarations);

    /* generate statments */
    code_gen_stmt(ctx, fn->statements);

    if (!is_block_terminated(ctx))
    {
        fprintf(stderr, "Error: missing a return statement\n");
        exit(3);
    }


    /*clean up */
    free(param_types);
    free(llvm_param_types);
    free(param_names);
    st_free(ctx->current_sym_tab); /* free the local symbol table */
    ctx->current_sym_tab = NULL;
    ctx->current_fn_ret_type = NULL;
    ctx->current_block_terminated = false;
}
