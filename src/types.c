#include "types.h"


void type_error(char* msg)
{
    fprintf(stderr, "Error : %s\n", msg); 
    exit(3); 
}

bool op_rel(Op_type op)
{
    return op == OP_GREATER     || op == OP_LESS        ||
        op == OP_GREATER_EQUAL  || op == OP_LESS_EQUAL  ||
        op == OP_EQUAL          || op == OP_NOT_EQUAL; 
}

Value_type type_resolve_op(Value_type left, Value_type right, Op_type op)
{
    switch (op)
    {
        //arithmetique operations 
        case OP_ADD: 
        case OP_SUB: 
        case OP_MUL: 
            if (left == VAL_BOOL || right == VAL_BOOL)
            {
                type_error("Error : arithmetique operation on booleen"); 
            }

            if (left == VAL_FLOAT || right == VAL_FLOAT)
                return VAL_FLOAT; 

            return VAL_INT; 

        case OP_DIV: 
            if (left == VAL_BOOL || right == VAL_BOOL)
            {
                type_error("Error : division on booleen"); 
            }
            return VAL_FLOAT; 
        
        case OP_MOD: 
        case OP_IDIV: 
            if (left != VAL_INT && right != VAL_INT)
                type_error("Error : integer operation on non integers"); 
            return VAL_INT;  

        case OP_UMIN: 
            if (left == VAL_BOOL) 
                type_error("Error: negation on a booleen"); 
            return left;  

        //relational 
        case OP_GREATER: 
        case OP_LESS: 
        case OP_GREATER_EQUAL: 
        case OP_LESS_EQUAL: 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("Error : relational operation on booleen"); 

            if (left == VAL_FLOAT || right == VAL_FLOAT)
                return VAL_FLOAT; 

            return VAL_INT; 
    
        case OP_EQUAL: 
        case OP_NOT_EQUAL:  
            if (left == VAL_BOOL || right == VAL_BOOL)
                return VAL_BOOL; 

            if (left != VAL_INT && left != VAL_FLOAT)
                type_error("Error : non-compatible equality operations"); 
            if (right != VAL_INT && right != VAL_FLOAT)
                type_error("Error : non-compatible equality operations"); 

            if (left == VAL_INT && right == VAL_INT)
                return VAL_INT; 

            return VAL_FLOAT; 

        //booleen
        case OP_AND: 
        case OP_OR: 
            if (left != VAL_BOOL && right != VAL_BOOL)
                type_error("Error: booleen operation on non booleen"); 
            return VAL_BOOL; 

        case OP_NOT: 
            if (left != VAL_BOOL)
                type_error("Error: booleen operation on non booleen"); 
            return VAL_BOOL; 

        default: 
            type_error("Unkown operation"); 
    }
    return VAL_NULL; 
}

Value_type type_resolve_assign(Value_type dest, Value_type exp)
{
    if (dest != exp)
    {
        if (dest == VAL_FLOAT && exp == VAL_INT)     
            return VAL_FLOAT; 
        
        type_error("Error : identifier and expression don't have the same type"); 
    }
    return dest;  
}
