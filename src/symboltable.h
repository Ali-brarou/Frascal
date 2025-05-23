#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdlib.h> 
#include <string.h> 
#include <llvm-c/Core.h> //LLVMValueRef

#include "linkedlist.h"
#include "types.h"

#define TABLE_SIZE 333

typedef Linkedlist Entries_llist;

typedef struct St_entry_s {
    char* name;   
    Value_type type; 
    // llvm : 
    LLVMValueRef id_alloca; 

} St_entry; 

typedef struct Symbol_table_s {
    Entries_llist* buckets[TABLE_SIZE]; 
} Symbol_table; 


Symbol_table* st_create(); 
void st_free(Symbol_table* table); 

St_entry* st_create_entry(char* name, Value_type type, LLVMValueRef id_alloca); 
void st_free_entry(void* entry); 

/*return 0 if insertion was successful or 1 if its already inserted*/
int st_insert(Symbol_table* table, char* name, Value_type type, LLVMValueRef id_alloca); 
St_entry* st_find(Symbol_table* table, char* name); 

#endif
