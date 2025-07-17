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

bool type_equal(Type* type_a, Type* type_b)
{
    if (!type_a || !type_b)
    {
        fprintf(stderr,"Error: comparing null types\n"); 
        exit(2); 
    }
    if (type_a == type_b)
        return true; 
    if (type_a->kind != type_b->kind)
        return false; 
    switch (type_a->kind)
    {
        case TYPE_PRIMITIVE: 
            return ((Primitive_type*)type_a)->val_type 
                                    == ((Primitive_type*)type_b)->val_type; 
        case TYPE_ARRAY: 
            return false;
        default: 
            return false; 
    }
}

Type* type_primitive_create(Value_type vtype)
{
    if (vtype >= VAL_ERR && vtype <= VAL_CHAR)
        return TYPE_PRIMITIVES[vtype];
    return TYPE_ERR;
}

Type* type_function_create(Type* return_type, Type** param_types, size_t param_count)
{
    Function_type* type = malloc(sizeof(Function_type)); 

    type->kind = TYPE_FUNCTION; 
    type->return_type = return_type; 
    type->param_types = NULL; 
    if (param_count > 0)
    {
        type->param_types = malloc(param_count * sizeof(Type*)); 
        memcpy(type->param_types, param_types, param_count * sizeof(Type*)); 
    }
    type->param_count = param_count; 

    return (Type*) type; 
}

Type* type_named_create(Type* actual_type); 

void type_free(Type* type)
{
    if (!type || type_is_singelton(type))
        return; 

    switch (type->kind)
    {
        case TYPE_PRIMITIVE: 
            //do nothing 
            break; 
        case TYPE_FUNCTION: 
            {
            Function_type* fn_type = (Function_type*)type; 
            type_free(fn_type->return_type); 
            if (fn_type->param_types)
            {
                for (size_t i = 0; i < fn_type->param_count; i++)
                {
                    type_free(fn_type->param_types[i]);
                }
                free(fn_type->param_types); 
            }
            }
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

bool op_unary(Op_type op)
{
    return op == OP_NOT || op == OP_UMIN; 
}

static Type* resolve_bin_op(Type* left_type, Type* right_type, Op_type op)
{
    if (!left_type || !right_type)
        type_error("types for operation resolution are null");

    if (left_type->kind != TYPE_PRIMITIVE || right_type->kind != TYPE_PRIMITIVE)
        type_error("Only primitive types are supported for operations");

    Value_type left = ((Primitive_type*)left_type) -> val_type; 
    Value_type right = ((Primitive_type*)right_type) -> val_type; 
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
                return TYPE_FLOAT; 

            return TYPE_INT;  

        case OP_DIV: 
            if (left == VAL_CHAR || right == VAL_CHAR)
                type_error("division on char"); 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("division on booleen"); 
            return TYPE_FLOAT; 
        
        case OP_MOD: 
        case OP_IDIV: 
            if (left != VAL_INT && right != VAL_INT)
                type_error("integer operation on non integers"); 
            return TYPE_INT;  


        //relational 
        case OP_GREATER: 
        case OP_LESS: 
        case OP_GREATER_EQUAL: 
        case OP_LESS_EQUAL: 
            if (left == VAL_CHAR && right == VAL_CHAR)
                return TYPE_CHAR; 
            if (left == VAL_BOOL || right == VAL_BOOL)
                type_error("relational operation on booleen"); 

            if (left != VAL_INT && left != VAL_FLOAT)
                type_error("non-compatible relational operations"); 
            if (right != VAL_INT && right != VAL_FLOAT)
                type_error("non-compatible relational operations"); 

            if (left == VAL_INT && right == VAL_INT)
                return TYPE_INT; 

            return TYPE_FLOAT; 
    
        case OP_EQUAL: 
        case OP_NOT_EQUAL:  
            if (left == VAL_CHAR && right == VAL_CHAR)
                return TYPE_CHAR; 
            if (left == VAL_BOOL && right == VAL_BOOL)
                return TYPE_BOOL; 

            if (left != VAL_INT && left != VAL_FLOAT)
                type_error("non-compatible equality operations"); 
            if (right != VAL_INT && right != VAL_FLOAT)
                type_error("non-compatible equality operations"); 

            if (left == VAL_INT && right == VAL_INT)
                return TYPE_INT; 

            return TYPE_FLOAT; 

        //booleen
        case OP_AND: 
        case OP_OR: 
            if (left != VAL_BOOL && right != VAL_BOOL)
                type_error("booleen operation on non booleen"); 
            return TYPE_BOOL; 


        default: 
            type_error("Unkown operation"); 
    }
    return NULL; /* not reachable */   
}

static Type* resolve_unary_op(Type* left_type, Op_type op) 
{
    if (!left_type)
        type_error("type for operation resolution is null");

    if (left_type->kind != TYPE_PRIMITIVE)
        type_error("Only primitive types are supported for operations");

    Value_type left = ((Primitive_type*)left_type) -> val_type; 
    switch (op)
    {
        case OP_UMIN: 
            if (left == VAL_CHAR)
                type_error("negation on a char"); 
            if (left == VAL_BOOL) 
                type_error("negation on a booleen"); 
            return left_type;  
        case OP_NOT: 
            if (left != VAL_BOOL)
                type_error("booleen operation on non booleen"); 
            return TYPE_BOOL; 
        default: 
            type_error("Unkown operation"); 
    }
    return NULL; /* not reachable */ 
}

Type* type_resolve_op(Type* left_type, Type* right_type, Op_type op)
{
    if (op_unary(op))
        return resolve_unary_op(left_type, op); 
    else 
        return resolve_bin_op(left_type, right_type, op); 
}

Type* type_resolve_assign(Type* dest, Type* exp)
{
    if (!dest || !exp)
        type_error("paramaters are null"); 

    if (type_equal(dest, exp))
        return dest;

    /* Implicit promotion: int → float */ 
    if (dest->kind == TYPE_PRIMITIVE && exp->kind == TYPE_PRIMITIVE) {
        Primitive_type* d = (Primitive_type*)dest;
        Primitive_type* e = (Primitive_type*)exp;

        if (d->val_type == VAL_FLOAT && e->val_type == VAL_INT)
            return dest;
    }

    type_error("identifier and expression don't have the same type");
    return NULL; 
}


LLVMTypeRef type_to_llvm_type(Type* type)
{
    if (!TYPE_IS_PRIMITIVE(type))
    {
        fprintf(stderr, "not implemented yet\n");
        exit(3);
    }
    switch (((Primitive_type*)type) -> val_type)
    {
        case VAL_INT:
            return LLVMInt32Type();
        case VAL_FLOAT:
            return LLVMFloatType();
        case VAL_BOOL:
            return LLVMInt1Type();
        case VAL_CHAR:
            return LLVMInt8Type();
        default:
            fprintf(stderr, "Error: bad type\n");
            exit(3);
    }
    return NULL;
}
