#ifndef AST_H
#define AST_H

typedef enum {
    /* Estruturas */
    NODE_PROGRAM,
    NODE_CLASS,
    NODE_METHOD,
    NODE_ATTR,
    NODE_FORMAL,

    /* Controle */
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_LET,
    NODE_LET_BINDING,
    NODE_CASE,
    NODE_CASE_BRANCH,
    NODE_NEW,
    NODE_ISVOID,

    /* Dispatch */
    NODE_DISPATCH,
    NODE_STATIC_DISPATCH,
    NODE_SELF_DISPATCH,

    /* Operadores binários */
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_LT,
    NODE_LE,
    NODE_EQ,

    /* Operadores unários */
    NODE_NEG,
    NODE_NOT,

    /* Literais e identificador */
    NODE_INT,
    NODE_STRING,
    NODE_BOOL,
    NODE_ID,
} NodeKind;

typedef struct ASTNode ASTNode;

//lista encadeada de nós (para features, formals, ags, branches...)
typedef struct NodeList{
    ASTNode *node;
    struct NodeList *next;
}NodeList;

struct ASTNode{
    NodeKind kind;
    int line;

    union
    {
        //nó de um program
        struct{NodeList *classes;} program;

        //nó de uma class
        struct{char *name; char *parent; NodeList *features;} class_; // underline porque "class é pelavra reservada em C"

    //------------ nós de feature--------------    
        //nó de method
        struct{char *name; NodeList *formals; char *return_type; ASTNode *expr;} method;

        // nó de um atribute 
        struct{char *name; char *type; ASTNode *expr;} attr;
    //-----------------------------------------
    
        // nó de um formal
        struct{char *name; char *type;} formal;

    //------------ nós de expr----------------
        // nó de um assign
        struct{char *name; ASTNode *expr;} assign;

        // nó de um dispatch
        struct{ASTNode *expr_object; char *method; NodeList *args;} dispatch;

        // nó de um dispach estático
        struct{ASTNode *expr_object; char *type; char *method; NodeList *args;} static_dispatch;

        //nó de um self_dispatch
        struct{char *method; NodeList *args;} self_dispatch;

        // nó de um if
        struct{ASTNode *expr_cond; ASTNode *expr_then; ASTNode *expr_else_;} if_;

        // nó de while
        struct{ASTNode *expr_cond; ASTNode *expr;} while_;

        // nó de um bloco
        struct{NodeList *exprs;} block;

        // nó de um bindingsde let
        struct{char *name; char *type; ASTNode *expr;} let_binding;

        // nó de um let
        struct{NodeList *binding; ASTNode *expr;} let;

        // nó de um case
        struct{ASTNode *expr; NodeList *branches;} case_;

        //nó de uma branch de case
        struct{char *name; char *type; ASTNode *expr;} case_branch;

        // nó de um new
        struct{char *type;} new_;

        // nó de isvoid/neg (~)/not
        struct{ASTNode *expr;} unary;

        // nó de operações binárias (+, -, /, *, <, <=, =)
        struct{ASTNode *left_expr; ASTNode *right_expr;} binary;

        // nó de ID
        struct{char *name;} id;

        // nó de int
        struct{int value;} int_lit;

        // nó de string
        struct{char *value;} str_lit;

        //nó de booleano
        struct{int value;} bool_lit; // 1 = true, 0 = false
    }data;
    
};

ASTNode *ast_new_node(NodeKind kind, int line);
NodeList *ast_list_append(NodeList *list, ASTNode *node);

ASTNode *ast_new_int(int line, int value);
ASTNode *ast_new_bool(int line, int value);
ASTNode *ast_new_string(int line, char *value);
ASTNode *ast_new_id(int line, char *name);
ASTNode *ast_new_binary(NodeKind kind, int line, ASTNode *left, ASTNode *right);
ASTNode *ast_new_unary(NodeKind kind, int line, ASTNode *expr);
ASTNode *ast_new_assign(int line, char *name, ASTNode *expr);
ASTNode *ast_new_if(int line, ASTNode *cond, ASTNode *then, ASTNode *else_);
ASTNode *ast_new_while(int line, ASTNode *cond, ASTNode *body);
ASTNode *ast_new_block(int line, NodeList *exprs);
ASTNode *ast_new_let(int line, NodeList *bindings, ASTNode *body);
ASTNode *ast_new_let_binding(int line, char *name, char *type, ASTNode *init);
ASTNode *ast_new_case(int line, ASTNode *expr, NodeList *branches);
ASTNode *ast_new_case_branch(int line, char *name, char *type, ASTNode *body);
ASTNode *ast_new_new(int line, char *type);
ASTNode *ast_new_isvoid(int line, ASTNode *expr);
ASTNode *ast_new_dispatch(int line, ASTNode *object, char *method, NodeList *args);
ASTNode *ast_new_static_dispatch(int line, ASTNode *object, char *type, char *method, NodeList *args);
ASTNode *ast_new_self_dispatch(int line, char *method, NodeList *args);
ASTNode *ast_new_attr(int line, char *name, char *type, ASTNode *init);
ASTNode *ast_new_method(int line, char *name, NodeList *formals, char *return_type, ASTNode *body);
ASTNode *ast_new_formal(int line, char *name, char *type);
ASTNode *ast_new_class(int line, char *name, char *parent, NodeList *features);
ASTNode *ast_new_program(int line, NodeList *classes);


void ast_free(ASTNode *node);
void ast_list_free(NodeList *list);

#endif