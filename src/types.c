#include "types.h"


void type_error(char* msg)
{
    fprintf(stderr, "Error : %s\n", msg); 
    exit(3); 
}


Value_type type_resolve_op(Value_type left, Value_type right, Op_type op)
{
    switch (op)
    {
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

        default: 
            type_error("Unkown operation"); 
    }
    return VAL_ERROR; 
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
