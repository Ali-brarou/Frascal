#include <codegen.h> 

static LLVMValueRef code_gen_op(Codegen_ctx *ctx, AST_node* root);
static LLVMValueRef code_gen_call(Codegen_ctx *ctx, AST_node* root);
static LLVMValueRef code_gen_arr_sub(Codegen_ctx *ctx, AST_node* root);
static LLVMValueRef code_gen_mat_sub(Codegen_ctx *ctx, AST_node* root);

static LLVMValueRef code_gen_op(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return NULL; 

    AST_op_node* node = (AST_op_node*) root;

    LLVMValueRef left = code_gen_exp(ctx, node -> lhs); 
    LLVMValueRef right = code_gen_exp(ctx, node -> rhs); 
     
    Type* left_node_type =  ast_exp_type(node -> lhs); 
    Type* right_node_type =  ast_exp_type(node -> rhs); 

    //resolve types
    Type* node_type = type_resolve_op(left_node_type, right_node_type, node -> op_type) ; 

    node->res_type = op_rel(node -> op_type) ? TYPE_BOOL :node_type; 

    //cast left and right
    LLVMValueRef cleft = code_gen_promote(ctx, left,  left_node_type, node_type); 
    LLVMValueRef cright = code_gen_promote(ctx, right,  right_node_type, node_type); 
    
    //do the operation 
    Value_type node_val_type = ((Primitive_type*)node_type)->val_type; 
    switch (node -> op_type)
    {
        case OP_ADD: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFAdd(ctx->builder, cleft, cright, "addtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildAdd(ctx->builder, cleft, cright, "faddtmp"); 
            break; 
        case OP_SUB: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFSub(ctx->builder, cleft, cright, "subtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildSub(ctx->builder, cleft, cright, "fsubtmp"); 
            break; 
        case OP_MUL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFMul(ctx->builder, cleft, cright, "multmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildMul(ctx->builder, cleft, cright, "fmultmp"); 
            break; 
        case OP_DIV: 
            return LLVMBuildFDiv(ctx->builder, cleft, cright, "multmp"); 
        case OP_IDIV: 
            return LLVMBuildSDiv(ctx->builder, cleft, cright, "idivtmp"); 
        case OP_MOD: 
            return LLVMBuildSRem(ctx->builder, cleft, cright, "modtmp"); 
        case OP_UMIN: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFNeg(ctx->builder, cleft, "fnegtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildNeg(ctx->builder, cleft, "negtmp"); 
            break; 


        case OP_GREATER: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealOGT, cleft, cright, "fgtcmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(ctx->builder, LLVMIntSGT, cleft, cright, "gtcmptmp"); 
            break; 
        case OP_LESS: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealOLT, cleft, cright, "fltcmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(ctx->builder, LLVMIntSLT, cleft, cright, "ltcmptmp"); 
            break; 
        case OP_GREATER_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealOGE, cleft, cright, "fgecmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(ctx->builder, LLVMIntSGE, cleft, cright, "gecmptmp"); 
            break; 
        case OP_LESS_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealOLE, cleft, cright, "flecmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(ctx->builder, LLVMIntSLE, cleft, cright, "lecmptmp"); 
            break; 
        case OP_EQUAL:  
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealOEQ, cleft, cright, "feqcmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_BOOL 
                    || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(ctx->builder, LLVMIntEQ, cleft, cright, "eqcmptmp"); 
            break; 
        case OP_NOT_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(ctx->builder, LLVMRealONE, cleft, cright, "fnecmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_BOOL 
                    || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(ctx->builder, LLVMIntNE, cleft, cright, "necmptmp"); 
            break; 

        case OP_AND: 
                return LLVMBuildAnd(ctx->builder, cleft, cright, "andtemp"); 
        case OP_OR: 
                return LLVMBuildOr(ctx->builder, cleft, cright, "ortemp"); 
        case OP_NOT: 
                return LLVMBuildNot(ctx->builder, cleft, "nottemp"); 
                

        default: 
            fprintf(stderr, "Error: bad node not an operation for now only integer operations\n"); 
            exit(3); 
            
    }
    return NULL; 
}

static LLVMValueRef code_gen_call(Codegen_ctx *ctx, AST_node* root)
{
    size_t args_count = 0; 
    Type** args_type = NULL; 
    LLVMValueRef* args_val = NULL; 
    AST_call_node* call = (AST_call_node*)root; 
    AST_args_node* args = (AST_args_node*)call->args; 

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

    /* search the function in the symbol table */ 
    char* fun_name = ((AST_id_node*)call->id_node)->id_str; 
    St_entry* fn_entry = find_fun(ctx, fun_name, args_type, args_count); 
    if (!fn_entry)
    {
        fprintf(stderr, "Error: %s function is not declared or args does not match\n", fun_name); 
        exit(3); 
    }

    LLVMValueRef result =  LLVMBuildCall2(ctx->builder, fn_entry->type_ref, fn_entry->value_ref, args_val, args_count, "calltemp"); 

    call->fun_type = fn_entry->type; 
    call->ret_type = ((Function_type*)call->fun_type)->return_type; 

    /* clean up */ 
    free(args_val); 
    free(args_type); 
    return result; 
}

static LLVMValueRef code_gen_arr_sub(Codegen_ctx *ctx, AST_node* root)
{
    LLVMValueRef elem_ptr = code_gen_lval(ctx, root); 
    return LLVMBuildLoad2(ctx->builder, 
            type_to_llvm_type(((AST_arr_sub_node*)root)->elem_type), 
            elem_ptr, 
            "loaded_elem"); 
}

static LLVMValueRef code_gen_mat_sub(Codegen_ctx *ctx, AST_node* root)
{
    LLVMValueRef elem_ptr = code_gen_lval(ctx, root); 
    return LLVMBuildLoad2(ctx->builder, 
            type_to_llvm_type(((AST_mat_sub_node*)root)->elem_type), 
            elem_ptr, 
            "loaded_elem"); 
}

LLVMValueRef code_gen_exp(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return NULL; 
    switch (root -> type)
    {
        case NODE_CONST: 
            {
                AST_const_node* node = (AST_const_node*)root; 
                LLVMValueRef const_ret; 
                switch (node -> val_type)
                {
                    case VAL_INT: 
                        const_ret = LLVMConstInt(LLVMInt32Type(), node -> value.ival, true); 
                    break; 
                    case VAL_FLOAT: 
                        const_ret = LLVMConstReal(LLVMFloatType(), node -> value.fval); 
                    break; 
                    case VAL_BOOL: 
                        const_ret = LLVMConstInt(LLVMInt1Type(), node -> value.bval, false); 
                    break; 
                    case VAL_CHAR: 
                        const_ret = LLVMConstInt(LLVMInt8Type(), node -> value.cval, false); 
                    break; 
                    default: 
                        fprintf(stderr, "bad expression node\n"); 
                        exit(3); 
                    break; 
                }
                return const_ret; 
            }
        case NODE_ID:
            {
                AST_id_node* node = (AST_id_node*)root; 
                St_entry* entry = find_var(ctx, node -> id_str); 
                if (entry == NULL)
                {
                    fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
                    exit(3); 
                }

                node -> id_type = entry -> type; 

                return LLVMBuildLoad2(ctx->builder, type_to_llvm_type(entry -> type), 
                        entry -> value_ref, "loaded_var"); 
            }
        case NODE_OP: 
            return code_gen_op(ctx, root); 
        case NODE_CALL: 
            return code_gen_call(ctx, root); 
        case NODE_ARR_SUB: 
            return code_gen_arr_sub(ctx, root); 
        case NODE_MAT_SUB: 
            return code_gen_mat_sub(ctx, root); 
        default: 
            fprintf(stderr, "Error: bad ast node not an expression.\n"); 
            exit(3); 

    }
    return NULL; 
}

LLVMValueRef code_gen_lval(Codegen_ctx *ctx, AST_node* root)
{
    if (root == NULL)
        return NULL; 

    switch (root -> type)
    {
        case NODE_ID: 
        {
            AST_id_node* node = (AST_id_node*)root; 
            St_entry* entry = find_var(ctx, node -> id_str); 
            if (entry == NULL)
            {
                fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
                exit(3); 
            }
            node -> id_type = entry -> type; 
            return entry -> value_ref; 
        }
        break; 
        case NODE_ARR_SUB: 
        {

            AST_arr_sub_node* node = (AST_arr_sub_node*)root; 
            /* find the array from the symbol table */ 
            LLVMValueRef arr_ref = code_gen_lval(ctx, node->id_node); 
            Array_type* arr_type = (Array_type*)ast_exp_type(node->id_node); 
            if (!TYPE_IS_ARRAY((Type*)arr_type))
            {
                fprintf(stderr, "%s not an array\n", ((AST_id_node*)node->id_node)->id_str); 
                exit(3); 
            }
            node->elem_type = arr_type->element_type; 
            LLVMTypeRef llvm_arr_type = type_to_llvm_type((Type*)arr_type); 
            LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, false);
            LLVMValueRef idx[2] = {zero, code_gen_exp(ctx, node->exp)}; 
            if (!type_equal(ast_exp_type(node->exp), TYPE_INT))
            {
                fprintf(stderr, "index must be an integer\n"); 
                exit(3); 
           }
            return LLVMBuildGEP2(ctx->builder, llvm_arr_type, arr_ref, idx, 2, "arr_sub_item"); 
        }
        break; 
        case NODE_MAT_SUB: 
        {
            AST_mat_sub_node* node = (AST_mat_sub_node*)root; 
            /* find the matrix from the symbol table */ 
            LLVMValueRef mat_ref = code_gen_lval(ctx, node->id_node); 
            Matrix_type* mat_type = (Matrix_type*)ast_exp_type(node->id_node); 
            if (!TYPE_IS_MATRIX((Type*)mat_type))
            {
                fprintf(stderr, "%s not a matrix\n", ((AST_id_node*)node->id_node)->id_str); 
                exit(3); 
            }
            node->elem_type = mat_type->element_type;
            LLVMTypeRef llvm_mat_type = type_to_llvm_type((Type*)mat_type); 
            LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, false);
            LLVMValueRef idx[3] = {zero, code_gen_exp(ctx, node->exp[0]), code_gen_exp(ctx, node->exp[1])}; 
            if (!type_equal(ast_exp_type(node->exp[0]), TYPE_INT))
            {
                fprintf(stderr, "index must be an integer\n"); 
                exit(3); 
           }
            if (!type_equal(ast_exp_type(node->exp[1]), TYPE_INT))
            {
                fprintf(stderr, "index must be an integer\n"); 
                exit(3); 
           }
            return LLVMBuildGEP2(ctx->builder, llvm_mat_type, mat_ref, idx, 3, "mat_sub_item"); 
        }
        break; 
        default: 
            fprintf(stderr, "Not an lvalue\n"); 
            exit(3); 
    }

}
