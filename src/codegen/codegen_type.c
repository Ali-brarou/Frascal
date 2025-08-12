#include "codegen.h"

LLVMValueRef code_gen_promote(Codegen_ctx *ctx, LLVMValueRef value, Type* val_type, Type* dest_type)
{
    if (value == NULL || val_type == NULL || dest_type == NULL)
        return NULL;

    if (type_equal(val_type, dest_type))
        return value;

    if (val_type->kind != TYPE_PRIMITIVE || dest_type->kind != TYPE_PRIMITIVE)
    {
        fprintf(stderr, "Cannot cast non-primitive types\n");
        exit(3);
    }

    Primitive_type* from = (Primitive_type*)val_type;
    Primitive_type* to = (Primitive_type*)dest_type;

    if (from->val_type == VAL_INT && to->val_type == VAL_FLOAT)
    {
        return LLVMBuildSIToFP(ctx->builder, value, LLVMFloatType(), "casted_float");
    }

    fprintf(stderr, "Unsupported cast from type %d to type %d\n", from->val_type, to->val_type);
    exit(3);
    return NULL;
}

Type* code_gen_resolve_type(Codegen_ctx* ctx, AST_node* type)
{
    AST_type_node* node = (AST_type_node*)type; 
    if (node->type_kind == TYPE_NODE_PRIMITIVE)
        return node->id_type; 

    /* search for the type */ 
    St_entry* type_entry = st_find_type(ctx->global_sym_tab, node->id); 
    if (!type_entry)
    {
        fprintf(stderr, "Type %s is not defined\n", node->id); 
        exit(3); 
    }
    return type_entry->type; 
}
