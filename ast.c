#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *ast_new_node(NodeKind kind, int line) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->kind = kind;
    node->line = line;
    return node;
}

NodeList *ast_list_append(NodeList *list, ASTNode *node) {
    NodeList *item = malloc(sizeof(NodeList));
    item->node = node;
    item->next = NULL;

    if (list == NULL) return item;

    NodeList *cur = list;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = item;
    return list;
}

ASTNode *ast_new_int(int line, int value) {
    ASTNode *node = ast_new_node(NODE_INT, line);
    node->data.int_lit.value = value;
    return node;
}

ASTNode *ast_new_bool(int line, int value) {
    ASTNode *node = ast_new_node(NODE_BOOL, line);
    node->data.bool_lit.value = value;
    return node;
}

ASTNode *ast_new_string(int line, char *value) {
    ASTNode *node = ast_new_node(NODE_STRING, line);
    node->data.str_lit.value = strdup(value);
    return node;
}

ASTNode *ast_new_id(int line, char *name) {
    ASTNode *node = ast_new_node(NODE_ID, line);
    node->data.id.name = strdup(name);
    return node;
}

ASTNode *ast_new_binary(NodeKind kind, int line, ASTNode *left, ASTNode *right) {
    ASTNode *node = ast_new_node(kind, line);
    node->data.binary.left_expr = left;
    node->data.binary.right_expr = right;
    return node;
}

ASTNode *ast_new_unary(NodeKind kind, int line, ASTNode *expr) {
    ASTNode *node = ast_new_node(kind, line);
    node->data.unary.expr = expr;
    return node;
}

ASTNode *ast_new_assign(int line, char *name, ASTNode *expr) {
    ASTNode *node = ast_new_node(NODE_ASSIGN, line);
    node->data.assign.name = strdup(name);
    node->data.assign.expr = expr;
    return node;
}

ASTNode *ast_new_if(int line, ASTNode *cond, ASTNode *then, ASTNode *else_) {
    ASTNode *node = ast_new_node(NODE_IF, line);
    node->data.if_.expr_cond = cond;
    node->data.if_.expr_then = then;
    node->data.if_.expr_else_ = else_;
    return node;
}

ASTNode *ast_new_while(int line, ASTNode *cond, ASTNode *body) {
    ASTNode *node = ast_new_node(NODE_WHILE, line);
    node->data.while_.expr_cond = cond;
    node->data.while_.expr = body;
    return node;
}

ASTNode *ast_new_block(int line, NodeList *exprs) {
    ASTNode *node = ast_new_node(NODE_BLOCK, line);
    node->data.block.exprs = exprs;
    return node;
}

ASTNode *ast_new_let_binding(int line, char *name, char *type, ASTNode *init) {
    ASTNode *node = ast_new_node(NODE_LET_BINDING, line);
    node->data.let_binding.name = strdup(name);
    node->data.let_binding.type = strdup(type);
    node->data.let_binding.expr = init;
    return node;
}

ASTNode *ast_new_let(int line, NodeList *bindings, ASTNode *body) {
    ASTNode *node = ast_new_node(NODE_LET, line);
    node->data.let.binding = bindings;
    node->data.let.expr = body;
    return node;
}

ASTNode *ast_new_case(int line, ASTNode *expr, NodeList *branches) {
    ASTNode *node = ast_new_node(NODE_CASE, line);
    node->data.case_.expr = expr;
    node->data.case_.branches = branches;
    return node;
}

ASTNode *ast_new_case_branch(int line, char *name, char *type, ASTNode *body) {
    ASTNode *node = ast_new_node(NODE_CASE_BRANCH, line);
    node->data.case_branch.name = strdup(name);
    node->data.case_branch.type = strdup(type);
    node->data.case_branch.expr = body;
    return node;
}

ASTNode *ast_new_new(int line, char *type) {
    ASTNode *node = ast_new_node(NODE_NEW, line);
    node->data.new_.type = strdup(type);
    return node;
}

ASTNode *ast_new_isvoid(int line, ASTNode *expr) {
    return ast_new_unary(NODE_ISVOID, line, expr);
}

ASTNode *ast_new_dispatch(int line, ASTNode *object, char *method, NodeList *args) {
    ASTNode *node = ast_new_node(NODE_DISPATCH, line);
    node->data.dispatch.expr_object = object;
    node->data.dispatch.method = strdup(method);
    node->data.dispatch.args = args;
    return node;
}

ASTNode *ast_new_static_dispatch(int line, ASTNode *object, char *type, char *method, NodeList *args) {
    ASTNode *node = ast_new_node(NODE_STATIC_DISPATCH, line);
    node->data.static_dispatch.expr_object = object;
    node->data.static_dispatch.type = strdup(type);
    node->data.static_dispatch.method = strdup(method);
    node->data.static_dispatch.args = args;
    return node;
}

ASTNode *ast_new_self_dispatch(int line, char *method, NodeList *args) {
    ASTNode *node = ast_new_node(NODE_SELF_DISPATCH, line);
    node->data.self_dispatch.method = strdup(method);
    node->data.self_dispatch.args = args;
    return node;
}

ASTNode *ast_new_formal(int line, char *name, char *type) {
    ASTNode *node = ast_new_node(NODE_FORMAL, line);
    node->data.formal.name = strdup(name);
    node->data.formal.type = strdup(type);
    return node;
}

ASTNode *ast_new_attr(int line, char *name, char *type, ASTNode *init) {
    ASTNode *node = ast_new_node(NODE_ATTR, line);
    node->data.attr.name = strdup(name);
    node->data.attr.type = strdup(type);
    node->data.attr.expr = init;
    return node;
}

ASTNode *ast_new_method(int line, char *name, NodeList *formals, char *return_type, ASTNode *body) {
    ASTNode *node = ast_new_node(NODE_METHOD, line);
    node->data.method.name = strdup(name);
    node->data.method.formals = formals;
    node->data.method.return_type = strdup(return_type);
    node->data.method.expr = body;
    return node;
}

ASTNode *ast_new_class(int line, char *name, char *parent, NodeList *features) {
    ASTNode *node = ast_new_node(NODE_CLASS, line);
    node->data.class_.name = strdup(name);
    node->data.class_.parent = parent ? strdup(parent) : NULL;
    node->data.class_.features = features;
    return node;
}

ASTNode *ast_new_program(int line, NodeList *classes) {
    ASTNode *node = ast_new_node(NODE_PROGRAM, line);
    node->data.program.classes = classes;
    return node;
}

void ast_free(ASTNode *node) {
    if (node == NULL) return;
    // TODO: liberar campos internos conforme o kind
    free(node);
}

void ast_list_free(NodeList *list) {
    while (list != NULL) {
        NodeList *next = list->next;
        ast_free(list->node);
        free(list);
        list = next;
    }
}