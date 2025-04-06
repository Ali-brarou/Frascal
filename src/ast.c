#include "ast.h"

#include <stdarg.h> 

AST_node *ast_node_create(Node_type type, AST_node* left, AST_node* right)
{
    AST_node* node = malloc(sizeof(AST_node)); 

    node -> type = type;  

    node -> left = left; 
    node -> right= right; 

    return node; 
}

AST_node *ast_program_create(AST_node* decls, AST_node* stmts)
{
    AST_program_node* node = malloc(sizeof(AST_program_node)); 

    node -> type = NODE_PROGRAM; 

    node -> statements = stmts; //NULL if there is no statement 
    node -> declarations = decls; 

    return (AST_node*) node; 
}

AST_node *ast_decls_node_create(AST_node* decl)
{
    AST_declarations_node* node = malloc(sizeof(AST_declarations_node)); 

    node -> type = NODE_DECLARATIONS; 

    node -> decls_list = LL_create_list(); 
    LL_insert_back(node -> decls_list, decl); 

    return (AST_node*) node; 
}

void ast_decls_node_insert(AST_node* decls, AST_node* decl)
{
    LL_insert_back(((AST_declarations_node*) decls) -> decls_list, decl); 
}

AST_node *ast_decl_node_create(Value_type id_type, AST_node* id_node)
{
    AST_declaration_node* node = malloc(sizeof(AST_declaration_node)); 

    node -> type = NODE_DECLARATION; 

    node -> id_type = id_type;  
    node -> id_node = id_node; 

    return (AST_node*) node; 
}

AST_node *ast_statements_node_create(AST_node* statement)
{
    AST_statements_node* node = malloc(sizeof(AST_statements_node)); 

    node -> type = NODE_STATEMENTS; 
    
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
    AST_assign_node* node = malloc(sizeof(AST_assign_node)); 

    node -> type = NODE_ASSIGN; 

    node -> dest = dest;  
    node -> assign_exp = assign_exp; 

    return (AST_node*) node; 
}

AST_node *ast_if_node_create(AST_node* cond, AST_node* action, AST_node* elif, AST_node* else_branch)
{
    AST_if_node* node = malloc(sizeof(AST_if_node)); 
    
    node -> type = NODE_IF; 

    node -> cond = cond; 
    node -> action = action; 
    node -> elif_branches = elif;//NULL if there is no else if branches
    node -> else_action = else_branch; //NULL if there is no final else branch  

    return (AST_node*) node; 
}

AST_node *ast_elif_node_create(AST_node* branch)
{
    AST_elif_node* node = malloc(sizeof(AST_elif_node)); 

    node -> type = NODE_ELIF; 

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
    AST_branch_node* node = malloc(sizeof(AST_branch_node)); 

    node -> type = NODE_BRANCH; 

    node -> cond = cond; 
    node -> action = action; 

    return (AST_node*) node; 
}




AST_node *ast_op_node_create(Op_type op_type, AST_node* lhs, AST_node* rhs)
{
    AST_op_node* node = malloc(sizeof(AST_op_node)); 

    node -> type = NODE_OP; 

    node -> op_type = op_type; 
    node -> lhs = lhs; 
    node -> rhs = rhs; 

    return (AST_node*) node; 
}

AST_node *ast_const_node_create(Value_type val_type, Value val)
{
    AST_const_node* node = malloc(sizeof(AST_const_node)); 

    node -> type    = NODE_CONST; 
    
    node -> val_type   = val_type; 
    node -> value   = val; 

    return (AST_node*) node; 
}

AST_node *ast_id_node_create(char* id_str)
{
    AST_id_node* node = malloc(sizeof(AST_id_node)); 

    node -> type = NODE_ID; 

    node -> id_str = id_str; 

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
                AST_tree_free(node -> statements); 
                AST_tree_free(node -> declarations); 
            }
            break; 
        case NODE_DECLARATIONS: 
            {
                AST_declarations_node* node = (AST_declarations_node*)root_node; 
                LL_free_list(&node -> decls_list, AST_tree_free); 
            }
            break; 
        case NODE_DECLARATION: 
            {
                AST_declaration_node* node = (AST_declaration_node*)root_node; 
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
                if (node -> id_str)
                    free(node -> id_str); 
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
                AST_tree_print(node -> declarations, depth + 1); 
                AST_tree_print(node -> statements, depth + 1); 
            }
            break; 
        case NODE_DECLARATIONS: 
            {
                AST_declarations_node* node = (AST_declarations_node*)root_node; 
                printd(depth, "multiple definitions : \n"); 
                LL_Node* head = node -> decls_list -> head; 
                while (head != NULL)
                {
                    AST_tree_print((AST_node*) head -> data, depth + 1); 
                    head = head -> next; 
                }
            }
            break; 
        case NODE_DECLARATION: 
            {
                AST_declaration_node* node = (AST_declaration_node*)root_node; 
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
