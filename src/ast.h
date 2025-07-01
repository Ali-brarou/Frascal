#ifndef AST_H
#define AST_H

#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
#include <stdio.h> 

#include "types.h"
#include "linkedlist.h"

typedef Linkedlist Var_declarations_llist; 
typedef Linkedlist Fun_declarations_llist; 
typedef Linkedlist New_type_decls_llist; 
typedef Linkedlist Statements_llist; 
typedef Linkedlist Branches_llist; 

typedef enum Node_type_e {
    NODE_PROGRAM,  
    //new types 
    NODE_NEW_TYPE_DECLS,
    NODE_ARRAY_TYPE_DECL,  
    //declarations
    NODE_DECLARATIONS, 
    NODE_VAR_DECLARATION, 
    NODE_FUN_DECLARATION, 
    //statements
    NODE_STATEMENTS, 
    NODE_ASSIGN, 
    NODE_IF, 
    NODE_ELIF,
    NODE_BRANCH, 
    //expressions
    NODE_OP, 
    NODE_CONST,  
    NODE_ID, 

} Node_type; 




typedef struct AST_node_s { /*basic node*/
    Node_type type;  
} AST_node; 

typedef struct AST_program_node_s { /*main program node*/
    Node_type type; 

    AST_node* declarations; 
    AST_node* statements; 

} AST_program_node; 

typedef struct AST_ntype_decls_node_s {
    Node_type type; 

    New_type_decls_llist* new_type_decls_list; 
} AST_ntype_decls_node; 

typedef struct AST_array_type_decl_node_s {
    Node_type type; 

    Value_type val_type; 
    size_t size; 
} AST_array_type_decl_node; 

typedef struct AST_declarations_node_s {
    Node_type type; 

    Var_declarations_llist* var_decls_list; 
    Fun_declarations_llist* fun_decls_list; 
        
} AST_declarations_node; 

typedef struct AST_var_declaration_node_s {
    Node_type type; 

    Value_type id_type; 

    AST_node* id_node; 
    
} AST_var_declaration_node; 

typedef struct AST_fun_declaration_node_s {
    Node_type type; 

    AST_node* id_node; 
    /* TODO fuck */ 

} AST_fun_declaration_node; 


typedef struct AST_statements_node_s {
    Node_type type; 

    Statements_llist* stmts_list; 

} AST_statements_node; 

typedef struct AST_assign_node_s {
    Node_type type; 

    AST_node* dest; 

    AST_node* assign_exp; 

} AST_assign_node; 

typedef struct AST_if_node_s {
    Node_type type; 
    
    AST_node* cond;  
    AST_node* action;  

    AST_node* elif_branches; 
    
    AST_node* else_action; /*NULL if else is not used*/ 

} AST_if_node; 

typedef struct AST_elif_node_s {
    Node_type type; 

    Branches_llist* branches_list; 

} AST_elif_node; 

typedef struct AST_branch_node_s {
    Node_type type; 

    AST_node* cond; 

    AST_node* action; 

} AST_branch_node; 

typedef struct AST_op_nodes_s {
    Node_type type; 

    Op_type op_type; 

    Value_type val_type; /*This will be populated during type resolution*/ 

    AST_node* lhs; 
    AST_node* rhs; 

} AST_op_node; 

typedef struct AST_const_s {
    Node_type type;  

    Value_type val_type; 

    Const_value value; 

} AST_const_node; 

typedef struct AST_id_node_s {
    Node_type type;  

    char* id_str;  

    Value_type val_type; /*This will be populated during type resolution*/ 

} AST_id_node; 


AST_node *ast_program_create(AST_node* decls, AST_node* stmts); 

//new types 
AST_node *ast_ntype_decls_node_create(AST_node* ntype_decl_node); 
void ast_ntype_decls_node_insert(AST_node* ntype_decls_node, AST_node* ntype_decl_node); 
AST_node *ast_ntype_array_node_create(AST_node* ntype_decl_node); 

//declarations
AST_node *ast_var_decl_node_create(Value_type id_type, AST_node* id_node); 
AST_node *ast_fun_decl_node_create(AST_node* id_node); 
AST_node *ast_decls_node_create(AST_node* decl); 
void ast_decls_node_insert(AST_node* decls, AST_node* decl); 

//statements
AST_node *ast_statements_node_create(AST_node* statement); 
void ast_statements_node_insert(AST_node* statements, AST_node* statement); 
AST_node *ast_assign_node_create(AST_node* dest, AST_node* assign_val); 
AST_node *ast_if_node_create(AST_node* cond, AST_node* action, AST_node* elif, AST_node* else_branch); 
AST_node *ast_elif_node_create(AST_node* branch); 
void ast_elif_node_insert(AST_node* elif_node, AST_node* branch); 
AST_node *ast_branch_node_create(AST_node* cond, AST_node* action);


//expressions
Value_type ast_exp_val_type(AST_node* exp_node); 
AST_node *ast_op_node_create(Op_type otype, AST_node* lhs, AST_node* rhs); 
AST_node *ast_const_node_create(Value_type val_type, Const_value val);  
AST_node *ast_id_node_create(char* id_str); 

void AST_tree_free(void* tree);

//debug
void AST_tree_print(AST_node* root_node, int depth); 
#endif
