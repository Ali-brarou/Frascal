#include "linkedlist.h"

LL_Node* LL_create_node(void* data)
{
    LL_Node* node = malloc(sizeof(LL_Node)); 

    node -> data = data; 
    node -> next = NULL; 
    node -> prev = NULL; 

    return node; 
}

void LL_free_node(LL_Node* node, void (*free_data)(void*))
{
    if (node != NULL)   
    {
        if (node -> data != NULL && free_data != NULL)
            free_data(node->data);
        free(node); 
    }
}

Linkedlist* LL_create_list() 
{
    Linkedlist* ll = malloc(sizeof(Linkedlist)); 

    ll -> head = NULL; 
    ll -> back = NULL; 
    ll -> size = 0; 

    return ll; 
}

void LL_clear(Linkedlist* ll, void (*free_data)(void*))
{
    if (ll == NULL)
        return; 

    LL_Node* node = ll -> head; 
    while (node != NULL)
    {
        LL_Node* tmp = node -> next; 
        LL_free_node(node, free_data); 
        node = tmp; 
    }
    ll -> head = NULL; 
    ll -> back = NULL; 
    ll -> size = 0; 
}

void LL_free_list(Linkedlist** ll, void (*free_data)(void*))
{
    if (*ll == NULL)
    {
        return;  
    }

    LL_clear(*ll, free_data); 
    free(*ll); 
    *ll = NULL; 
}

void LL_insert_front(Linkedlist* ll, void* data)
{
    if (ll == NULL)
        return; 

    LL_Node* node = LL_create_node(data); 
    node -> next = ll -> head; 
    if (ll -> head != NULL)
        ll -> head -> prev = node; 
    ll -> head = node; 
    if (ll -> size == 0)
    {
        ll -> back = node; 
    }
    ll -> size++; 
}

void LL_insert_back(Linkedlist* ll, void* data)
{
    if (ll == NULL)
        return; 

    LL_Node* node = LL_create_node(data); 
    node -> prev = ll -> back; 
    if (ll -> back != NULL)
        ll -> back -> next = node; 
    ll -> back = node; 
    if (ll -> size == 0)
    {
        ll -> head = node; 
    }
    ll -> size++; 
}

void* LL_pop_back(Linkedlist* ll)
{
    if (ll -> back == NULL)
        return NULL; 


    LL_Node* tmp = ll -> back; 
    ll -> back = tmp -> prev; 
    if (ll -> back != NULL)
        ll -> back -> next = NULL; 
     
    ll -> size--; 
    
    if (ll -> size == 0)
    {
        ll -> head = NULL;  
    }

    void* ret = tmp -> data; 

    free(tmp); 

    return ret; 
}

void* LL_pop_front(Linkedlist* ll)
{
    if (ll -> head == NULL)
        return NULL; 

    LL_Node* tmp = ll -> head; 

    ll -> head = tmp -> next; 
    if (ll -> head != NULL)
        ll -> head -> prev = NULL; 
     
    ll -> size--; 
    
    if (ll -> size == 0)
    {
        ll -> back = NULL;  
    }

    void* ret = tmp -> data; 

    free(tmp); 

    return ret; 
}


LL_Node* LL_find(Linkedlist* ll, void* data, int (*data_cmp)(void*, void*))
{
    LL_FOR_EACH(ll, node)
    {
        if (!data_cmp(node -> data, data)) 
            return node; 
    }
    return NULL; 
}

void* LL_front(Linkedlist* ll)
{
    if (ll == NULL || ll -> head == NULL)
        return NULL; 
    return ll -> head -> data; 
}

void* LL_back(Linkedlist* ll)
{
    if (ll == NULL || ll -> back == NULL)
        return NULL; 
    return ll -> back -> data; 
}

size_t LL_size(Linkedlist* ll)
{
    if (ll == NULL)
        return 0; 
    return ll -> size; 
}

bool LL_empty(Linkedlist* ll)
{
    if (ll == NULL)
        return true; 
    return ll -> size == 0; 
}

void LL_print(Linkedlist* ll, void print_data(void*))
{
    if (ll == NULL)
        return; 

    LL_Node* node = ll -> head; 
    while (node != NULL)
    {
        if (node -> data != NULL)
            print_data(node -> data); 
        node = node -> next; 
    }
}
