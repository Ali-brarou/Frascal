#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h>  

typedef enum Value_type_e {
    VAL_NULL, 
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

bool op_rel(Op_type op); //check wether an operation is a relational operation 
Value_type type_resolve_op(Value_type left, Value_type right, Op_type op); 
Value_type type_resolve_assign(Value_type dest, Value_type exp); 

#endif
