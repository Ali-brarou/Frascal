#include "builtins.h"

#define MAX_BUILTINS 32

Builtin_fn builtins[MAX_BUILTINS];
size_t builtins_count = 0;

static void register_builtin(Builtin_fn fn); 
static Builtin_fn define_function(LLVMModuleRef module, const char* name, Type* ret_type, Type** params_type, size_t type_count); 
static Builtin_fn create_builtin_ord(LLVMModuleRef module); 
static void implement_ord(LLVMValueRef fn); 

void builtins_init(LLVMModuleRef module)
{    
    Builtin_fn ord_builtin = create_builtin_ord(module); 
    register_builtin(ord_builtin); 
}

void builtins_add_to_symtab(Symbol_table* sym_tab)
{
    for (size_t i = 0; i < builtins_count; i++)
    {
        Builtin_fn builtin = builtins[i]; 
        st_insert_fun(sym_tab, builtin.name, builtin.type, builtin.fun_ref, builtin.fun_type); 
    }
}

static void register_builtin(Builtin_fn fn)
{
    if (builtins_count < MAX_BUILTINS)
        builtins[builtins_count++] = fn; 
}

static Builtin_fn define_function(LLVMModuleRef module, const char* name, Type* ret_type, Type** params_type, size_t params_count)
{
    LLVMTypeRef llvm_ret_type = type_to_llvm_type(ret_type); 
    LLVMTypeRef* llvm_params_type = NULL; 
    if (params_count > 0)
        llvm_params_type = malloc(params_count * sizeof(LLVMTypeRef)); 
    for (size_t i = 0; i < params_count; i++)
    {
        llvm_params_type[i] = type_to_llvm_type(params_type[i]); 
    }

    LLVMTypeRef llvm_fun_type = LLVMFunctionType(llvm_ret_type, llvm_params_type, params_count, 0); 
    LLVMValueRef llvm_fun_ref = LLVMAddFunction(module, name, llvm_fun_type); 

    Type* fun_type = type_function_create(ret_type, params_type, params_count); 
    //clean up 
    free(llvm_params_type); 

    return (Builtin_fn){
        name, 
        fun_type, 
        llvm_fun_ref, 
        llvm_fun_type, 
    }; 
}

/* each function will have it's own builder to avoid interference with main code */ 
static void implement_ord(LLVMValueRef fn) {
    LLVMBuilderRef builder = LLVMCreateBuilder();

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fn, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMValueRef arg = LLVMGetParam(fn, 0);
    LLVMValueRef result = LLVMBuildZExt(builder, arg, LLVMInt32Type(), "ord_result");
    LLVMBuildRet(builder, result);

    LLVMDisposeBuilder(builder);
}

/* ord function (returns ascii number of char as an integer)
 * args (char)
 * return int
 */
static Builtin_fn create_builtin_ord(LLVMModuleRef module)
{
    Type* ret_type = TYPE_INT; 
    Type* params_type[] = {TYPE_CHAR}; 
    size_t params_count = 1; 
     
    Builtin_fn builtin = define_function(module, "ord", ret_type, params_type, params_count); 
    implement_ord(builtin.fun_ref); 

    return builtin; 
}
