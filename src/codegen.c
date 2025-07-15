#include "codegen.h"

//no need for a context 
static LLVMModuleRef module; 
static LLVMBuilderRef builder; 

static Symbol_table* sym_tab; 

// note: both code_gen_id and code_gen_exp can take id_node as input 
//      first one returns a pointer for assignment 
//      and the other retunrs a value for expressions
static LLVMTypeRef type_to_llvm_type(Type* type); 
static LLVMValueRef cast_if_needed(LLVMValueRef value, Value_type val_type, Value_type dest_type); 
static void populate_sym_table(AST_node* decls); 
static void code_gen_stmt(AST_node* root); 
static void code_gen_for_stmt(AST_node* root); 
static void code_gen_if_stmt(AST_node* root); 
static void code_gen_while_stmt(AST_node* root); 
static void code_gen_dowhile_stmt(AST_node* root); 
static LLVMValueRef code_gen_id(AST_node* root); 
static LLVMValueRef code_gen_op(AST_node* root); 
static LLVMValueRef code_gen_exp(AST_node* root); 

void code_gen_init()
{
    //init llvm 
    module = LLVMModuleCreateWithName("main_module"); 
    builder = LLVMCreateBuilder(); 

    //set up the symbol table 
    sym_tab = st_create(); 
}

void code_gen_cleanup()
{
    //cleanup llvm 
    LLVMDisposeBuilder(builder); 
    LLVMDisposeModule(module); 

    //free the symbol table
    st_free(sym_tab); 
}

static LLVMTypeRef type_to_llvm_type(Type* type)
{
    if (type->kind != TYPE_PRIMITIVE)
    {
        fprintf(stderr, "not implemented yet\n"); 
        exit(3); 
    }
    switch (((Primitive_type*)type) -> val_type)
    {
        case VAL_INT:
            return LLVMInt32Type(); 
        case VAL_FLOAT: 
            return LLVMFloatType(); 
        case VAL_BOOL: 
            return LLVMInt1Type(); 
        case VAL_CHAR: 
            return LLVMInt8Type(); 
        default: 
            fprintf(stderr, "Error: unkown type\n"); 
            exit(3); 
    }
    return NULL; 
}

static LLVMValueRef cast_if_needed(LLVMValueRef value, Value_type val_type, Value_type dest_type)
{
    if (value == NULL) 
        return NULL; 
    if (val_type  == dest_type)
        return value; 

    if (dest_type == VAL_FLOAT)
        return LLVMBuildSIToFP(builder, value, LLVMFloatType(), "casted_float"); 

    fprintf(stderr, "cant cast this node\n"); 
    exit(3); 
    return NULL; 
}

static void populate_sym_table(AST_node* decls)
{
    LL_FOR_EACH(((AST_declarations_node*)decls) -> var_decls_list, ll_node)
    {
        AST_var_declaration_node* decl_node = (AST_var_declaration_node*)ll_node -> data; 
        AST_id_node* id_node = (AST_id_node*)(decl_node -> id_node); 

        LLVMValueRef id_alloca = LLVMBuildAlloca(builder, type_to_llvm_type(decl_node->id_type), id_node->id_str);
        if (st_insert(sym_tab, id_node -> id_str, decl_node -> id_type, id_alloca))
        {
            fprintf(stderr, "Error : variable %s already declared\n", id_node -> id_str); 
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
     
    Value_type left_node_type =  ast_exp_val_type(node -> lhs); 
    Value_type right_node_type =  ast_exp_val_type(node -> rhs); 

    //resolve types
    Value_type node_val_type = type_resolve_op(left_node_type, right_node_type, node -> op_type) ; 

    node->res_type = type_primitive_create(op_rel(node -> op_type) ? VAL_BOOL :node_val_type); 

    //cast left and right
    LLVMValueRef cleft = cast_if_needed(left,  left_node_type, node_val_type); 
    LLVMValueRef cright = cast_if_needed(right,  right_node_type, node_val_type); 
    
    //do the operation 
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
                St_entry* entry = st_find(sym_tab, node -> id_str); 
                if (entry == NULL)
                {
                    fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
                    exit(3); 
                }

                node -> id_type = entry -> type; 

                return LLVMBuildLoad2(builder, type_to_llvm_type(entry -> type), 
                        entry -> id_alloca, "loaded_var"); 
            }
        case NODE_OP: 
            return code_gen_op(root); 
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
    St_entry* entry = st_find(sym_tab, node -> id_str); 
    if (entry == NULL)
    {
        fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
        exit(3); 
    }
    node -> id_type = entry -> type; 
    return entry -> id_alloca; 
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
    LL_insert_back(merge_buffer, then_block); 
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
            LL_insert_back(merge_buffer, then_block); 
            LLVMPositionBuilderAtEnd(builder, elif_block); 

            prev_cond = code_gen_exp(branch_node -> cond); 
            if (ast_exp_val_type(branch_node -> cond) != VAL_BOOL)
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
    LL_insert_back(merge_buffer, else_block); 

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
    LLVMBuildBr(builder, for_inc); 

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
    LLVMBuildBr(builder, while_cond); 
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
    LLVMBuildBr(builder, dowhile_cond); 

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

static void code_gen_stmt(AST_node* root)
{
    if (root == NULL)
        return; 

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

                Value_type dest_type = ast_exp_val_type(node -> dest); 
                Value_type exp_type = ast_exp_val_type(node -> assign_exp); 

                Value_type assign_type = type_resolve_assign(dest_type, exp_type); 

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
        default: 
            
        fprintf(stderr, "Error : bad ast node not a statement\n"); 
        exit(3); 
    }
}

void code_gen_ir(AST_node* program_node)
{
    
    //creating a main function without arguments
    LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_function = LLVMAddFunction(module, "main", ret_type);

    //creating entry basic block 
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_function, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    
    //allocate variables in the stack
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
