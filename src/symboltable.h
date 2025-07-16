#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdlib.h> 
#include <string.h> 
#include <llvm-c/Core.h> //LLVMValueRef

#include "linkedlist.h"
#include "types.h"

#define TABLE_SIZE 333

typedef Linkedlist Entries_llist;

typedef enum Entry_kind_e {
    ENTRY_FUN, 
    ENTRY_VAR, 
    ENTRY_TYPE, /* user defined types */ 
} Entry_kind; 


typedef struct St_entry_s {
    char* name;   
    Entry_kind kind; 
    Type* type; 
    // llvm : 
    LLVMValueRef value_ref; 
    LLVMTypeRef type_ref;  /* used by function */ 
} St_entry; 

typedef struct Symbol_table_s {
    Entries_llist* buckets[TABLE_SIZE]; 
} Symbol_table; 


Symbol_table* st_create(); 
void st_free(Symbol_table* table); 


#define ST_INSERT_SUCCESS   0
#define ST_ALREADY_DECLARED  1
/* return insert success if success */
/* otherwise already declared if it's already declared duh */
int st_insert_var(Symbol_table* table, char* name, Type* type, LLVMValueRef id_alloca); 
int st_insert_fun(Symbol_table* table, char* name, Type* type, LLVMValueRef function, LLVMTypeRef fun_type); 

St_entry* st_find_var(Symbol_table* table, char* name); 
/* function overloading so you need to pass also args type */ 
St_entry* st_find_fun(Symbol_table* table, char* name, Type** args, size_t args_count); 

#endif
