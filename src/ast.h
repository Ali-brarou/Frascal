#ifndef AST_H
#define AST_H

#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
#include <stdio.h> 

#include "types.h"
#include "linkedlist.h"

typedef enum Node_type_e {
    NODE_PROGRAM,  
    NODE_SUBPROGRAMS, 
    NODE_FUNCTION, 
    NODE_PARAMS, 
    NODE_PARAM, 
    NODE_ARG, 
    //new types 
    NODE_NEW_TYPE_DECLS,
    NODE_ARRAY_TYPE_DECL,  
    //declarations
    NODE_DECLARATIONS, 
    NODE_VAR_DECL, 
    NODE_FUN_DECLARATION, 
    //statements
    NODE_STATEMENTS, 
    NODE_ASSIGN, 
    NODE_IF, 
    NODE_ELIF,
    NODE_BRANCH, 
    NODE_FOR, 
    NODE_WHILE, 
    NODE_DOWHILE, 
    NODE_RETURN, 
    NODE_PRINT, 
    //expressions
    NODE_OP, 
    NODE_CONST,  
    NODE_ID, 
    NODE_CALL, 

} Node_type; 

/*basic node*/
typedef struct AST_node_s {
    Node_type type; 
    struct AST_node_s* next;
} AST_node; 

#define FIELD_DEFINE(type, name)    type name; 
#define FIELD_DECLARE(type, name)   ,type name
#define FIELD_SET(type, name)       node->name = name; 
#define FIELD_SKIP(type, name)      /* do nothing */ 

#define AST_DEFINE_STRUCT(NAME, FIELDS)         \
    typedef struct AST_##NAME##_s {             \
        AST_node base;                          \
        FIELDS(FIELD_DEFINE, FIELD_DEFINE)      \
    } AST_##NAME;                               \

#define AST_DEFINE_CTOR(NAME, TYPE, FIELDS)     \
    static inline AST_node* ast_##TYPE##_create(    \
            int _dummy __attribute__((unused))  \
            FIELDS(FIELD_DECLARE, FIELD_SKIP)   \
            )                                   \
    {                                           \
        AST_##NAME* node = malloc(sizeof(AST_##NAME));  \
        memset(node, 0, sizeof(AST_##NAME));    \
        node->base.type = TYPE;                 \
        FIELDS(FIELD_SET, FIELD_SKIP)           \
        return (AST_node*) node;                \
    }                                           \

#define AST_DEFINE_NODE(NAME, TYPE, FIELDS)     \
    AST_DEFINE_STRUCT(NAME, FIELDS)             \
    AST_DEFINE_CTOR(NAME, TYPE, FIELDS)
    

// nodes declaration */ 
#define PROGRAM_FIELDS(EXPORTED, INTERNAL)  \
    EXPORTED(AST_node*, subprograms)        \
    EXPORTED(AST_node*, declarations)       \
    EXPORTED(AST_node*, statements)         

#define FUN_FIELDS(EXPORTED, INTERNAL)      \
    EXPORTED(AST_node*, id_node)            \
    EXPORTED(AST_node*, params)             \
    EXPORTED(AST_node*, declarations)       \
    EXPORTED(AST_node*, statements)         \
    EXPORTED(Type*, ret_type)

#define ARG_FIELDS(EXPORTED, INTERNAL)      \
    EXPORTED(AST_node*, exp)

#define VAR_DECL_FIEDLS(EXPORTED, INTERNAL) \
    EXPORTED(Type*, id_type)                \
    EXPORTED(AST_node*, id_node)

#define ASSIGN_FIELDS(EXPORTED, INTERNAL)   \
    EXPORTED(AST_node*, dest)               \
    EXPORTED(AST_node*, assign_exp)

#define IF_FIELDS(EXPORTED, INTERNAL)       \
    EXPORTED(AST_node*, cond)               \
    EXPORTED(AST_node*, action)             \
    EXPORTED(AST_node*, elif_branches)      \
    EXPORTED(AST_node*, else_action)        

#define FOR_FIELDS(EXPORTED, INTERNAL)      \
    EXPORTED(AST_node*, iter)               \
    EXPORTED(AST_node*, from)               \
    EXPORTED(AST_node*, to)                 \
    EXPORTED(AST_node*, statements)

#define WHILE_FIELDS(EXPORTED, INTERNAL)    \
    EXPORTED(AST_node*, cond)               \
    EXPORTED(AST_node*, statements)

#define DOWHILE_FIELDS(EXPORTED, INTERNAL)  \
    EXPORTED(AST_node*, cond)               \
    EXPORTED(AST_node*, statements)

#define RETURN_FIELDS(EXPORTED, INTERNAL)   \
    EXPORTED(AST_node*, exp)

#define PRINT_FIELDS(EXPORTED, INTERNAL)    \
    EXPORTED(AST_node*, args)

#define OP_FIELDS(EXPORTED, INTERNAL)       \
    EXPORTED(Op_type, op_type)              \
    EXPORTED(AST_node*, lhs)                \
    EXPORTED(AST_node*, rhs)                \
    INTERNAL(Type*, res_type)

#define CONST_FIELDS(EXPORTED, INTERNAL)    \
    EXPORTED(Value_type, val_type)          \
    EXPORTED(Const_value, value)

#define ID_FIELDS(EXPORTED, INTERNAL)       \
    EXPORTED(char*, id_str)                 \
    INTERNAL(Type*, id_type)

#define CALL_FIELDS(EXPORTED, INTERNAL)     \
    EXPORTED(AST_node*, id_node)            \
    EXPORTED(AST_node*, args)               \
    INTERNAL(Type*, fun_type)               \
    INTERNAL(Type*, ret_type)               \

AST_DEFINE_NODE(program_node, NODE_PROGRAM, PROGRAM_FIELDS)
AST_DEFINE_NODE(function_node, NODE_FUNCTION, FUN_FIELDS)
AST_DEFINE_NODE(arg_node, NODE_ARG, ARG_FIELDS); 

AST_DEFINE_NODE(var_declaration_node, NODE_VAR_DECL, VAR_DECL_FIEDLS)

AST_DEFINE_NODE(assign_node, NODE_ASSIGN, ASSIGN_FIELDS)
AST_DEFINE_NODE(if_node, NODE_IF, IF_FIELDS)
AST_DEFINE_NODE(for_node, NODE_FOR, FOR_FIELDS)
AST_DEFINE_NODE(while_node, NODE_WHILE, WHILE_FIELDS)
AST_DEFINE_NODE(dowhile_node, NODE_DOWHILE, DOWHILE_FIELDS)
AST_DEFINE_NODE(return_node, NODE_RETURN, RETURN_FIELDS)
AST_DEFINE_NODE(print_node, NODE_PRINT, PRINT_FIELDS)

AST_DEFINE_NODE(op_node, NODE_OP, OP_FIELDS)
AST_DEFINE_NODE(const_node, NODE_CONST, CONST_FIELDS)
AST_DEFINE_NODE(id_node, NODE_ID, ID_FIELDS)
AST_DEFINE_NODE(call_node, NODE_CALL, CALL_FIELDS)

#define AST_CREATE(TYPE, ...)  ast_##TYPE##_create(0, __VA_ARGS__)
#define AST_FOR_EACH(head, current) \
    for(current = (AST_node*)(head); (current); current = (current)->next)
#define AST_NODE_NEXT(node, next_node) do {(node)->next = (next_node)} while(0)

typedef Linkedlist Functions_llist; 
typedef Linkedlist Params_llist; 
typedef Linkedlist Var_declarations_llist; 
typedef Linkedlist Fun_declarations_llist; 
typedef Linkedlist New_type_decls_llist; 
typedef Linkedlist Statements_llist; 
typedef Linkedlist Branches_llist; 

typedef struct AST_subprograms_node_s {
    Node_type type;  

    Functions_llist* functions_list; 
} AST_subprograms_node; 

typedef struct AST_params_node_s {
    Node_type type; 
    
    Params_llist* params_list; 
} AST_params_node; 

typedef struct AST_param_node_s {
    Node_type type; 

    AST_node* id_node; 
    Type* id_type; 
} AST_param_node; 

typedef struct AST_ntype_decls_node_s {
    Node_type type; 

    New_type_decls_llist* new_type_decls_list; 
} AST_ntype_decls_node; 

typedef struct AST_array_type_decl_node_s {
    Node_type type; 

    Type* element_type; 
    size_t size; 
} AST_array_type_decl_node; 

typedef struct AST_declarations_node_s {
    Node_type type; 

    Var_declarations_llist* var_decls_list; 
    Fun_declarations_llist* fun_decls_list; 
        
} AST_declarations_node; 

typedef struct AST_fun_declaration_node_s {
    Node_type type; 

    AST_node* id_node; 
    /* TODO fuck */ 

} AST_fun_declaration_node; 


typedef struct AST_statements_node_s {
    Node_type type; 

    Statements_llist* stmts_list; 

} AST_statements_node; 


typedef struct AST_elif_node_s {
    Node_type type; 

    Branches_llist* branches_list; 

} AST_elif_node; 

typedef struct AST_branch_node_s {
    Node_type type; 

    AST_node* cond; 
    AST_node* action; 

} AST_branch_node; 

AST_node *ast_subprograms_create(AST_node* subprogram); 
void ast_subprograms_insert(AST_node* subprograms, AST_node* subprogram); 
AST_node *ast_params_create(AST_node* param); 
void ast_params_insert(AST_node* params, AST_node* param); 
AST_node *ast_param_create(Type* id_type, AST_node* id_node); 

//new types 
AST_node *ast_ntype_decls_node_create(AST_node* ntype_decl_node); 
void ast_ntype_decls_node_insert(AST_node* ntype_decls_node, AST_node* ntype_decl_node); 
AST_node *ast_ntype_array_node_create(AST_node* ntype_decl_node); 

//declarations
AST_node *ast_fun_decl_node_create(AST_node* id_node); 
AST_node *ast_decls_node_create(AST_node* decl); 
void ast_decls_node_insert(AST_node* decls, AST_node* decl); 

//statements
AST_node *ast_statements_node_create(AST_node* statement); 
void ast_statements_node_insert(AST_node* statements, AST_node* statement); 
AST_node *ast_elif_node_create(AST_node* branch); 
void ast_elif_node_insert(AST_node* elif_node, AST_node* branch); 
AST_node *ast_branch_node_create(AST_node* cond, AST_node* action);
//expressions
#define AST_IS_INT_TYPE(node) (type_equal(ast_exp_type(node),TYPE_INT))
#define AST_IS_BOOL_TYPE(node) (type_equal(ast_exp_type(node),TYPE_BOOL))
Type* ast_exp_type(AST_node* exp_node); 

void AST_tree_free(void* tree);

//debug
void AST_tree_print(AST_node* root_node, int depth); 
#endif
