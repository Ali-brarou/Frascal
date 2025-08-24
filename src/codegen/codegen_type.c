#include "codegen.h"

static void code_gen_new_array_type(Codegen_ctx *ctx, AST_node* new_array_type); 
static void code_gen_new_matrix_type(Codegen_ctx *ctx, AST_node* new_matrix_type); 

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

void code_gen_new_types(Codegen_ctx *ctx, AST_node* new_types)
{
    if (!new_types)
        return;
    AST_ntype_decls_node* node = (AST_ntype_decls_node*)new_types;
    LL_FOR_EACH(node->new_type_decls_list, ll_node)
    {
        AST_node* decl_node = (AST_node*)ll_node->data;
        switch (decl_node->type)
        {
            case NODE_ARRAY_TYPE_DECL:  
                code_gen_new_array_type(ctx, decl_node);
                break; 
            case NODE_MATRIX_TYPE_DECL: 
                code_gen_new_matrix_type(ctx, decl_node);
                break; 
            default: 
                assert(0); 
        }
    }
}

static void code_gen_new_array_type(Codegen_ctx *ctx, AST_node* new_array_type)
{
    if (!new_array_type)
        return;
    assert(new_array_type->type == NODE_ARRAY_TYPE_DECL);

    AST_array_type_decl_node* node = (AST_array_type_decl_node*)new_array_type;
    Type* element_type = code_gen_resolve_type(ctx, node->element_type);
    Type* arr_type = type_array_create(element_type, node->size);
    if (st_insert_type(ctx->global_sym_tab, ((AST_id_node*)node->id_node)->id_str, arr_type) == ST_ALREADY_DECLARED)
    {
        fprintf(stderr, "Error : type %s declared twice\n",((AST_id_node*)node)->id_str);
        exit(3);
    }
}

static void code_gen_new_matrix_type(Codegen_ctx *ctx, AST_node* new_matrix_type)
{
    if (!new_matrix_type)
        return; 
    assert(new_matrix_type->type == NODE_MATRIX_TYPE_DECL);
    
    AST_matrix_type_decl_node* node = (AST_matrix_type_decl_node*)new_matrix_type; 
    Type* element_type = code_gen_resolve_type(ctx, node->element_type); 
    Type* mat_type = type_matrix_create(element_type, node->size[0], node->size[1]);
    if (st_insert_type(ctx->global_sym_tab, ((AST_id_node*)node->id_node)->id_str, mat_type) == ST_ALREADY_DECLARED)
    {
        fprintf(stderr, "Error : type %s declared twice\n",((AST_id_node*)node)->id_str);
        exit(3);
    }
}
