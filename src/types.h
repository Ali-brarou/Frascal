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
    TYPE_ARRAY, 
} Type_kind; 

typedef struct Type_s {
    Type_kind kind; 
} Type; 

typedef struct Array_type_s {
    Type_kind kind; 
    
    size_t length;   
    Type* element_type; 
} Array_ntype; 

typedef struct Primitive_type_s {
    Type_kind kind; 
    
    Value_type val_type; 
} Primitive_type;  

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

Type* type_primitive_create(Value_type val_type); 
void type_free(Type* type);  

bool op_rel(Op_type op); //check wether an operation is a relational operation 
Value_type type_resolve_op(Value_type left, Value_type right, Op_type op); 
Value_type type_resolve_assign(Value_type dest, Value_type exp); 

#endif
