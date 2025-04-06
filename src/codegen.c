#include "codegen.h"

//no need for a context 
static LLVMModuleRef module; 
static LLVMBuilderRef builder; 

static Symbol_table* sym_tab; 

// note: both code_gen_id and code_gen_exp can take id_node as input but 
// first one returns a pointer for assignment and the other retunrs a value for expressions
static LLVMTypeRef val_type_to_llvm_type(Value_type val_type); 
static void populate_sym_table(AST_node* decls); 
static void code_gen_stmt(AST_node* root); 
static void code_gen_if_stmt(AST_node* root); 
static LLVMValueRef code_gen_id(AST_node* root); 
static LLVMValueRef code_gen_op(AST_node* root); 
static LLVMValueRef code_gen_exp(AST_node* root); 

void code_gen_init()
{
    //init llvm 
    module = LLVMModuleCreateWithName("test_module"); 
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

static LLVMTypeRef val_type_to_llvm_type(Value_type val_type)
{
    switch (val_type)
    {
        case VAL_INT:
            return LLVMInt32Type(); 
        case VAL_FLOAT: 
            return LLVMFloatType(); 
            break; 
        case VAL_BOOL: 
            return LLVMInt1Type(); 
            break; 
        default: 
            fprintf(stderr, "Error: unkown type\n"); 
            exit(3); 
    }
    return NULL; 
}

static void populate_sym_table(AST_node* decls)
{
    LL_FOR_EACH(((AST_declarations_node*)decls) -> decls_list, ll_node)
    {
        AST_declaration_node* decl_node = (AST_declaration_node*)ll_node -> data; 
        AST_id_node* id_node = (AST_id_node*)(decl_node -> id_node); 

        LLVMValueRef id_alloca;
        switch (decl_node -> id_type)
        {
            case VAL_INT: 
                id_alloca = LLVMBuildAlloca(builder, LLVMInt32Type(), id_node -> id_str); 
            break; 
            case VAL_FLOAT: 
                id_alloca = LLVMBuildAlloca(builder, LLVMFloatType(), id_node -> id_str); 
            break; 
            case VAL_BOOL:
                id_alloca = LLVMBuildAlloca(builder, LLVMInt1Type(), id_node -> id_str); 
            break; 
        }
        if (st_insert(sym_tab, id_node -> id_str, decl_node -> id_type, id_alloca))
        {
            fprintf(stderr, "Error : variable %s already declared\n", id_node -> id_str); 
            exit(3); 
        }
    }
}


static LLVMValueRef code_gen_op(AST_node* root)
{
    if (root == NULL)
        return NULL; 
    AST_op_node* node = (AST_op_node*) root;
    LLVMValueRef left = code_gen_exp(node -> lhs); 
    LLVMValueRef right = code_gen_exp(node -> rhs); 
    switch (node -> op_type)
    {
        case OP_ADD: 
            return LLVMBuildAdd(builder, left, right, "addtmp"); 
        case OP_SUB: 
            return LLVMBuildSub(builder, left, right, "subtmp"); 
        case OP_MUL: 
            return LLVMBuildMul(builder, left, right, "multmp"); 
        case OP_IDIV: 
            return LLVMBuildSDiv(builder, left, right, "idivtmp"); 
        case OP_UMIN: 
            return LLVMBuildNeg(builder, left, "negtmp"); 
        default: 
            fprintf(stderr, "Error: bad node not an operation for now only intiger operations\n"); 
            exit(3); 
            
    }
    return NULL; 
}


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
                return LLVMBuildLoad2(builder, val_type_to_llvm_type(entry -> type), 
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
    AST_id_node* node = (AST_id_node*)root; 
    St_entry* entry = st_find(sym_tab, node -> id_str); 
    if (entry == NULL)
    {
        fprintf(stderr, "Error: %s is not declared\n", node -> id_str);
        exit(3); 
    }
    return entry -> id_alloca; 
}

static void code_gen_if_stmt(AST_node* root)
{
    if (root == NULL)
        return;  
    AST_if_node* node = (AST_if_node*) root; 

    LLVMValueRef condition = code_gen_exp(node -> cond); 

    //get the function from the builder position 
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then"); 
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "else"); 
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge"); 

    LLVMBuildCondBr(builder, condition, then_block, else_block); 

    LLVMPositionBuilderAtEnd(builder, then_block); 
    code_gen_stmt(node -> action); 
    LLVMBuildBr(builder, merge_block); 

    LLVMPositionBuilderAtEnd(builder, else_block); 
    code_gen_stmt(node -> else_action); 
    LLVMBuildBr(builder, merge_block); 

    LLVMPositionBuilderAtEnd(builder, merge_block); 
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
                LLVMBuildStore(builder, val_ref, dest_ref); 
            }
            break; 
        case NODE_IF: 
            code_gen_if_stmt(root);
            break; 
        default: 
        fprintf(stderr, "Error : bad ast node not a statement\n"); 
        exit(3); 
    }
}

void gen_ir(AST_node* program_node)
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
