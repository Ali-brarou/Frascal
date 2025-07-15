#include "types.h"

Type* TYPE_PRIMITIVES[VAL_TYPE_LAST + 1]; /*exposed one :3*/ 
static Primitive_type primitive_types[VAL_CHAR + 1]; 

void type_init(void)
{
    for (int val_type = 0; val_type <= VAL_TYPE_LAST; val_type++)
    {
        primitive_types[val_type].kind = TYPE_PRIMITIVE; 
        primitive_types[val_type].val_type = (Value_type)val_type; 
        TYPE_PRIMITIVES[val_type] = (Type*)&primitive_types[val_type]; 
    }
}

bool type_is_singelton(Type* type)
{
    for (int val_type = 0; val_type <= VAL_TYPE_LAST; val_type++)
    {
        if (type == TYPE_PRIMITIVES[val_type])
            return true; 
    }
    return false; 
}

Type* type_primitive_create(Value_type vtype)
{
    if (vtype >= VAL_ERR && vtype <= VAL_CHAR)
        return TYPE_PRIMITIVES[vtype];
    return TYPE_ERR;
}

void type_free(Type* type)
{
    if (!type || type_is_singelton(type))
        return; 

    switch (type->kind)
    {
        case TYPE_PRIMITIVE: 
            //do nothing 
            break; 
        default: 
            fprintf(stderr, "not implemented yet\n"); 
            exit(1); 
    }

    free(type); 
}

void type_error(char* msg)
{
    fprintf(stderr, "Error : %s\n", msg); 
    exit(3); 
}

bool op_rel(Op_type op)
{
    return op >= OP_GREATER && op <= OP_NOT_EQUAL; 
}

Value_type type_resolve_op(Value_type left, Value_type right, Op_type op)
{
    switch (op)
    {
        //arithmetique operations 
        case OP_ADD: 
        case OP_SUB: 
        case OP_MUL: 
            if (left == VAL_CHAR || right == VAL_CHAR)
                type_error("arithmetique operation on char"); 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("arithmetique operation on booleen"); 

            if (left == VAL_FLOAT || right == VAL_FLOAT)
                return VAL_FLOAT; 

            return VAL_INT; 

        case OP_DIV: 
            if (left == VAL_CHAR || right == VAL_CHAR)
                type_error("division on char"); 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("division on booleen"); 
            return VAL_FLOAT; 
        
        case OP_MOD: 
        case OP_IDIV: 
            if (left != VAL_INT && right != VAL_INT)
                type_error("integer operation on non integers"); 
            return VAL_INT;  

        case OP_UMIN: 
            if (left == VAL_CHAR)
                type_error("negation on a char"); 
            if (left == VAL_BOOL) 
                type_error("negation on a booleen"); 
            return left;  

        //relational 
        case OP_GREATER: 
        case OP_LESS: 
        case OP_GREATER_EQUAL: 
        case OP_LESS_EQUAL: 
            if (left == VAL_CHAR && right == VAL_CHAR)
                return VAL_CHAR; 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("relational operation on booleen"); 

            if (left != VAL_INT && left != VAL_FLOAT)
                type_error("non-compatible relational operations"); 
            if (right != VAL_INT && right != VAL_FLOAT)
                type_error("non-compatible relational operations"); 

            if (left == VAL_INT && right == VAL_INT)
                return VAL_INT; 

            return VAL_FLOAT; 
    
        case OP_EQUAL: 
        case OP_NOT_EQUAL:  
            if (left == VAL_CHAR && right == VAL_CHAR)
                return VAL_CHAR; 
            if (left == VAL_BOOL && right == VAL_BOOL)
                return VAL_BOOL; 

            if (left != VAL_INT && left != VAL_FLOAT)
                type_error("non-compatible equality operations"); 
            if (right != VAL_INT && right != VAL_FLOAT)
                type_error("non-compatible equality operations"); 

            if (left == VAL_INT && right == VAL_INT)
                return VAL_INT; 

            return VAL_FLOAT; 

        //booleen
        case OP_AND: 
        case OP_OR: 
            if (left != VAL_BOOL && right != VAL_BOOL)
                type_error("booleen operation on non booleen"); 
            return VAL_BOOL; 

        case OP_NOT: 
            if (left != VAL_BOOL)
                type_error("booleen operation on non booleen"); 
            return VAL_BOOL; 

        default: 
            type_error("Unkown operation"); 
    }
    return VAL_ERR; 
}

Value_type type_resolve_assign(Value_type dest, Value_type exp)
{
    if (dest != exp)
    {
        if (dest == VAL_FLOAT && exp == VAL_INT)     
            return VAL_FLOAT; 
        
        type_error("identifier and expression don't have the same type"); 
    }
    return dest;  
}
