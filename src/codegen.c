#include "codegen.h"

//no need for a context 
static LLVMModuleRef module; 
static LLVMBuilderRef builder; 

/* I am not a fan of global variables I think I will use a context instead */ 
static Symbol_table* global_sym_tab  = NULL; 
static Symbol_table* current_sym_tab = NULL; /* represent local symbol table */ 
static Type* current_fn_ret_type    = NULL; 
bool current_block_terminated = false; 
static LLVMTypeRef printf_type; 
static LLVMValueRef printf_ref; 

// note: both code_gen_id and code_gen_exp can take id_node as input 
//      first one returns a pointer for assignment 
//      and the other retunrs a value for expressions
static LLVMValueRef cast_if_needed(LLVMValueRef value, Type* val_type, Type* dest_type); 
static void code_gen_subprograms(AST_node* subprograms); 
static void code_gen_function(AST_node* function); 
static void populate_sym_table(AST_node* decls); 
static void code_gen_stmt(AST_node* root); 
static void code_gen_for_stmt(AST_node* root); 
static void insert_last_block_if_needed(Linkedlist* buffer); /* helper function for if code gen */ 
static void code_gen_if_stmt(AST_node* root); 
static void code_gen_while_stmt(AST_node* root); 
static void code_gen_dowhile_stmt(AST_node* root); 
static void code_gen_print_stmt(AST_node* root); 
static LLVMValueRef code_gen_id(AST_node* root); 
static LLVMValueRef code_gen_op(AST_node* root); 
static LLVMValueRef code_gen_call(AST_node* root); 
static LLVMValueRef code_gen_exp(AST_node* root); 
static St_entry* find_var(char * name); 
static St_entry* find_fun(char * name, Type** args, size_t args_count); 

void code_gen_init()
{
    //init llvm 
    module = LLVMModuleCreateWithName("main_module"); 
    builder = LLVMCreateBuilder(); 

    //set up the symbol table 
    global_sym_tab = st_create(); 

    //define builtins functions 
    builtins_init(module); 
    builtins_add_to_symtab(global_sym_tab); 

    //declare external functions like printf 

    /* printf 
     * return type : int
     * argument : (char* , ...)
     * isVarArg : true 
     */
    LLVMTypeRef printf_arg_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    printf_type = LLVMFunctionType(LLVMInt32Type(), printf_arg_types, 1, true); 
    printf_ref = LLVMAddFunction(module, "printf", printf_type); 

}

void code_gen_cleanup()
{
    //cleanup llvm 
    LLVMDisposeBuilder(builder); 
    LLVMDisposeModule(module); 

    //free the symbol table
    st_free(global_sym_tab); 
}

static St_entry* find_var(char* name)
{
    St_entry* var = st_find_var(current_sym_tab, name); 
    if (var)
        return var; 

    /* if it's the main function dont research in the global sym tab */ 
    if (current_sym_tab == global_sym_tab)
        return NULL; 

    return st_find_var(global_sym_tab, name); 
}

/* for now search only the global table */ 
static St_entry* find_fun(char* name, Type** args, size_t args_count)
{
    return st_find_fun(global_sym_tab, name, args, args_count); 
}


static LLVMValueRef cast_if_needed(LLVMValueRef value, Type* val_type, Type* dest_type)
{
    if (value == NULL || val_type == NULL || dest_type == NULL)
        return NULL;

    if (type_equal(val_type, dest_type))
        return value;

    if (val_type->kind != TYPE_PRIMITIVE || dest_type->kind != TYPE_PRIMITIVE)
    {
        fprintf(stderr, "Cannot cast non-primitive types\n");
        exit(3);
    }

    Primitive_type* from = (Primitive_type*)val_type;
    Primitive_type* to = (Primitive_type*)dest_type;

    if (from->val_type == VAL_INT && to->val_type == VAL_FLOAT)
    {
        return LLVMBuildSIToFP(builder, value, LLVMFloatType(), "casted_float");
    }

    fprintf(stderr, "Unsupported cast from type %d to type %d\n", from->val_type, to->val_type);
    exit(3);
    return NULL;
}

static void populate_sym_table(AST_node* decls)
{
    if (!decls)
        return; 
    LL_FOR_EACH(((AST_declarations_node*)decls) -> var_decls_list, ll_node)
    {
        AST_var_declaration_node* decl_node = (AST_var_declaration_node*)ll_node -> data; 
        AST_id_node* id_node = (AST_id_node*)(decl_node -> id_node); 

        LLVMValueRef id_alloca = LLVMBuildAlloca(builder, type_to_llvm_type(decl_node->id_type), id_node->id_str);
        if (st_insert_var(current_sym_tab, id_node->id_str, decl_node->id_type, id_alloca)
                                                == ST_ALREADY_DECLARED)
        {
            fprintf(stderr, "Error : variable %s declared twice\n", id_node -> id_str); 
            exit(3); 
        }
    }
}

// resolve types first then cast left and right nodes if needed then do the operations
static LLVMValueRef code_gen_op(AST_node* root)
{
    if (root == NULL)
        return NULL; 

    AST_op_node* node = (AST_op_node*) root;

    LLVMValueRef left = code_gen_exp(node -> lhs); 
    LLVMValueRef right = code_gen_exp(node -> rhs); 
     
    Type* left_node_type =  ast_exp_type(node -> lhs); 
    Type* right_node_type =  ast_exp_type(node -> rhs); 

    //resolve types
    Type* node_type = type_resolve_op(left_node_type, right_node_type, node -> op_type) ; 

    node->res_type = op_rel(node -> op_type) ? TYPE_BOOL :node_type; 

    //cast left and right
    LLVMValueRef cleft = cast_if_needed(left,  left_node_type, node_type); 
    LLVMValueRef cright = cast_if_needed(right,  right_node_type, node_type); 
    
    //do the operation 
    Value_type node_val_type = ((Primitive_type*)node_type)->val_type; 
    switch (node -> op_type)
    {
        case OP_ADD: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFAdd(builder, cleft, cright, "addtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildAdd(builder, cleft, cright, "faddtmp"); 
            break; 
        case OP_SUB: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFSub(builder, cleft, cright, "subtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildSub(builder, cleft, cright, "fsubtmp"); 
            break; 
        case OP_MUL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFMul(builder, cleft, cright, "multmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildMul(builder, cleft, cright, "fmultmp"); 
            break; 
        case OP_DIV: 
            return LLVMBuildFDiv(builder, cleft, cright, "multmp"); 
        case OP_IDIV: 
            return LLVMBuildSDiv(builder, cleft, cright, "idivtmp"); 
        case OP_MOD: 
            return LLVMBuildSRem(builder, cleft, cright, "modtmp"); 
        case OP_UMIN: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFNeg(builder, cleft, "fnegtmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildNeg(builder, cleft, "negtmp"); 
            break; 


        case OP_GREATER: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealOGT, cleft, cright, "fgtcmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(builder, LLVMIntSGT, cleft, cright, "gtcmptmp"); 
            break; 
        case OP_LESS: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealOLT, cleft, cright, "fltcmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(builder, LLVMIntSLT, cleft, cright, "ltcmptmp"); 
            break; 
        case OP_GREATER_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealOGE, cleft, cright, "fgecmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(builder, LLVMIntSGE, cleft, cright, "gecmptmp"); 
            break; 
        case OP_LESS_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealOLE, cleft, cright, "flecmptmp"); 
            else if (node_val_type == VAL_INT)
                return LLVMBuildICmp(builder, LLVMIntSLE, cleft, cright, "lecmptmp"); 
            break; 
        case OP_EQUAL:  
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealOEQ, cleft, cright, "feqcmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_BOOL 
                    || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(builder, LLVMIntEQ, cleft, cright, "eqcmptmp"); 
            break; 
        case OP_NOT_EQUAL: 
            if (node_val_type == VAL_FLOAT)
                return LLVMBuildFCmp(builder, LLVMRealONE, cleft, cright, "fnecmptmp"); 
            else if (node_val_type == VAL_INT || node_val_type == VAL_BOOL 
                    || node_val_type == VAL_CHAR)
                return LLVMBuildICmp(builder, LLVMIntNE, cleft, cright, "necmptmp"); 
            break; 

        case OP_AND: 
                return LLVMBuildAnd(builder, cleft, cright, "andtemp"); 
        case OP_OR: 
                return LLVMBuildOr(builder, cleft, cright, "ortemp"); 
        case OP_NOT: 
                return LLVMBuildNot(builder, cleft, "nottemp"); 
                

        default: 
            fprintf(stderr, "Error: bad node not an operation for now only integer operations\n"); 
            exit(3); 
            
    }
    return NULL; 
}

static LLVMValueRef code_gen_call(AST_node* root)
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
            args_val[index] = code_gen_exp(arg->exp); 
            args_type[index] = ast_exp_type(arg->exp); 
            index++; 
        }
    }

    /* search the function in the symbol table */ 
    char* fun_name = ((AST_id_node*)call->id_node)->id_str; 
    St_entry* fn_entry = find_fun(fun_name, args_type, args_count); 
    if (!fn_entry)
    {
        fprintf(stderr, "Error: %s function is not declared or args does not match\n", fun_name); 
        exit(3); 
    }

    LLVMValueRef result =  LLVMBuildCall2(builder, fn_entry->type_ref, fn_entry->value_ref, args_val, args_count, "calltemp"); 

    call->fun_type = fn_entry->type; 
    call->ret_type = ((Function_type*)call->fun_type)->return_type; 

    /* clean up */ 
    free(args_val); 
    free(args_type); 
    return result; 
}

//also include type resolution
static LLVMValueRef code_gen_exp(AST_node* root)
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
                St_entry* entry = find_var(node -> id_str); 
                if (entry == NULL)
                {
                    fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
                    exit(3); 
                }

                node -> id_type = entry -> type; 

                return LLVMBuildLoad2(builder, type_to_llvm_type(entry -> type), 
                        entry -> value_ref, "loaded_var"); 
            }
        case NODE_OP: 
            return code_gen_op(root); 
        case NODE_CALL: 
            return code_gen_call(root); 
        default: 
            fprintf(stderr, "Error: bad ast node not an expression.\n"); 
            exit(3); 

    }
    return NULL; 
}

static LLVMValueRef code_gen_id(AST_node* root)
{
    if (root == NULL)
        return NULL; 

    AST_id_node* node = (AST_id_node*)root; 
    St_entry* entry = find_var(node -> id_str); 
    if (entry == NULL)
    {
        fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
        exit(3); 
    }
    node -> id_type = entry -> type; 
    return entry -> value_ref; 
}

static void insert_last_block_if_needed(Linkedlist* buffer) 
{
    if (!current_block_terminated)
    {
        LLVMBasicBlockRef last_block = LLVMGetInsertBlock(builder);
        LL_insert_back(buffer, last_block); 
    }
    current_block_terminated = false; 
}

static void code_gen_if_stmt(AST_node* root)
{
    if (root == NULL)
        return;  
    AST_if_node* node = (AST_if_node*) root; 
    AST_elif_node* elif_node = (AST_elif_node*) node->elif_branches; 

    Linkedlist* merge_buffer = LL_create_list(); 

    //get the function from the builder position 
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef if_block = LLVMAppendBasicBlock(current_function, "if_block"); 
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then_block"); 

    LLVMBuildBr(builder, if_block); 

    LLVMPositionBuilderAtEnd(builder, then_block); 
    code_gen_stmt(node -> action); 
    insert_last_block_if_needed(merge_buffer); 
    LLVMPositionBuilderAtEnd(builder, if_block); 

    LLVMValueRef prev_cond = code_gen_exp(node -> cond); 
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

            LLVMBuildCondBr(builder, prev_cond, prev_then_block, elif_block); 
            LLVMPositionBuilderAtEnd(builder, then_block); 
            code_gen_stmt(branch_node -> action); 
            insert_last_block_if_needed(merge_buffer); 
            LLVMPositionBuilderAtEnd(builder, elif_block); 

            prev_cond = code_gen_exp(branch_node -> cond); 
            if (!type_equal(ast_exp_type(branch_node -> cond),TYPE_BOOL))
            {
                fprintf(stderr,"Error: the condition for the if statement is not a booleen\n"); 
                exit(3); 
            }
            prev_then_block = then_block; 
        }
    }

    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "else_block"); 

    LLVMBuildCondBr(builder, prev_cond, prev_then_block, else_block); 

    LLVMPositionBuilderAtEnd(builder, else_block); 
    code_gen_stmt(node -> else_action); 
    insert_last_block_if_needed(merge_buffer); 

    //mergin all then blocks
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge_block"); 
    LL_FOR_EACH(merge_buffer, ll_node)
    {
        //cast it back from void* to basicblockref
        LLVMBasicBlockRef then_block = ll_node -> data; 
        LLVMPositionBuilderAtEnd(builder, then_block); 
        LLVMBuildBr(builder, merge_block); 
    }
    LLVMPositionBuilderAtEnd(builder, merge_block); 
    LL_free_list(&merge_buffer, NULL); 
}

static void code_gen_for_stmt(AST_node* root)
{
    if (root == NULL)
        return; 

    AST_for_node* node = (AST_for_node*) root; 

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef for_cond   = LLVMAppendBasicBlock(current_function, "for_cond"); 
    LLVMBasicBlockRef for_body   = LLVMAppendBasicBlock(current_function, "for_body"); 
    LLVMBasicBlockRef for_inc    = LLVMAppendBasicBlock(current_function, "for_inc"); 
    LLVMBasicBlockRef for_end    = LLVMAppendBasicBlock(current_function, "for_end"); 


    //check all variables are integers 

    LLVMValueRef iter = code_gen_id(node -> iter); 
    LLVMValueRef from = code_gen_exp(node -> from); 
    LLVMValueRef to   = code_gen_exp(node -> to); 
    if (!AST_IS_INT_TYPE(node -> iter) || !AST_IS_INT_TYPE(node -> from) || !AST_IS_INT_TYPE(node -> to))
    {
        fprintf(stderr,"Error: Can't work with non integers in a for loop\n"); 
        exit(3); 
    }

    //initilize the iterator
    LLVMBuildStore(builder, from, iter); 

    LLVMBuildBr(builder, for_cond); 
    LLVMPositionBuilderAtEnd(builder, for_cond);

    //for loop condition 
    LLVMPositionBuilderAtEnd(builder, for_cond);
    LLVMValueRef iter_val = LLVMBuildLoad2(builder, LLVMInt32Type(), iter, "iter_val");
    LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntSLE, iter_val, to, "for_cond");
    LLVMBuildCondBr(builder, cond, for_body, for_end);

    //body of the loop 
    LLVMPositionBuilderAtEnd(builder, for_body);
    code_gen_stmt(node->statements); 
    if (!current_block_terminated)
        LLVMBuildBr(builder, for_inc);
    current_block_terminated = false; 

    //increment block
    LLVMPositionBuilderAtEnd(builder, for_inc);
    LLVMValueRef next_val = LLVMBuildAdd(builder, iter_val, LLVMConstInt(LLVMInt32Type(), 1, false), "nextval");
    LLVMBuildStore(builder, next_val, iter);
    LLVMBuildBr(builder, for_cond); //jump back to the condition

    //finished the loop 
    LLVMPositionBuilderAtEnd(builder, for_end);
    
}

static void code_gen_while_stmt(AST_node* root)
{
    if (root == NULL)
        return; 

    AST_while_node* node = (AST_while_node*) root; 

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef while_cond   = LLVMAppendBasicBlock(current_function, "while_cond"); 
    LLVMBasicBlockRef while_body   = LLVMAppendBasicBlock(current_function, "while_body"); 
    LLVMBasicBlockRef while_end    = LLVMAppendBasicBlock(current_function, "while_end"); 

    LLVMBuildBr(builder, while_cond); 
    LLVMPositionBuilderAtEnd(builder, while_cond);

    LLVMValueRef cond = code_gen_exp(node -> cond); 
    if (!AST_IS_BOOL_TYPE(node -> cond))
    {
        fprintf(stderr,"Error: the condition for the if statement is not a booleen\n"); 
        exit(3); 
    }
    LLVMBuildCondBr(builder, cond, while_body, while_end); 

    LLVMPositionBuilderAtEnd(builder, while_body);
    code_gen_stmt(node -> statements); 
    if (!current_block_terminated)
        LLVMBuildBr(builder, while_cond);
    current_block_terminated = false; 
    LLVMPositionBuilderAtEnd(builder, while_end);
    
}

static void code_gen_dowhile_stmt(AST_node* root)
{
    if (root == NULL)
        return; 

    AST_dowhile_node* node = (AST_dowhile_node*) root; 

    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef dowhile_body   = LLVMAppendBasicBlock(current_function, "dowhile_body"); 
    LLVMBasicBlockRef dowhile_cond   = LLVMAppendBasicBlock(current_function, "dowhile_cond"); 
    LLVMBasicBlockRef dowhile_end    = LLVMAppendBasicBlock(current_function, "dowhile_end"); 

    LLVMBuildBr(builder, dowhile_body); 
    LLVMPositionBuilderAtEnd(builder, dowhile_body);
    code_gen_stmt(node -> statements); 
    if (!current_block_terminated)
        LLVMBuildBr(builder, dowhile_cond);
    current_block_terminated = false; 

    LLVMPositionBuilderAtEnd(builder, dowhile_cond);
    LLVMValueRef cond = code_gen_exp(node -> cond); 
    if (!AST_IS_BOOL_TYPE(node -> cond))
    {
        fprintf(stderr,"Error: the condition for the if statement is not a booleen\n"); 
        exit(3); 
    }
    LLVMBuildCondBr(builder, cond, dowhile_end, dowhile_body); 

    LLVMPositionBuilderAtEnd(builder, dowhile_end);
}

#define FMT_LEN 512
#define MAX_PRINT_ARGS 128
static char* gen_fmtstr(Type** args_type, size_t args_count)
{
    char fmt_str[FMT_LEN] = {0}; 
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

    return strdup(fmt_str); 
}

/* print will be a wrapper around printf */
static void code_gen_print_stmt(AST_node* root)
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
            args_val[index] = code_gen_exp(arg->exp); 
            args_type[index] = ast_exp_type(arg->exp); 
            index++; 
        }
    }

    char* fmt_str = gen_fmtstr(args_type, args_count); 
    LLVMValueRef fmt_global = LLVMBuildGlobalStringPtr(builder, fmt_str, "fmtstr");
    free(fmt_str); 

    size_t printf_args_count = args_count + 1;  

    LLVMValueRef* printf_args = malloc(printf_args_count * sizeof(LLVMValueRef)); 
    printf_args[0] = fmt_global; 

    for (size_t i = 0; i < args_count; i++)
    {
        if (type_equal(args_type[i], TYPE_BOOL))
        {
            LLVMValueRef true_str = LLVMBuildGlobalStringPtr(builder, "vrai", "true_str");
            LLVMValueRef false_str = LLVMBuildGlobalStringPtr(builder, "faux", "false_str");

            LLVMValueRef is_true = LLVMBuildICmp(builder, LLVMIntNE, args_val[i],
                                    LLVMConstInt(LLVMInt1Type(), 0, 0), "bool_cmp");
            printf_args[i + 1] = LLVMBuildSelect(builder, is_true, true_str, false_str, "bool_str");
        }
        else 
        {
            printf_args[i + 1] = args_val[i];
        }

    }

    LLVMBuildCall2(builder, printf_type, printf_ref, printf_args, printf_args_count, "print_calltmp"); 

    //clean up 
    free(printf_args); 
    free(args_type); 
    free(args_val); 
}

static void code_gen_stmt(AST_node* root)
{
    if (root == NULL)
        return; 

    if (current_block_terminated) 
    {   
        LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
        LLVMBasicBlockRef cont = LLVMAppendBasicBlock(current_function, "after_ret");
        LLVMPositionBuilderAtEnd(builder, cont);

        current_block_terminated = false;      
    }


    switch (root -> type)
    {
        case NODE_STATEMENTS:  
            {
                //iterate over every statement in the linked list
                LL_FOR_EACH(((AST_statements_node*)root) -> stmts_list, ll_node)
                {
                    code_gen_stmt(ll_node -> data); 
                }
            }
            break; 
        case NODE_ASSIGN: 
            {
                AST_assign_node* node = (AST_assign_node*)root; 

                LLVMValueRef dest_ref = code_gen_id(node -> dest); 
                LLVMValueRef val_ref = code_gen_exp(node -> assign_exp); 

                Type* dest_type = ast_exp_type(node -> dest); 
                Type* exp_type = ast_exp_type(node -> assign_exp); 

                Type* assign_type = type_resolve_assign(dest_type, exp_type); 

                LLVMValueRef cexp = cast_if_needed(val_ref, exp_type, assign_type); 


                LLVMBuildStore(builder, cexp, dest_ref); 
            }
            break; 
        case NODE_IF: 
            code_gen_if_stmt(root);
            break; 
        case NODE_FOR: 
            code_gen_for_stmt(root);
            break; 
        case NODE_WHILE: 
            code_gen_while_stmt(root);
            break; 
        case NODE_DOWHILE: 
            code_gen_dowhile_stmt(root);
            break; 
        case NODE_RETURN: 
            {
                if (!current_fn_ret_type)
                {
                    fprintf(stderr,"Error: return statement in void function\n"); 
                    exit(3); 
                }

                AST_return_node* node = (AST_return_node*)root; 
                LLVMValueRef ret_ref = code_gen_exp(node->exp); 
                Type* ret_type = ast_exp_type(node->exp); 
                if (!type_equal(ret_type, current_fn_ret_type)) 
                {
                    fprintf(stderr,"Error: return statement with wrong type\n"); 
                    exit(3); 
                }

                LLVMBuildRet(builder, ret_ref);  
                current_block_terminated = true; 
            }
            break; 
        case NODE_PRINT: 
            code_gen_print_stmt(root); 
            break; 
        default: 
            
        fprintf(stderr, "Error : bad ast node not a statement\n"); 
        exit(3); 
    }
}

static void code_gen_subprograms(AST_node* subprograms)
{
    AST_subprograms_node* node = (AST_subprograms_node*)subprograms; 
    LL_FOR_EACH(node->functions_list, ll_node)
    {
        AST_node* fn_node = ll_node->data;  
        code_gen_function(fn_node); 
    }
}

static void code_gen_function(AST_node* function)
{
    AST_function_node* fn = (AST_function_node*)function; 
    AST_params_node* params = (AST_params_node*)fn->params; 
    size_t params_count = 0; 
    Type** param_types = NULL; 
    char** param_names = NULL; 
    LLVMTypeRef* llvm_param_types = NULL; 

    /* iterate over every paramater */ 
    if (params != NULL)
    {
        params_count = LL_size(params->params_list); 
        param_types = malloc(params_count * sizeof(Type*));
        param_names = malloc(params_count * sizeof(char*)); 
        llvm_param_types = malloc(params_count * sizeof(LLVMTypeRef));
        size_t index = 0; 
        LL_FOR_EACH(params->params_list, ll_node)        
        {
            AST_param_node* param = ll_node->data; 
            param_names[index] = ((AST_id_node*)param->id_node)->id_str; 
            param_types[index] = param->id_type;  
            llvm_param_types[index] = type_to_llvm_type(param_types[index]); 
            index++; 
        }
    }

    
    /* create function type and insert it into the global symbol table */ 
    char* fun_name = ((AST_id_node*)fn->id_node)->id_str; 
    Type* func_type = type_function_create(fn->ret_type, param_types, params_count);
    LLVMTypeRef llvm_func_type = LLVMFunctionType(type_to_llvm_type(fn->ret_type), llvm_param_types, params_count, 0); 
    LLVMValueRef func_ref= LLVMAddFunction(module, fun_name, llvm_func_type); 
    if (st_insert_fun(global_sym_tab, fun_name, func_type, func_ref, llvm_func_type) 
            == ST_ALREADY_DECLARED)
    {
            fprintf(stderr, "Error : function %s defined twice\n", fun_name); 
            exit(3); 
    }

    /* create a local symbol table */ 
    current_sym_tab = st_create(); 
    current_fn_ret_type = fn->ret_type; 
    
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func_ref, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMValueRef param_alloca; 
    for (size_t i = 0; i < params_count; i++) 
    {
        param_alloca = LLVMBuildAlloca(builder, llvm_param_types[i], param_names[i]);
        LLVMBuildStore(builder, LLVMGetParam(func_ref, i), param_alloca);
        st_insert_var(current_sym_tab, param_names[i], param_types[i], param_alloca);
    }

    /* populate local sym table */ 
    populate_sym_table(fn->declarations); 

    /* generate statments */ 
    code_gen_stmt(fn->statements); 

    if (!current_block_terminated)
    {
        fprintf(stderr, "Error: missing a return statement\n"); 
        exit(3); 
    }


    /*clean up */ 
    free(param_types); 
    free(llvm_param_types); 
    free(param_names); 
    st_free(current_sym_tab); /* free the local symbol table */ 
    current_sym_tab = NULL; 
    current_fn_ret_type = NULL;
    current_block_terminated = false; 
}

void code_gen_ir(AST_node* program_node)
{
    //code generating subprograms 
    code_gen_subprograms(((AST_program_node*)program_node)->subprograms); 
    
    //creating a main function without arguments
    LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_function = LLVMAddFunction(module, "main", ret_type);

    //creating entry basic block 
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    //allocate variables in the stack
    //* it's the main function there is no local symtoble so make it point to the gloable table *//  
    current_sym_tab = global_sym_tab; 
    populate_sym_table(((AST_program_node*) program_node) -> declarations); 

    //statements ir generation
    code_gen_stmt(((AST_program_node*) program_node) -> statements); 

    //main function return 
    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
    //verify the main module
    char* error = NULL; 
    if (LLVMVerifyModule(module, LLVMAbortProcessAction, &error))
    {
        fprintf(stderr, "Error : %s\n", error); 
       exit(3); 
    }
    LLVMDisposeMessage(error); 
    //print the final ir to a file  
    LLVMPrintModuleToFile(module, "out.ll", NULL); 

}
