#include "builtins.h"

#define BUILTINS_NB 3

Builtin_fn builtins[BUILTINS_NB];
size_t builtins_count = 0;

static void implement_ord(LLVMValueRef fn); 
static void implement_chr(LLVMValueRef fn) ; 
static void implement_ent(LLVMValueRef fn) ; 
static void builtin_register(Builtin_fn fn); 
static Builtin_fn builtin_build(LLVMModuleRef module, Builtin_prototype* prototype); 

Builtin_prototype prototypes[BUILTINS_NB] = {
    {
        "ord", 
        TYPE_INT, 
        (Type*[]){TYPE_CHAR}, 
        1,
        implement_ord, 
    }, 
    {
        "chr", 
        TYPE_CHAR, 
        (Type*[]){TYPE_INT}, 
        1,
        implement_chr, 
    }, 
    {
        "ent", 
        TYPE_INT, 
        (Type*[]){TYPE_FLOAT}, 
        1,
        implement_ent, 
    }, 
}; 

void builtins_init(LLVMModuleRef module)
{    
    for (int i = 0; i < BUILTINS_NB; i++)
    {

        Builtin_fn builtin = builtin_build(module, &prototypes[i]); 
        builtin_register(builtin); 
    }
}

void builtins_add_to_symtab(Symbol_table* sym_tab)
{
    for (size_t i = 0; i < builtins_count; i++)
    {
        Builtin_fn builtin = builtins[i]; 
        st_insert_fun(sym_tab, builtin.name, builtin.type, builtin.fun_ref, builtin.fun_type); 
    }
}

static void builtin_register(Builtin_fn fn)
{
    assert(builtins_count < BUILTINS_NB);
    builtins[builtins_count++] = fn; 
}

static Builtin_fn builtin_build(LLVMModuleRef module, Builtin_prototype* prot)
{
    LLVMTypeRef llvm_ret_type = type_to_llvm_type(prot->ret_type); 
    LLVMTypeRef* llvm_params_type = NULL; 
    if (prot->params_count > 0)
        llvm_params_type = malloc(prot->params_count * sizeof(LLVMTypeRef)); 
    for (size_t i = 0; i < prot->params_count; i++)
    {
        llvm_params_type[i] = type_to_llvm_type(prot->params_type[i]); 
    }

    LLVMTypeRef llvm_fun_type = LLVMFunctionType(llvm_ret_type, llvm_params_type, prot->params_count, 0); 
    LLVMValueRef llvm_fun_ref = LLVMAddFunction(module, prot->name, llvm_fun_type); 

    Type* fun_type = type_function_create(prot->ret_type, prot->params_type, prot->params_count); 
    //clean up 
    free(llvm_params_type); 

    /* implement */ 
    prot->implement(llvm_fun_ref); 

    return (Builtin_fn){
        prot->name, 
        fun_type, 
        llvm_fun_ref, 
        llvm_fun_type, 
    }; 
}

static inline LLVMBuilderRef create_fn_builder(LLVMValueRef fn)
{
    LLVMBuilderRef builder = LLVMCreateBuilder();

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fn, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    return builder; 
}

/* each function will have it's own builder to avoid interference with main code */ 

/* ord function (returns ascii integer from char)
 * args (char)
 * return int
 */
static void implement_ord(LLVMValueRef fn) 
{
    LLVMBuilderRef builder = create_fn_builder(fn);

    LLVMValueRef arg = LLVMGetParam(fn, 0);
    LLVMValueRef result = LLVMBuildZExt(builder, arg, LLVMInt32Type(), "ord_result");
    LLVMBuildRet(builder, result);

    LLVMDisposeBuilder(builder);
}

/* chr function (returns character from ascii integer) 
 * args (int)
 * return char
 */
static void implement_chr(LLVMValueRef fn) 
{
    LLVMBuilderRef builder = create_fn_builder(fn);

    LLVMValueRef arg = LLVMGetParam(fn, 0);
    LLVMValueRef result = LLVMBuildTrunc(builder, arg, LLVMInt8Type(), "chr_result"); 
    LLVMBuildRet(builder, result); 

    LLVMDisposeBuilder(builder);
}

/* ent function (float to int)
 * args (float)
 * return int 
 */
static void implement_ent(LLVMValueRef fn) 
{
    LLVMBuilderRef builder = create_fn_builder(fn);

    LLVMValueRef arg = LLVMGetParam(fn, 0);
    LLVMValueRef result = LLVMBuildFPToSI(builder, arg, LLVMInt32Type(), "ent_result"); 
    LLVMBuildRet(builder, result); 

    LLVMDisposeBuilder(builder);
}
