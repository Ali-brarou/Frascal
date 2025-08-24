#include "ast.h"

#include <stdarg.h> 

#define NODE_CREATE(node, node_var_type, node_type) \
    node_var_type* node = malloc(sizeof(node_var_type));\
    node -> type = node_type\

AST_node *ast_program_create(AST_node* new_types, AST_node* subprograms, AST_node* decls, AST_node* stmts)
{
    NODE_CREATE(node, AST_program_node, NODE_PROGRAM); 

    /* will be null if not provided because they are optional */
    node -> new_types = new_types; 
    node -> subprograms = subprograms; 
    node -> statements = stmts; 
    node -> declarations = decls; 

    return (AST_node*) node; 
}

AST_node *ast_subprograms_create(AST_node* subprogram_node)
{
    NODE_CREATE(node, AST_subprograms_node, NODE_SUBPROGRAMS); 
    
    node -> functions_list = LL_create_list(); 
    ast_subprograms_insert((AST_node*)node, subprogram_node); 

    return (AST_node*) node; 
}

void ast_subprograms_insert(AST_node* subprogram_nodes, AST_node* subprogram_node)
{
    if (subprogram_node->type == NODE_FUNCTION)
        LL_insert_back(((AST_subprograms_node*)subprogram_nodes)->functions_list,subprogram_node); 
}

AST_node *ast_function_create(AST_node* id_node, AST_node* params, AST_node* decls, AST_node* stmts, AST_node* ret_type)
{
    NODE_CREATE(node, AST_function_node, NODE_FUNCTION); 

    node->id_node = id_node; 
    node->ret_type = ret_type;
    node->params   = params; 
    node->declarations = decls;  
    node -> statements = stmts; 

    return (AST_node*) node; 
}

AST_node *ast_params_create(AST_node* param)
{
    NODE_CREATE(node, AST_params_node, NODE_PARAMS); 
    
    node->params_list = LL_create_list(); 
    ast_params_insert((AST_node*)node, param); 
    
    return (AST_node*) node; 
}

void ast_params_insert(AST_node* params, AST_node* param)
{
    LL_insert_back(((AST_params_node*)params)->params_list, param); 
}

AST_node *ast_param_create(AST_node* id_type, AST_node* id_node)
{
    NODE_CREATE(node, AST_param_node, NODE_PARAM); 

    node->id_node = id_node; 
    node->id_type = id_type; 

    return (AST_node*) node; 
}

AST_node *ast_args_create(AST_node* arg)
{
    NODE_CREATE(node, AST_args_node, NODE_PARAMS); 
    
    node->args_list = LL_create_list(); 
    ast_args_insert((AST_node*)node, arg); 
    
    return (AST_node*) node; 
}

void ast_args_insert(AST_node* args, AST_node* arg)
{
    LL_insert_back(((AST_args_node*)args)->args_list, arg); 
}

AST_node *ast_arg_create(AST_node* exp)
{
    NODE_CREATE(node, AST_arg_node, NODE_ARG); 

    node->exp = exp; 

    return (AST_node*) node; 
}

AST_node *ast_type_create_from_type(Type* type)
{
    NODE_CREATE(node, AST_type_node, NODE_TYPE); 

    node->type_kind = TYPE_NODE_PRIMITIVE; 
    node->id        = NULL; 
    node->id_type   = type; 
    
    return (AST_node*)node; 
}

AST_node *ast_type_create_from_name(char* name)
{
    NODE_CREATE(node, AST_type_node, NODE_TYPE); 

    node->type_kind = TYPE_NODE_USER_DEFINED; 
    node->id        = name; 
    node->id_type   = NULL; 
    
    return (AST_node*)node; 
}

AST_node *ast_ntype_decls_node_create(AST_node* decl_node) 
{
    NODE_CREATE(node, AST_ntype_decls_node, NODE_NEW_TYPE_DECLS); 

    node -> new_type_decls_list = LL_create_list(); 
    ast_ntype_decls_node_insert((AST_node*)node, decl_node); 

    return (AST_node*) node; 
}

void ast_ntype_decls_node_insert(AST_node* ntype_decls_node, AST_node* ntype_decl_node)
{
    LL_insert_back(((AST_ntype_decls_node*) ntype_decls_node) -> new_type_decls_list, ntype_decl_node); 
}

AST_node *ast_ntype_array_node_create(AST_node* id_node, size_t arr_size, AST_node* elem_type)
{
    NODE_CREATE(node, AST_array_type_decl_node, NODE_ARRAY_TYPE_DECL); 

    node -> id_node = id_node; 
    node -> element_type = elem_type; 
    node -> size = arr_size; 

    return (AST_node*)node; 
}

AST_node *ast_ntype_matrix_node_create(AST_node* id_node, size_t rows, size_t cols, AST_node* elem_type)
{
    NODE_CREATE(node, AST_matrix_type_decl_node, NODE_MATRIX_TYPE_DECL); 

    node -> id_node = id_node; 
    node -> element_type = elem_type; 
    node -> size[0] = rows; 
    node -> size[1] = cols; 

    return (AST_node*) node; 
}

AST_node *ast_decls_node_create(AST_node* decl)
{
    NODE_CREATE(node, AST_declarations_node, NODE_DECLARATIONS); 

    node -> var_decls_list = LL_create_list(); 
    node -> fun_decls_list = LL_create_list(); 
    ast_decls_node_insert((AST_node*)node, decl); 

    return (AST_node*) node; 
}

void ast_decls_node_insert(AST_node* decls, AST_node* decl)
{
    if (decl -> type == NODE_VAR_DECLARATION)
        LL_insert_back(((AST_declarations_node*) decls) -> var_decls_list, decl); 
    else 
        LL_insert_back(((AST_declarations_node*) decls) -> fun_decls_list, decl); 
}

AST_node *ast_var_decl_node_create(AST_node* id_type, AST_node* id_node)
{
    NODE_CREATE(node, AST_var_declaration_node, NODE_VAR_DECLARATION); 

    node -> id_type = id_type; 
    node -> id_node = id_node; 

    return (AST_node*) node; 
}

AST_node *ast_fun_decl_node_create(AST_node* id_node)
{
    NODE_CREATE(node, AST_fun_declaration_node, NODE_FUN_DECLARATION); 

    node -> id_node = id_node; 

    return (AST_node*) node; 
}

AST_node *ast_statements_node_create(AST_node* statement)
{
    NODE_CREATE(node, AST_statements_node, NODE_STATEMENTS); 
    
    node -> stmts_list = LL_create_list(); 
    LL_insert_back(node -> stmts_list, statement); 
    
    return (AST_node*) node; 
}

void ast_statements_node_insert(AST_node* statements, AST_node* statement)
{
    LL_insert_back(((AST_statements_node*) statements) -> stmts_list, statement); 
}

AST_node *ast_assign_node_create(AST_node* dest, AST_node* assign_exp)
{
    NODE_CREATE(node, AST_assign_node, NODE_ASSIGN); 

    node -> dest = dest;  
    node -> assign_exp = assign_exp; 

    return (AST_node*) node; 
}

AST_node *ast_if_node_create(AST_node* cond, AST_node* action, AST_node* elif, AST_node* else_branch)
{
    NODE_CREATE(node, AST_if_node, NODE_IF); 

    node -> cond = cond; 
    node -> action = action; 
    node -> elif_branches = elif;//NULL if there is no else if branches
    node -> else_action = else_branch; //NULL if there is no final else branch  

    return (AST_node*) node; 
}

AST_node *ast_elif_node_create(AST_node* branch)
{
    NODE_CREATE(node, AST_elif_node, NODE_ELIF); 

    node -> branches_list = LL_create_list(); 
    LL_insert_back(node -> branches_list, branch); 

    return (AST_node*) node; 
}
void ast_elif_node_insert(AST_node* elif_node, AST_node* branch)
{
    LL_insert_back(((AST_elif_node*) elif_node) -> branches_list, branch); 
}

AST_node *ast_branch_node_create(AST_node* cond, AST_node* action)
{
    NODE_CREATE(node, AST_branch_node, NODE_BRANCH); 

    node -> cond = cond; 
    node -> action = action; 

    return (AST_node*) node; 
}

AST_node *ast_for_node_create(AST_node* iter, AST_node* from, AST_node* to, AST_node* stmts)
{
    NODE_CREATE(node, AST_for_node, NODE_FOR); 

    node -> iter = iter;
    node -> from = from; 
    node -> to   = to; 
    node -> statements = stmts; 

    return (AST_node*) node; 
}

AST_node *ast_while_node_create(AST_node* cond, AST_node* stmts)
{
    NODE_CREATE(node, AST_while_node, NODE_WHILE); 

    node -> cond = cond; 
    node -> statements = stmts; 

    return (AST_node*) node; 
}

AST_node *ast_dowhile_node_create(AST_node* cond, AST_node* stmts)
{
    NODE_CREATE(node, AST_dowhile_node, NODE_DOWHILE); 

    node -> cond = cond; 
    node -> statements = stmts; 

    return (AST_node*) node; 
}

AST_node *ast_return_node_create(AST_node* exp)
{
    NODE_CREATE(node, AST_return_node, NODE_RETURN); 

    node->exp = exp; 

    return (AST_node*) node; 
}

AST_node *ast_print_node_create(AST_node* args)
{
    NODE_CREATE(node, AST_print_node, NODE_PRINT); 

    node->args = args; 
    return (AST_node*) node; 
}

Type* ast_exp_type(AST_node* exp_node)
{
    if (exp_node == NULL)
        return VAL_ERR; 
    
    switch (exp_node -> type)
    {
        case NODE_OP: 
            {
                AST_op_node* node = (AST_op_node*) exp_node; 
                return node->res_type;  
            }
        case NODE_CONST: 
            {
                AST_const_node* node = (AST_const_node*) exp_node; 
                return type_primitive_create(node->val_type); 
            }
            break; 
        case NODE_ID: 
            {
                AST_id_node* node = (AST_id_node*) exp_node; 
                return node->id_type;  
            }
            break; 
        case NODE_CALL: 
            {
                AST_call_node* node = (AST_call_node*) exp_node; 
                return node->ret_type; 
            }
        case NODE_ARR_SUB: 
            {
                AST_arr_sub_node* node = (AST_arr_sub_node*) exp_node; 
                return node->elem_type; 
            }
        case NODE_MAT_SUB: 
            {
                AST_mat_sub_node* node = (AST_mat_sub_node*) exp_node; 
                return node->elem_type; 
            }
        default: 
            fprintf(stderr,"Error : ast node is not an expression\n"); 
            exit(3);
    }

    return VAL_ERR;  
}

AST_node *ast_op_node_create(Op_type op_type, AST_node* lhs, AST_node* rhs)
{
    NODE_CREATE(node, AST_op_node, NODE_OP); 

    node -> res_type = NULL;  
    node -> op_type = op_type; 
    node -> lhs = lhs; 
    node -> rhs = rhs; 

    return (AST_node*) node; 
}

AST_node *ast_const_node_create(Value_type val_type, Const_value val)
{
    NODE_CREATE(node, AST_const_node, NODE_CONST); 
    
    node -> val_type   = val_type; 
    node -> value   = val; 

    return (AST_node*) node; 
}

AST_node *ast_id_node_create(char* id_str)
{
    NODE_CREATE(node, AST_id_node, NODE_ID); 

    node -> id_type = NULL; 
    node -> id_str = id_str; 

    return (AST_node*) node;  
}

AST_node *ast_call_node_create(AST_node* id_node, AST_node* args)
{
    NODE_CREATE(node, AST_call_node, NODE_CALL); 

    node -> id_node = id_node; 
    node -> args = args; 

    return (AST_node*) node; 
}

AST_node *ast_arr_sub_create(AST_node* id_node, AST_node* exp)
{
    NODE_CREATE(node, AST_arr_sub_node, NODE_ARR_SUB); 

    node -> id_node = id_node; 
    node -> exp = exp; 

    return (AST_node*) node; 
}

AST_node *ast_mat_sub_create(AST_node* id_node, AST_node* exp_row, AST_node* exp_col)
{
    NODE_CREATE(node, AST_mat_sub_node, NODE_MAT_SUB); 

    node -> id_node = id_node; 
    node -> exp[0] = exp_row; 
    node -> exp[1] = exp_col; 

    return (AST_node*) node; 
}

void AST_tree_free(void* tree)
{
    AST_node* root_node = (AST_node*) tree; 
    if (!root_node)
        return; 
    switch (root_node -> type)    
    {
        case NODE_PROGRAM: 
            {
                AST_program_node* node = (AST_program_node*)root_node; 
                AST_tree_free(node -> new_types); 
                AST_tree_free(node -> subprograms); 
                AST_tree_free(node -> statements); 
                AST_tree_free(node -> declarations); 
            }
            break; 
        case NODE_SUBPROGRAMS: 
            {
                AST_subprograms_node* node = (AST_subprograms_node*)root_node; 
                LL_free_list(&node->functions_list, AST_tree_free); 
            }
            break; 
        case NODE_FUNCTION: 
            {
                AST_function_node* node = (AST_function_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->params); 
                AST_tree_free(node->declarations); 
                AST_tree_free(node->statements); 
                AST_tree_free(node->ret_type); 
            }
            break; 
        case NODE_PARAMS: 
            {
                AST_params_node* node = (AST_params_node*)root_node; 
                LL_free_list(&node->params_list, AST_tree_free); 
            }
            break; 
        case NODE_PARAM: 
            {
                AST_param_node* node = (AST_param_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->id_type); 
            }
            break; 
        case NODE_ARGS: 
            {
                AST_args_node* node = (AST_args_node*)root_node; 
                LL_free_list(&node->args_list, AST_tree_free); 
            }
            break; 
        case NODE_ARG: 
            {
                AST_arg_node* node = (AST_arg_node*)root_node; 
                AST_tree_free(node->exp); 
            }
            break; 
        case NODE_TYPE: 
            {
                AST_type_node* node = (AST_type_node*)root_node; 
                free(node->id); 
            }
            break; 
        case NODE_NEW_TYPE_DECLS: 
            {
                AST_ntype_decls_node* node = (AST_ntype_decls_node*)root_node; 
                LL_free_list(&node->new_type_decls_list, AST_tree_free); 
            }
            break; 
        case NODE_ARRAY_TYPE_DECL: 
            {
                AST_array_type_decl_node* node = (AST_array_type_decl_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->element_type); 
            }
            break; 
        case NODE_MATRIX_TYPE_DECL: 
            {
                AST_matrix_type_decl_node* node = (AST_matrix_type_decl_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->element_type); 
            }
            break; 
        case NODE_DECLARATIONS: 
            {
                AST_declarations_node* node = (AST_declarations_node*)root_node; 
                LL_free_list(&node -> var_decls_list, AST_tree_free); 
                LL_free_list(&node -> fun_decls_list, AST_tree_free); 
            }
            break; 
        case NODE_VAR_DECLARATION: 
            {
                AST_var_declaration_node* node = (AST_var_declaration_node*)root_node; 
                AST_tree_free(node -> id_node);
                AST_tree_free(node -> id_type); 
            }
            break; 
        case NODE_FUN_DECLARATION: 
            {
                AST_fun_declaration_node* node = (AST_fun_declaration_node*)root_node; 
                AST_tree_free(node -> id_node);
            }
            break; 
        case NODE_STATEMENTS: 
            {
                AST_statements_node* node = (AST_statements_node*)root_node; 
                LL_free_list(&node -> stmts_list, AST_tree_free); 
            }
            break; 
        case NODE_ASSIGN: 
            {
                AST_assign_node* node = (AST_assign_node*)root_node; 
                AST_tree_free(node -> dest); 
                AST_tree_free(node -> assign_exp); 
            }
            break; 
        case NODE_IF: 
            {
                AST_if_node* node = (AST_if_node*)root_node; 
                AST_tree_free(node -> cond); 
                AST_tree_free(node -> action); 
                AST_tree_free(node -> elif_branches); 
                AST_tree_free(node -> else_action); 
            }
            break; 
        case NODE_ELIF: 
            {
                AST_elif_node* node = (AST_elif_node*)root_node; 
                LL_free_list(&node -> branches_list, AST_tree_free) ; 
            }
            break; 
        case NODE_BRANCH: 
            {
                AST_branch_node* node = (AST_branch_node*)root_node; 
                AST_tree_free(node -> cond); 
                AST_tree_free(node -> action); 
            }
            break;  
        case NODE_FOR: 
            {
                AST_for_node* node = (AST_for_node*)root_node; 
                AST_tree_free(node -> iter); 
                AST_tree_free(node -> from); 
                AST_tree_free(node -> to); 
                AST_tree_free(node -> statements); 
            }
            break; 
        case NODE_WHILE: 
            {
                AST_while_node* node = (AST_while_node*)root_node; 
                AST_tree_free(node -> cond); 
                AST_tree_free(node -> statements); 
            }
            break; 
        case NODE_DOWHILE: 
            {
                AST_dowhile_node* node = (AST_dowhile_node*)root_node; 
                AST_tree_free(node -> cond); 
                AST_tree_free(node -> statements); 
            }
            break; 
        case NODE_RETURN: 
            {
                AST_return_node* node = (AST_return_node*)root_node; 
                AST_tree_free(node -> exp); 
            }
            break; 
        case NODE_PRINT: 
            {
                AST_print_node* node = (AST_print_node*)root_node; 
                AST_tree_free(node->args); 
            }
            break; 
        case NODE_OP: 
            {
                AST_op_node* node = (AST_op_node*)root_node; 
                AST_tree_free(node -> lhs); 
                AST_tree_free(node -> rhs); 
            }
            break; 
        case NODE_CONST: 
            {
                //do nothing; 
            } 
            break; 
        case NODE_ID: 
            {
                AST_id_node* node = (AST_id_node*)root_node; 
                free(node -> id_str); 
            }
            break; 
        case NODE_CALL: 
            {
                AST_call_node* node = (AST_call_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->args); 
            }
            break; 
        case NODE_ARR_SUB: 
            {
                AST_arr_sub_node* node = (AST_arr_sub_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->exp); 
            }
            break; 
        case NODE_MAT_SUB: 
            {
                AST_mat_sub_node* node = (AST_mat_sub_node*)root_node; 
                AST_tree_free(node->id_node); 
                AST_tree_free(node->exp[0]); 
                AST_tree_free(node->exp[1]); 
            }
            break; 
        default: 
            fprintf(stderr, "Error bad node %d\n", root_node -> type); 
            break; 
        
    }

    free(root_node); 
}

static void printd(int depth, char* format, ...)
{
    va_list args; 
    va_start (args, format);
    for (int i = 0; i < depth; i++)
        printf("\t"); 

    vprintf(format, args); 
    va_end(args); 
}

void AST_tree_print(AST_node* root_node, int depth)
{
    if (!root_node)
        return; 

    switch (root_node -> type)
    {
        case NODE_PROGRAM: 
            {
                AST_program_node* node = (AST_program_node*)root_node; 
                printd(depth, "program node : \n"); 
                AST_tree_print(node -> subprograms, depth+1); 
                AST_tree_print(node -> declarations, depth + 1); 
                AST_tree_print(node -> statements, depth + 1); 
            }
            break; 
        case NODE_SUBPROGRAMS: 
            {
                AST_subprograms_node* node= (AST_subprograms_node*)root_node; 
                printd(depth, "multiple subprograms : \n"); 
                for (LL_Node* head = node->functions_list->head;head;head = head->next)
                {
                    AST_tree_print((AST_node*)head->data, depth+1); 
                }
            }
            break; 
        case NODE_FUNCTION: 
            {
                AST_function_node* node = (AST_function_node*)root_node; 
                printd(depth, "function defintion : \n");
                AST_tree_print(node->id_node, depth); 
                AST_tree_print(node->declarations, depth); 
                AST_tree_print(node->statements, depth); 
            }
            break; 
        case NODE_DECLARATIONS: 
            {
                AST_declarations_node* node = (AST_declarations_node*)root_node; 
                printd(depth, "multiple definitions : \n"); 
                LL_Node* head = node -> var_decls_list -> head; 
                while (head != NULL)
                {
                    AST_tree_print((AST_node*) head -> data, depth + 1); 
                    head = head -> next; 
                }
            }
            break; 
        case NODE_VAR_DECLARATION: 
            {
                AST_var_declaration_node* node = (AST_var_declaration_node*)root_node; 
                printd(depth, "definition of identifier with type : \n"); 
                AST_tree_print(node -> id_node, depth + 1); 
            }
            break; 
        case NODE_STATEMENTS: 
            {
                AST_statements_node* node = (AST_statements_node*)root_node; 
                printd(depth, "multiples statements nodes : \n"); 
                LL_Node* head = node -> stmts_list -> head; 
                while (head != NULL)
                {
                    AST_tree_print((AST_node*) head -> data, depth + 1); 
                    head = head -> next; 
                }
            }
            break; 
        case NODE_ASSIGN: 
            {
                AST_assign_node* node = (AST_assign_node*)root_node; 
                printd(depth, "assigning id to expr : \n"); 
                AST_tree_print(node -> dest, depth); 
                AST_tree_print(node -> assign_exp, depth + 1); 
            }
            break; 
        case NODE_IF: 
            {
                AST_if_node* node = (AST_if_node*)root_node; 
                printd(depth, "an if statement with condition : \n"); 
                AST_tree_print(node -> cond, depth + 1); 
                printd(depth, "an with action : \n"); 
                AST_tree_print(node -> action, depth + 1); 
                printd(depth, "optional elif branches : \n"); 
                AST_tree_print(node -> elif_branches, depth); 
                printd(depth, "an optional else action : \n"); 
                AST_tree_print(node -> else_action, depth + 1); 
            }
            break; 
        case NODE_ELIF: 
            {
                AST_elif_node* node = (AST_elif_node*)root_node; 
                printd(depth, "multiples elif branches : \n"); 
                LL_Node* head = node -> branches_list -> head; 
                while (head != NULL)
                {
                    AST_tree_print((AST_node*) head -> data, depth + 1); 
                    head = head -> next; 
                }
            }
            break; 
        case NODE_BRANCH: 
            {
                AST_branch_node* node = (AST_branch_node*)root_node; 
                printd(depth, "this branch condition : \n"); 
                AST_tree_print(node -> cond, depth + 1); 
                printd(depth, "this branch action : \n"); 
                AST_tree_print(node -> action, depth + 1); 
            }
            break; 
        case NODE_FOR: 
            {
                AST_for_node* node = (AST_for_node*)root_node; 
                printd(depth, "for stmt with iter : \n"); 
                AST_tree_print(node -> iter, depth + 1); 
                printd(depth, "for from  : \n"); 
                AST_tree_print(node -> from, depth + 1); 
                printd(depth, "for to  : \n"); 
                AST_tree_print(node -> to, depth + 1); 
                printd(depth, "for stmts: \n"); 
                AST_tree_print(node -> statements, depth + 1); 

            }
            break; 
        case NODE_OP: 
            {
                AST_op_node* node = (AST_op_node*)root_node; 
                if (node -> op_type == OP_NOT)
                {
                    printd(depth, "not operation expression : \n"); 
                    AST_tree_print(node -> lhs, depth + 1); 
                }
                if (node -> op_type == OP_UMIN)
                {
                    printd(depth, "minus operation expression : \n"); 
                    AST_tree_print(node -> lhs, depth + 1); 
                }
                else
                {
                    char* op; 
                    switch (node -> op_type)
                    {
                        case OP_ADD: 
                            op = "add"; 
                            break; 
                        case OP_SUB: 
                            op = "sub"; 
                            break; 
                        case OP_MUL: 
                            op = "mul"; 
                            break; 
                        case OP_DIV: 
                            op = "div"; 
                            break; 
                        case OP_MOD: 
                            op = "mod"; 
                            break; 
                        case OP_IDIV: 
                            op = "idiv"; 
                            break; 
                        case OP_AND: 
                            op = "and"; 
                            break; 
                        case OP_OR: 
                            op = "or"; 
                            break; 
                        default: 
                            op = "unkown"; 
                            break; 
                    }
                    printd(depth, "operation %s between 2 expressions\n", op); 
                    AST_tree_print(node -> lhs, depth + 1); 
                    AST_tree_print(node -> rhs, depth + 1); 
                }
            }
            break; 
        case NODE_CONST: 
            {
                AST_const_node* node = (AST_const_node*)root_node; 
                switch (node -> val_type)
                {
                    case VAL_INT: 
                        printd(depth, "node const with value : %d\n", node -> value.ival); 
                        break; 
                    case VAL_FLOAT: 
                        printd(depth, "node const with value : %f\n", node -> value.fval); 
                        break; 
                    case VAL_BOOL: 
                        printd(depth, "node const with value : %d\n", node -> value.bval); 
                        break; 
                    case VAL_CHAR:
                        printd(depth, "node const with value : %c\n", node -> value.cval); 
                        break; 
                    case VAL_ERR: 
                        printd(depth, "a bad const node\n"); 
                        break; 
                }
            } 
            break; 
        case NODE_ID: 
            {
                AST_id_node* node = (AST_id_node*)root_node; 
                printd(depth, "id node : %s\n", node -> id_str); 
            }
            break; 
        default:      
            fprintf(stderr, "Error bad node %d\n", root_node -> type); 
            return; 
    }
}
