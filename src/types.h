#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h>  
#include <string.h> 
#include <llvm-c/Core.h>

#define VAL_TYPE_NB 5
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
    TYPE_ARRAY, 
    TYPE_MATRIX, 
} Type_kind; 

typedef struct Type_s {
    Type_kind kind; 
} Type; 

typedef struct Matrix_type_s{
    Type_kind kind; 
    
    size_t size[2];   
    Type* element_type; 
} Matrix_type; 

typedef struct Array_type_s {
    Type_kind kind; 
    
    size_t size;   
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

/* an ugly solution for ownership of the primitive types (singeltons) */ 
extern Primitive_type type_primitives[VAL_TYPE_NB]; 
#define TYPE_INT    ((Type*)&type_primitives[VAL_INT])
#define TYPE_FLOAT  ((Type*)&type_primitives[VAL_FLOAT])
#define TYPE_BOOL   ((Type*)&type_primitives[VAL_BOOL])
#define TYPE_CHAR   ((Type*)&type_primitives[VAL_CHAR])
#define TYPE_ERR    ((Type*)&type_primitives[VAL_ERR])

bool type_is_singelton(Type* type); 
bool type_equal(Type* type_a, Type* type_b); 

#define TYPE_IS_PRIMITIVE(t) ((t)->kind == TYPE_PRIMITIVE)
Type* type_primitive_create(Value_type val_type); 
/* warning: the params array will be copied */ 
Type* type_function_create(Type* return_type, Type** param_types, size_t param_count);
#define TYPE_IS_ARRAY(t) ((t)->kind == TYPE_ARRAY)
Type* type_array_create(Type* elem_type, size_t arr_size); 
#define TYPE_IS_MATRIX(t) ((t)->kind == TYPE_MATRIX)
Type* type_matrix_create(Type* elem_type, size_t size_row, size_t size_col); 
void type_free(Type* type);  

bool op_rel(Op_type op); /* check whether an operation is a relational operation */
bool op_unary(Op_type op); /* check whether an operation is a unary operation */ 
#define op_binary(op) (!op_unary(op)) 
Type* type_resolve_op(Type* left, Type* right, Op_type op); 
Type* type_resolve_assign(Type* dest, Type* exp); 

LLVMTypeRef type_to_llvm_type(Type* type);

#endif
