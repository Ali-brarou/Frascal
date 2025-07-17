#ifndef BUILTINS_H
#define BUILTINS_H

#include <llvm-c/Core.h>

#include "symboltable.h"

typedef struct Builtin_fn_s {
    const char* name; 
    Type* type; /* function type */ 
    
    /* llvm */ 
    LLVMValueRef fun_ref;  
    LLVMTypeRef fun_type;  

} Builtin_fn; 

void builtins_init(LLVMModuleRef module); 
void builtins_add_to_symtab(Symbol_table* sym_tab); 

#endif
