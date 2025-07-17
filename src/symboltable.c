#include "symboltable.h"

static St_entry* st_create_var_entry(const char* name, Type* type, LLVMValueRef value_ref); 
static St_entry* st_create_fun_entry(const char* name, Type* fun_type, LLVMValueRef fun_ref, LLVMTypeRef llvm_fun_type); 
static void st_free_entry(void* entry); 

static unsigned int st_hash(const char* key)
{
    //djb2 hash
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; 

    return hash % TABLE_SIZE;
}

Symbol_table* st_create()
{
    Symbol_table* table = malloc(sizeof(Symbol_table)); 

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        table -> buckets[i] = LL_create_list();
    }

    return table; 
}

void st_free(Symbol_table* table)
{
    if (table == NULL)
        return; 
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        LL_free_list(&table -> buckets[i], st_free_entry); 
    }
    free(table); 
}

static St_entry* st_create_var_entry(const char* name, Type* type, LLVMValueRef id_alloca)
{
    St_entry* entry = malloc(sizeof(St_entry));
    
    entry -> kind = ENTRY_VAR; 
    entry -> name = strdup(name); 
    entry -> type = type; 
    entry -> value_ref = id_alloca; 

    return (St_entry*)entry; 
}

static St_entry* st_create_fun_entry(const char* name, Type* fun_type, LLVMValueRef fun_ref, LLVMTypeRef llvm_fun_type)
{
    St_entry* entry = malloc(sizeof(St_entry));
    
    entry -> kind = ENTRY_FUN; 
    entry -> name = strdup(name); 
    entry -> type = fun_type; 
    entry -> value_ref = fun_ref; 
    entry -> type_ref = llvm_fun_type; 

    return (St_entry*)entry; 
}

static void st_free_entry(void* entry)
{
    if (!entry)
        return; 

    if (((St_entry*)entry) -> name != NULL)
        free(((St_entry*)entry) -> name); 
    type_free(((St_entry*)entry)->type); 
    
    free(entry); 
}

int st_insert_var(Symbol_table* table, const char* name, Type* type, LLVMValueRef id_alloca)
{
    if (st_find_var(table, name) != NULL)
        return ST_ALREADY_DECLARED; 
    
    St_entry* entry = st_create_var_entry(name, type, id_alloca); 
    unsigned index = st_hash(name); 
    LL_insert_back(table->buckets[index], entry); 
    
    return ST_INSERT_SUCCESS; 
}

int st_insert_fun(Symbol_table* table, const char* name, Type* fun_type, LLVMValueRef fun_ref, LLVMTypeRef llvm_fun_type)
{
    Function_type* type = (Function_type*)fun_type; 
    if (st_find_fun(table, name, type->param_types, type->param_count) != NULL)
        return ST_ALREADY_DECLARED; 

    St_entry* entry = st_create_fun_entry(name, fun_type, fun_ref, llvm_fun_type); 
    unsigned index = st_hash(name); 
    LL_insert_back(table->buckets[index], entry); 
    
    return ST_INSERT_SUCCESS; 
}

St_entry* st_find_var(Symbol_table* table, const char* name)
{
    unsigned index = st_hash(name); 

    LL_FOR_EACH(table->buckets[index], node)
    {
        if (!strcmp(((St_entry*)node -> data) -> name, name) 
                && ((St_entry*)node->data)->kind==ENTRY_VAR)
        {
            return (St_entry*) node -> data; 
        }
    }

    return NULL; 
}

static bool is_entry_fn_equal(St_entry* entry, const char* name, Type** args, size_t args_count)
{
    if (strcmp(name, entry->name))
        return false; 
    if (entry->type->kind != TYPE_FUNCTION)
        return false; 
    Function_type* fn_type = (Function_type*)entry->type; 
    if (fn_type->param_count != args_count)
        return false; 
    for (size_t i = 0; i < args_count; i++)
    {
        if (!type_equal(fn_type->param_types[i], args[i]))
            return false; 
    }
    return true; 
}

St_entry* st_find_fun(Symbol_table* table, const char* name, Type** args, size_t args_count)
{
    unsigned index = st_hash(name); 

    LL_FOR_EACH(table->buckets[index], node)
    {
        if (is_entry_fn_equal((St_entry*)node->data, name, args, args_count))
        {
            return (St_entry*) node -> data; 
        }
    }

    return NULL; 
    
}
