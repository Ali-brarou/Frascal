#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>  
#include <stdbool.h> 
#include <stddef.h> 

#define LL_FOR_EACH(llist, node)\
    for(LL_Node*(node) = (llist)->head;(node) != NULL;(node) = (node)->next)

typedef struct LL_Node_s {
    void* data;  
    struct LL_Node_s* next; 
    struct LL_Node_s* prev; 
} LL_Node; 

typedef struct Linkelist_s {
    LL_Node* head; 
    LL_Node* back; 
    size_t size;  
} Linkedlist; 

LL_Node*    LL_create_node(void* data); 
void        LL_free_node(LL_Node* node, void (*free_data)(void*)); 

Linkedlist* LL_create_list(); 
void        LL_clear(Linkedlist* ll, void (*free_data)(void*)); 
void        LL_free_list(Linkedlist** ll, void (*free_data)(void*)); 

void    LL_insert_front(Linkedlist* ll, void* data); 
void    LL_insert_back(Linkedlist* ll, void* data); 
void*   LL_pop_back(Linkedlist* ll); 
void*   LL_pop_front(Linkedlist* ll); 

LL_Node*   LL_find(Linkedlist* ll, void* data, int (*data_cmp)(void*, void*)); 
void*   LL_front(Linkedlist* ll); 
void*   LL_back(Linkedlist* ll); 
size_t  LL_size(Linkedlist* ll); 
bool    LL_empty(Linkedlist* ll); 

//debugging
void LL_print(Linkedlist* ll, void (*print_data)(void*)); 

#endif
