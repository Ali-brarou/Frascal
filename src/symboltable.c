#include "symboltable.h"

static unsigned int st_hash(char* key)
{
    //djb2 hash
    unsigned long hash = 5381;
    int c;

    while (c = *key++)
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

St_entry* st_create_entry(char* name, Value_type type, LLVMValueRef id_alloca)
{
    St_entry* entry = malloc(sizeof(St_entry));
    
    entry -> name = strdup(name); 
    entry -> type = type; 
    entry -> id_alloca = id_alloca; 

    return entry; 
}

void st_free_entry(void* entry)
{
    if (((St_entry*)entry) -> name != NULL)
        free(((St_entry*)entry) -> name); 
    if (entry != NULL)
        free(entry); 
}

int st_insert(Symbol_table* table, char* name, Value_type type, LLVMValueRef id_alloca)
{
    if (st_find(table, name) != NULL)
        return 1; 
    
    St_entry* entry = st_create_entry(name, type, id_alloca); 
    unsigned index = st_hash(name); 
    LL_insert_back(table->buckets[index], entry); 
    
    return 0; 
}

St_entry* st_find(Symbol_table* table, char* name)
{
    unsigned index = st_hash(name); 

    LL_FOR_EACH(table->buckets[index], node)
    {
        if (!strcmp(((St_entry*)node -> data) -> name, name))
        {
            return (St_entry*) node -> data; 
        }
    }

    return NULL; 
}
