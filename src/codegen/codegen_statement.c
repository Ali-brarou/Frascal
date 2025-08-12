#include "codegen.h"

static void code_gen_for_stmt(Codegen_ctx *ctx, AST_node* root); 
static void insert_last_block_if_needed(Codegen_ctx *ctx, Linkedlist* buffer); /* helper function for if code gen */ 
static void code_gen_if_stmt(Codegen_ctx *ctx, AST_node* root); 
static void code_gen_while_stmt(Codegen_ctx *ctx, AST_node* root); 
static void code_gen_dowhile_stmt(Codegen_ctx *ctx, AST_node* root); 
static void code_gen_print_stmt(Codegen_ctx *ctx, AST_node* root); 


static void insert_last_block_if_needed(Codegen_ctx *ctx, Linkedlist* buffer)
{
    if (!ctx->current_block_terminated)
    {
        LLVMBasicBlockRef last_block = LLVMGetInsertBlock(ctx->builder);
        LL_insert_back(buffer, last_block);
    }
    ctx->current_block_terminated = false;
}

static void code_gen_if_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;
    AST_if_node* node = (AST_if_node*) root;
    AST_elif_node* elif_node = (AST_elif_node*) node->elif_branches;

    Linkedlist* merge_buffer = LL_create_list();

    //get the function from the builder position
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));

    LLVMBasicBlockRef if_block = LLVMAppendBasicBlock(current_function, "if_block");
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then_block");

    LLVMBuildBr(ctx->builder, if_block);

    LLVMPositionBuilderAtEnd(ctx->builder, then_block);
    code_gen_stmt(ctx, node -> action);
    insert_last_block_if_needed(ctx, merge_buffer);
    LLVMPositionBuilderAtEnd(ctx->builder, if_block);

    LLVMValueRef prev_cond = code_gen_exp(ctx, node -> cond);
    if (!AST_IS_BOOL_TYPE(node -> cond))
    {
        fprintf(stderr,"Error: the condition for the if statement is not a booleen\n");
        exit(3);
    }
    LLVMBasicBlockRef prev_then_block = then_block;

    if (elif_node != NULL)
    {
        LL_FOR_EACH(elif_node -> branches_list, ll_node)
        {
            AST_branch_node* branch_node = (AST_branch_node*)ll_node -> data;
            LLVMBasicBlockRef elif_block =
                LLVMAppendBasicBlock(current_function, "elif_block");
            LLVMBasicBlockRef then_block =
                LLVMAppendBasicBlock(current_function, "then_block");

            LLVMBuildCondBr(ctx->builder, prev_cond, prev_then_block, elif_block);
            LLVMPositionBuilderAtEnd(ctx->builder, then_block);
            code_gen_stmt(ctx, branch_node -> action);
            insert_last_block_if_needed(ctx, merge_buffer);
            LLVMPositionBuilderAtEnd(ctx->builder, elif_block);

            prev_cond = code_gen_exp(ctx, branch_node -> cond);
            if (!type_equal(ast_exp_type(branch_node -> cond),TYPE_BOOL))
            {
                fprintf(stderr,"Error: the condition for the if statement is not a booleen\n");
                exit(3);
            }
            prev_then_block = then_block;
        }
    }

    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "else_block");

    LLVMBuildCondBr(ctx->builder, prev_cond, prev_then_block, else_block);

    LLVMPositionBuilderAtEnd(ctx->builder, else_block);
    code_gen_stmt(ctx, node -> else_action);
    insert_last_block_if_needed(ctx, merge_buffer);

    //mergin all then blocks
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge_block");
    LL_FOR_EACH(merge_buffer, ll_node)
    {
        //cast it back from void* to basicblockref
        LLVMBasicBlockRef then_block = ll_node -> data;
        LLVMPositionBuilderAtEnd(ctx->builder, then_block);
        LLVMBuildBr(ctx->builder, merge_block);
    }
    LLVMPositionBuilderAtEnd(ctx->builder, merge_block);
    LL_free_list(&merge_buffer, NULL);
}

static void code_gen_for_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;

    AST_for_node* node = (AST_for_node*) root;

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));

    LLVMBasicBlockRef for_cond   = LLVMAppendBasicBlock(current_function, "for_cond");
    LLVMBasicBlockRef for_body   = LLVMAppendBasicBlock(current_function, "for_body");
    LLVMBasicBlockRef for_inc    = LLVMAppendBasicBlock(current_function, "for_inc");
    LLVMBasicBlockRef for_end    = LLVMAppendBasicBlock(current_function, "for_end");


    //check all variables are integers

    LLVMValueRef iter = code_gen_lval(ctx, node -> iter);
    LLVMValueRef from = code_gen_exp(ctx, node -> from);
    LLVMValueRef to   = code_gen_exp(ctx, node -> to);
    if (!AST_IS_INT_TYPE(node -> iter) || !AST_IS_INT_TYPE(node -> from) || !AST_IS_INT_TYPE(node -> to))
    {
        fprintf(stderr,"Error: Can't work with non integers in a for loop\n");
        exit(3);
    }

    //initilize the iterator
    LLVMBuildStore(ctx->builder, from, iter);

    LLVMBuildBr(ctx->builder, for_cond);
    LLVMPositionBuilderAtEnd(ctx->builder, for_cond);

    //for loop condition
    LLVMPositionBuilderAtEnd(ctx->builder, for_cond);
    LLVMValueRef iter_val = LLVMBuildLoad2(ctx->builder, LLVMInt32Type(), iter, "iter_val");
    LLVMValueRef cond = LLVMBuildICmp(ctx->builder, LLVMIntSLE, iter_val, to, "for_cond");
    LLVMBuildCondBr(ctx->builder, cond, for_body, for_end);

    //body of the loop
    LLVMPositionBuilderAtEnd(ctx->builder, for_body);
    code_gen_stmt(ctx, node->statements);
    if (!ctx->current_block_terminated)
        LLVMBuildBr(ctx->builder, for_inc);
    ctx->current_block_terminated = false;

    //increment block
    LLVMPositionBuilderAtEnd(ctx->builder, for_inc);
    LLVMValueRef next_val = LLVMBuildAdd(ctx->builder, iter_val, LLVMConstInt(LLVMInt32Type(), 1, false), "nextval");
    LLVMBuildStore(ctx->builder, next_val, iter);
    LLVMBuildBr(ctx->builder, for_cond); //jump back to the condition

    //finished the loop
    LLVMPositionBuilderAtEnd(ctx->builder, for_end);
}

static void code_gen_while_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;

    AST_while_node* node = (AST_while_node*) root;

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx-> builder));

    LLVMBasicBlockRef while_cond   = LLVMAppendBasicBlock(current_function, "while_cond");
    LLVMBasicBlockRef while_body   = LLVMAppendBasicBlock(current_function, "while_body");
    LLVMBasicBlockRef while_end    = LLVMAppendBasicBlock(current_function, "while_end");

    LLVMBuildBr(ctx->builder, while_cond);
    LLVMPositionBuilderAtEnd(ctx->builder, while_cond);

    LLVMValueRef cond = code_gen_exp(ctx, node -> cond);
    if (!AST_IS_BOOL_TYPE(node -> cond))
    {
        fprintf(stderr,"Error: the condition for the if statement is not a booleen\n");
        exit(3);
    }
    LLVMBuildCondBr(ctx->builder, cond, while_body, while_end);

    LLVMPositionBuilderAtEnd(ctx->builder, while_body);
    code_gen_stmt(ctx, node -> statements);
    if (!ctx->current_block_terminated)
        LLVMBuildBr(ctx->builder, while_cond);
    ctx->current_block_terminated = false;
    LLVMPositionBuilderAtEnd(ctx->builder, while_end);

}

static void code_gen_dowhile_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;

    AST_dowhile_node* node = (AST_dowhile_node*) root;

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx->builder));

    LLVMBasicBlockRef dowhile_body   = LLVMAppendBasicBlock(current_function, "dowhile_body");
    LLVMBasicBlockRef dowhile_cond   = LLVMAppendBasicBlock(current_function, "dowhile_cond");
    LLVMBasicBlockRef dowhile_end    = LLVMAppendBasicBlock(current_function, "dowhile_end");

    LLVMBuildBr(ctx->builder, dowhile_body);
    LLVMPositionBuilderAtEnd(ctx->builder, dowhile_body);
    code_gen_stmt(ctx, node -> statements);
    if (!ctx->current_block_terminated)
        LLVMBuildBr(ctx->builder, dowhile_cond);
    ctx->current_block_terminated = false;

    LLVMPositionBuilderAtEnd(ctx->builder, dowhile_cond);
    LLVMValueRef cond = code_gen_exp(ctx, node -> cond);
    if (!AST_IS_BOOL_TYPE(node -> cond))
    {
        fprintf(stderr,"Error: the condition for the if statement is not a booleen\n");
        exit(3);
    }
    LLVMBuildCondBr(ctx->builder, cond, dowhile_end, dowhile_body);

    LLVMPositionBuilderAtEnd(ctx->builder, dowhile_end);
}

#define FMT_LEN 512
#define MAX_PRINT_ARGS 128
static void gen_fmtstr(Type** args_type, size_t args_count, char* fmt_str)
{
    if (args_count >= MAX_PRINT_ARGS)
    {
        fprintf(stderr, "Error: a big number of arguments\n");
        exit(3);
    }

    for (size_t i = 0; i < args_count; i++)
    {
        const char* frag = NULL;
        if (!TYPE_IS_PRIMITIVE(args_type[i]))
        {
            fprintf(stderr, "Error: can't print non primitive types\n");
            exit(3);
        }
        switch (((Primitive_type*)args_type[i])->val_type)
        {
            case VAL_INT:   frag = "%d"; break;
        case VAL_FLOAT: frag = "%f"; break;
            case VAL_BOOL:  frag = "%s"; break;
            case VAL_CHAR:  frag = "%c"; break;
            case VAL_ERR:
                fprintf(stderr, "Error: bad type\n");
                break;
        }

        if (i > 0) strncat(fmt_str, " ", FMT_LEN - strlen(fmt_str) - 1);
        strncat(fmt_str, frag, FMT_LEN - strlen(fmt_str) - 1);
    }
    strncat(fmt_str, "\n", FMT_LEN - strlen(fmt_str) - 1);
}

static void code_gen_print_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;

    AST_print_node* print = (AST_print_node*)root;
    AST_args_node* args = (AST_args_node*)print->args;

    size_t args_count = 0;
    Type** args_type = NULL;
    LLVMValueRef* args_val = NULL;

    if (args != NULL)
    {
        args_count = LL_size(args->args_list);
        args_type = malloc(args_count * sizeof(Type*));
        args_val = malloc(args_count * sizeof(LLVMValueRef));
        size_t index = 0;
        LL_FOR_EACH(args->args_list, ll_node)
        {
            AST_arg_node* arg = ll_node->data;
            args_val[index] = code_gen_exp(ctx, arg->exp);
            args_type[index] = ast_exp_type(arg->exp);
            index++;
        }
    }

    char fmt_str[FMT_LEN] = {0};
    gen_fmtstr(args_type, args_count, fmt_str);
    LLVMValueRef fmt_global = LLVMBuildGlobalStringPtr(ctx->builder, fmt_str, "fmtstr");

    size_t printf_args_count = args_count + 1;

    LLVMValueRef* printf_args = malloc(printf_args_count * sizeof(LLVMValueRef));
    printf_args[0] = fmt_global;

    for (size_t i = 0; i < args_count; i++)
    {
        if (type_equal(args_type[i], TYPE_BOOL))
        {
            LLVMValueRef true_str = LLVMBuildGlobalStringPtr(ctx->builder, "vrai", "true_str");
            LLVMValueRef false_str = LLVMBuildGlobalStringPtr(ctx->builder, "faux", "false_str");

            LLVMValueRef is_true = LLVMBuildICmp(ctx->builder, LLVMIntNE, args_val[i],
                                    LLVMConstInt(LLVMInt1Type(), 0, 0), "bool_cmp");
            printf_args[i + 1] = LLVMBuildSelect(ctx->builder, is_true, true_str, false_str, "bool_str");
        }
        else if (type_equal(args_type[i], TYPE_FLOAT))
        {
            /* cast float to double so printf can print it */
            LLVMValueRef double_cast = LLVMBuildFPExt(ctx->builder, args_val[i], LLVMDoubleType(), "double_cast_tmp");
            printf_args[i + 1] = double_cast;
        }
        else
        {
            printf_args[i + 1] = args_val[i];
        }

    }

    LLVMBuildCall2(ctx->builder, ctx->printf_type, ctx->printf_ref, printf_args, printf_args_count, "print_calltmp");

    //clean up
    free(printf_args);
    free(args_type);
    free(args_val);
}

void code_gen_stmt(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return;

    if (ctx->current_block_terminated)
    {
        LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx-> builder));
        LLVMBasicBlockRef cont = LLVMAppendBasicBlock(current_function, "after_ret");
        LLVMPositionBuilderAtEnd(ctx->builder, cont);

        ctx->current_block_terminated = false;
    }


    switch (root -> type)
    {
        case NODE_STATEMENTS:
            {
                //iterate over every statement in the linked list
                LL_FOR_EACH(((AST_statements_node*)root) -> stmts_list, ll_node)
                {
                    code_gen_stmt(ctx, ll_node -> data);
                }
            }
            break;
        case NODE_ASSIGN:
            {
                AST_assign_node* node = (AST_assign_node*)root;

                LLVMValueRef dest_ref = code_gen_lval(ctx, node -> dest);
                LLVMValueRef val_ref = code_gen_exp(ctx, node -> assign_exp);

                Type* dest_type = ast_exp_type(node -> dest);
                Type* exp_type = ast_exp_type(node -> assign_exp);

                Type* assign_type = type_resolve_assign(dest_type, exp_type);

                LLVMValueRef cexp = code_gen_promote(ctx, val_ref, exp_type, assign_type);


                LLVMBuildStore(ctx->builder, cexp, dest_ref);
            }
            break;
        case NODE_IF:
            code_gen_if_stmt(ctx, root);
            break;
        case NODE_FOR:
            code_gen_for_stmt(ctx, root);
            break;
        case NODE_WHILE:
            code_gen_while_stmt(ctx, root);
            break;
        case NODE_DOWHILE:
            code_gen_dowhile_stmt(ctx, root);
            break;
        case NODE_RETURN:
            {
                if (!ctx->current_fn_ret_type)
                {
                    fprintf(stderr,"Error: return statement in void function\n");
                    exit(3);
                }

                AST_return_node* node = (AST_return_node*)root;
                LLVMValueRef ret_ref = code_gen_exp(ctx, node->exp);
                Type* ret_type = ast_exp_type(node->exp);
                if (!type_equal(ret_type, ctx->current_fn_ret_type))
                {
                    fprintf(stderr,"Error: return statement with wrong type\n");
                    exit(3);
                }

                LLVMBuildRet(ctx->builder, ret_ref);
                ctx->current_block_terminated = true;
            }
            break;
        case NODE_PRINT:
            code_gen_print_stmt(ctx, root);
            break;
        default:

        fprintf(stderr, "Error : bad ast node not a statement\n");
        exit(3);
    }
}
