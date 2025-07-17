#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h>  

#define VAL_TYPE_LAST VAL_CHAR
typedef enum Value_type_e {
    VAL_ERR = 0, 
    VAL_INT , 
    VAL_FLOAT, 
    VAL_BOOL, 
    VAL_CHAR,     
} Value_type; 

typedef union Const_value_u {
    int     ival; 
    float   fval; 
    bool    bval;  
    char    cval; 
} Const_value; 

typedef enum Type_kind_e {
    TYPE_PRIMITIVE, 
    TYPE_FUNCTION, 
    TYPE_NAMED,  /* new types */ 
    TYPE_ARRAY, 
} Type_kind; 

typedef struct Type_s {
    Type_kind kind; 
} Type; 

typedef struct Array_type_s {
    Type_kind kind; 
    
    size_t length;   
    Type* element_type; 
} Array_type; 

/* should be responsible for freeing the array of params types */ 
typedef struct Function_type_s {
    Type_kind kind; 

    Type* return_type;
    Type** param_types;
    size_t param_count;
} Function_type;

typedef struct Primitive_type_s {
    Type_kind kind; 
    
    Value_type val_type; 
} Primitive_type;  

/* symbol table has the ownership */
typedef struct Named_type_s {
    Type_kind kind; 
    
    Type* actual_type;  
} Name_type;

typedef enum Op_e {
    //arithmatique
    OP_ADD, 
    OP_SUB, 
    OP_MUL,
    OP_DIV, 
    OP_IDIV, 
    OP_MOD, 
    OP_UMIN, 
    //relational
    OP_GREATER,  
    OP_LESS, 
    OP_GREATER_EQUAL, 
    OP_LESS_EQUAL, 
    OP_EQUAL, 
    OP_NOT_EQUAL, 
    //booleen
    OP_OR, 
    OP_AND, 
    OP_NOT, 

} Op_type; 

/* an ugly solution for ownership of the primitive types */ 
extern Type* TYPE_PRIMITIVES[VAL_TYPE_LAST + 1]; 
#define TYPE_INT   TYPE_PRIMITIVES[VAL_INT]
#define TYPE_FLOAT TYPE_PRIMITIVES[VAL_FLOAT]
#define TYPE_BOOL  TYPE_PRIMITIVES[VAL_BOOL]
#define TYPE_CHAR  TYPE_PRIMITIVES[VAL_CHAR]
#define TYPE_ERR   TYPE_PRIMITIVES[VAL_ERR]

void type_init(void);
bool type_is_singelton(Type* type); 
bool type_equal(Type* type_a, Type* type_b); 

#define TYPE_IS_PRIMITIVE(t) ((t)->kind == TYPE_PRIMITIVE)
Type* type_primitive_create(Value_type val_type); 
Type* type_function_create(Type* return_type, Type** param_types, size_t param_count);
Type* type_named_create(Type* actual_type); 
void type_free(Type* type);  

bool op_rel(Op_type op); /* check whether an operation is a relational operation */
bool op_unary(Op_type op); /* check whether an operation is a unary operation */ 
#define op_binary(op) (!op_unary(op)) 
Type* type_resolve_op(Type* left, Type* right, Op_type op); 
Type* type_resolve_assign(Type* dest, Type* exp); 

#endif
