#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semant.h"
#include "intern.h"

/* ------------------------------------------------------------------ */
/* Criação e liberação                                                  */
/* ------------------------------------------------------------------ */

SemantState *semant_new(void) {
    SemantState *s = malloc(sizeof(SemantState));
    s->env           = classenv_new();
    s->sym           = symtable_new();
    s->current_class = NULL;
    s->errors        = 0;
    return s;
}

void semant_free(SemantState *s) {
    classenv_free(s->env);
    symtable_free(s->sym);
    free(s);
}

/* ------------------------------------------------------------------ */
/* Reporte de erros                                                     */
/* ------------------------------------------------------------------ */

static void semant_error(SemantState *s, int line, const char *msg) {
    fprintf(stderr, "Erro semântico linha %d: %s\n", line, msg);
    s->errors++;
}

/* ------------------------------------------------------------------ */
/* Forward declarations                                                 */
/* ------------------------------------------------------------------ */

static const char *check_expr(SemantState *s, ASTNode *node);
static void        check_feature(SemantState *s, ASTNode *node);
static void        check_class(SemantState *s, ASTNode *node);

/* ------------------------------------------------------------------ */
/* Primeira passagem — registra todas as classes                       */
/* ------------------------------------------------------------------ */

static void collect_classes(SemantState *s, ASTNode *program) {
    NodeList *cls = program->data.program.classes;
    while (cls != NULL) {
        ASTNode *node = cls->node;
        const char *name   = node->data.class_.name;
        const char *parent = node->data.class_.parent
                             ? node->data.class_.parent
                             : "Object";

        /* classes proibidas de herdar */
        if (strcmp(parent, "Int")    == 0 ||
            strcmp(parent, "Bool")   == 0 ||
            strcmp(parent, "String") == 0) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                     "classe '%s' não pode herdar de '%s'", name, parent);
            semant_error(s, node->line, msg);
        }

        if (!classenv_add_class(s->env, name, parent)) {
            char msg[128];
            snprintf(msg, sizeof(msg), "classe '%s' redefinida", name);
            semant_error(s, node->line, msg);
        }

        cls = cls->next;
    }

    if (classenv_check_cycles(s->env)) {
        semant_error(s, 0, "ciclo detectado na hierarquia de herança");
    }
}

/* ------------------------------------------------------------------ */
/* Segunda passagem — registra métodos e atributos de cada classe      */
/* ------------------------------------------------------------------ */

static void collect_features(SemantState *s, ASTNode *program) {
    NodeList *cls = program->data.program.classes;
    while (cls != NULL) {
        ASTNode *node       = cls->node;
        const char *class_name = node->data.class_.name;

        NodeList *feature = node->data.class_.features;
        while (feature != NULL) {
            ASTNode *f = feature->node;

            if (f->kind == NODE_ATTR) {
                if (!classenv_add_attr(s->env, class_name,
                                       f->data.attr.name,
                                       f->data.attr.type)) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "atributo '%s' redefinido em '%s'",
                             f->data.attr.name, class_name);
                    semant_error(s, f->line, msg);
                }
            } else if (f->kind == NODE_METHOD) {
                /* coleta tipos dos parâmetros */
                int param_count = 0;
                NodeList *frm = f->data.method.formals;
                while (frm != NULL) { param_count++; frm = frm->next; }

                char **param_types = malloc(sizeof(char *) * param_count);
                frm = f->data.method.formals;
                for (int i = 0; i < param_count; i++) {
                    param_types[i] = frm->node->data.formal.type;
                    frm = frm->next;
                }

                if (!classenv_add_method(s->env, class_name,
                                          f->data.method.name,
                                          param_types, param_count,
                                          f->data.method.return_type)) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "método '%s' redefinido em '%s'",
                             f->data.method.name, class_name);
                    semant_error(s, f->line, msg);
                }

                free(param_types);
            }

            feature = feature->next;
        }

        cls = cls->next;
    }
}

/* ------------------------------------------------------------------ */
/* Verificação de expressões — retorna o tipo inferido                 */
/* ------------------------------------------------------------------ */

static const char *check_expr(SemantState *s, ASTNode *node) {
    if (node == NULL) return "Object";

    switch (node->kind) {

        /* literais */
        case NODE_INT:    return intern("Int");
        case NODE_BOOL:   return intern("Bool");
        case NODE_STRING: return intern("String");

        /* identificador */
        case NODE_ID: {
            const char *type = symtable_lookup(s->sym,
                                               node->data.id.name);
            if (type == NULL) {
                /* tenta como atributo da classe atual */
                AttrEntry *attr = classenv_lookup_attr(s->env,
                                      s->current_class,
                                      node->data.id.name);
                if (attr == NULL) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "variável '%s' não declarada",
                             node->data.id.name);
                    semant_error(s, node->line, msg);
                    return intern("Object");
                }
                return attr->type;
            }
            return type;
        }

        /* atribuição */
        case NODE_ASSIGN: {
            const char *var_type = symtable_lookup(s->sym,
                                                   node->data.assign.name);
            if (var_type == NULL) {
                AttrEntry *attr = classenv_lookup_attr(s->env,
                                      s->current_class,
                                      node->data.assign.name);
                if (attr == NULL) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "variável '%s' não declarada",
                             node->data.assign.name);
                    semant_error(s, node->line, msg);
                    return intern("Object");
                }
                var_type = attr->type;
            }
            const char *expr_type = check_expr(s, node->data.assign.expr);
            if (!classenv_is_subtype(s->env, expr_type, var_type)) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "tipo '%s' não é subtipo de '%s' na atribuição",
                         expr_type, var_type);
                semant_error(s, node->line, msg);
            }
            return expr_type;
        }

        /* operadores binários */
        case NODE_PLUS:
        case NODE_MINUS:
        case NODE_MUL:
        case NODE_DIV: {
            const char *lt = check_expr(s, node->data.binary.left_expr);
            const char *rt = check_expr(s, node->data.binary.right_expr);
            if (strcmp(lt, "Int") != 0 || strcmp(rt, "Int") != 0)
                semant_error(s, node->line,
                             "operadores aritméticos exigem Int");
            return intern("Int");
        }

        case NODE_LT:
        case NODE_LE: {
            const char *lt = check_expr(s, node->data.binary.left_expr);
            const char *rt = check_expr(s, node->data.binary.right_expr);
            if (strcmp(lt, "Int") != 0 || strcmp(rt, "Int") != 0)
                semant_error(s, node->line,
                             "operadores de comparação exigem Int");
            return intern("Bool");
        }

        case NODE_EQ: {
            const char *lt = check_expr(s, node->data.binary.left_expr);
            const char *rt = check_expr(s, node->data.binary.right_expr);
            /* Int, Bool e String só podem ser comparados com o mesmo tipo */
            if ((strcmp(lt, "Int")    == 0 || strcmp(lt, "Bool") == 0 ||
                 strcmp(lt, "String") == 0) && strcmp(lt, rt) != 0) {
                semant_error(s, node->line,
                             "comparação de igualdade entre tipos incompatíveis");
            }
            return intern("Bool");
        }

        /* operadores unários */
        case NODE_NEG: {
            const char *t = check_expr(s, node->data.unary.expr);
            if (strcmp(t, "Int") != 0)
                semant_error(s, node->line, "'~' exige Int");
            return intern("Int");
        }

        case NODE_NOT: {
            const char *t = check_expr(s, node->data.unary.expr);
            if (strcmp(t, "Bool") != 0)
                semant_error(s, node->line, "'not' exige Bool");
            return intern("Bool");
        }

        case NODE_ISVOID:
            check_expr(s, node->data.unary.expr);
            return intern("Bool");

        /* new */
        case NODE_NEW: {
            const char *type = node->data.new_.type;
            if (classenv_lookup_class(s->env, type) == NULL) {
                char msg[128];
                snprintf(msg, sizeof(msg), "tipo '%s' não existe", type);
                semant_error(s, node->line, msg);
            }
            return type;
        }

        /* if */
        case NODE_IF: {
            const char *cond = check_expr(s, node->data.if_.expr_cond);
            if (strcmp(cond, "Bool") != 0)
                semant_error(s, node->line, "condição do if deve ser Bool");
            const char *then = check_expr(s, node->data.if_.expr_then);
            const char *els  = check_expr(s, node->data.if_.expr_else_);
            return classenv_lub(s->env, then, els);
        }

        /* while */
        case NODE_WHILE: {
            const char *cond = check_expr(s, node->data.while_.expr_cond);
            if (strcmp(cond, "Bool") != 0)
                semant_error(s, node->line, "condição do while deve ser Bool");
            check_expr(s, node->data.while_.expr);
            return intern("Object");
        }

        /* bloco */
        case NODE_BLOCK: {
            const char *type = "Object";
            NodeList *exprs = node->data.block.exprs;
            while (exprs != NULL) {
                type = check_expr(s, exprs->node);
                exprs = exprs->next;
            }
            return type;
        }

        /* let */
        case NODE_LET: {
            symtable_enter_scope(s->sym);
            NodeList *bindings = node->data.let.binding;
            while (bindings != NULL) {
                ASTNode *b = bindings->node;
                const char *decl_type = b->data.let_binding.type;
                if (classenv_lookup_class(s->env, decl_type) == NULL) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "tipo '%s' não existe no let", decl_type);
                    semant_error(s, b->line, msg);
                }
                if (b->data.let_binding.expr != NULL) {
                    const char *init_type = check_expr(s,
                                                b->data.let_binding.expr);
                    if (!classenv_is_subtype(s->env, init_type, decl_type)) {
                        char msg[128];
                        snprintf(msg, sizeof(msg),
                                 "tipo '%s' não é subtipo de '%s' no let",
                                 init_type, decl_type);
                        semant_error(s, b->line, msg);
                    }
                }
                symtable_add(s->sym, b->data.let_binding.name, decl_type);
                bindings = bindings->next;
            }
            const char *body_type = check_expr(s, node->data.let.expr);
            char *type_copy = strdup(body_type);
            symtable_exit_scope(s->sym);
            return body_type;
        }

        /* case */
        case NODE_CASE: {
            check_expr(s, node->data.case_.expr);
            const char *lub = NULL;
            NodeList *branches = node->data.case_.branches;
            while (branches != NULL) {
                ASTNode *b = branches->node;
                symtable_enter_scope(s->sym);
                symtable_add(s->sym,
                             b->data.case_branch.name,
                             b->data.case_branch.type);
                const char *branch_type = check_expr(s,
                                               b->data.case_branch.expr);
                symtable_exit_scope(s->sym);
                lub = lub == NULL
                      ? branch_type
                      : classenv_lub(s->env, lub, branch_type);
                branches = branches->next;
            }
            return lub ? lub : intern("Object");
        }

        /* dispatch dinâmico */
        case NODE_DISPATCH: {
            const char *obj_type = check_expr(s,
                                       node->data.dispatch.expr_object);
            MethodEntry *method  = classenv_lookup_method(s->env, obj_type,
                                       node->data.dispatch.method);
            if (method == NULL) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "método '%s' não existe em '%s'",
                         node->data.dispatch.method, obj_type);
                semant_error(s, node->line, msg);
                return intern("Object");
            }
            /* verifica argumentos */
            NodeList *args = node->data.dispatch.args;
            for (int i = 0; i < method->param_count; i++) {
                if (args == NULL) {
                    semant_error(s, node->line, "argumentos insuficientes");
                    break;
                }
                const char *arg_type = check_expr(s, args->node);
                if (!classenv_is_subtype(s->env, arg_type,
                                          method->param_types[i])) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "argumento %d: tipo '%s' não é subtipo de '%s'",
                             i + 1, arg_type, method->param_types[i]);
                    semant_error(s, node->line, msg);
                }
                args = args->next;
            }
            if (args != NULL)
                semant_error(s, node->line, "argumentos em excesso");
            return method->return_type;
        }

        /* dispatch estático */
        case NODE_STATIC_DISPATCH: {
            const char *obj_type = check_expr(s,
                                       node->data.static_dispatch.expr_object);
            const char *cast_type = node->data.static_dispatch.type;
            if (!classenv_is_subtype(s->env, obj_type, cast_type)) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "tipo '%s' não é subtipo de '%s' no dispatch estático",
                         obj_type, cast_type);
                semant_error(s, node->line, msg);
            }
            MethodEntry *method = classenv_lookup_method(s->env, cast_type,
                                      node->data.static_dispatch.method);
            if (method == NULL) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "método '%s' não existe em '%s'",
                         node->data.static_dispatch.method, cast_type);
                semant_error(s, node->line, msg);
                return intern("Object");
            }
            NodeList *args = node->data.static_dispatch.args;
            for (int i = 0; i < method->param_count; i++) {
                if (args == NULL) {
                    semant_error(s, node->line, "argumentos insuficientes");
                    break;
                }
                const char *arg_type = check_expr(s, args->node);
                if (!classenv_is_subtype(s->env, arg_type,
                                          method->param_types[i])) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "argumento %d: tipo '%s' não é subtipo de '%s'",
                             i + 1, arg_type, method->param_types[i]);
                    semant_error(s, node->line, msg);
                }
                args = args->next;
            }
            if (args != NULL)
                semant_error(s, node->line, "argumentos em excesso");
            return method->return_type;
        }

        /* self dispatch */
        case NODE_SELF_DISPATCH: {
            MethodEntry *method = classenv_lookup_method(s->env,
                                      s->current_class,
                                      node->data.self_dispatch.method);
            if (method == NULL) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "método '%s' não existe em '%s'",
                         node->data.self_dispatch.method, s->current_class);
                semant_error(s, node->line, msg);
                return intern("Object");
            }
            NodeList *args = node->data.self_dispatch.args;
            for (int i = 0; i < method->param_count; i++) {
                if (args == NULL) {
                    semant_error(s, node->line, "argumentos insuficientes");
                    break;
                }
                const char *arg_type = check_expr(s, args->node);
                if (!classenv_is_subtype(s->env, arg_type,
                                          method->param_types[i])) {
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                             "argumento %d: tipo '%s' não é subtipo de '%s'",
                             i + 1, arg_type, method->param_types[i]);
                    semant_error(s, node->line, msg);
                }
                args = args->next;
            }
            if (args != NULL)
                semant_error(s, node->line, "argumentos em excesso");
            return method->return_type;
        }

        default:
            semant_error(s, node->line, "expressão desconhecida");
            return intern("Object");
    }
}

/* ------------------------------------------------------------------ */
/* Verificação de features                                              */
/* ------------------------------------------------------------------ */

static void check_feature(SemantState *s, ASTNode *node) {
    if (node->kind == NODE_ATTR) {
        if (node->data.attr.expr != NULL) {
            const char *init_type = check_expr(s, node->data.attr.expr);
            if (!classenv_is_subtype(s->env, init_type,
                                      node->data.attr.type)) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                         "tipo '%s' não é subtipo de '%s' no atributo '%s'",
                         init_type, node->data.attr.type,
                         node->data.attr.name);
                semant_error(s, node->line, msg);
            }
        }
    } else if (node->kind == NODE_METHOD) {
        symtable_enter_scope(s->sym);

        /* adiciona parâmetros ao escopo */
        NodeList *formals = node->data.method.formals;
        while (formals != NULL) {
            ASTNode *f = formals->node;
            symtable_add(s->sym, f->data.formal.name, f->data.formal.type);
            formals = formals->next;
        }

        const char *body_type = check_expr(s, node->data.method.expr);

        if (!classenv_is_subtype(s->env, body_type,
                                  node->data.method.return_type)) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                     "corpo do método '%s' tem tipo '%s', esperado '%s'",
                     node->data.method.name, body_type,
                     node->data.method.return_type);
            semant_error(s, node->line, msg);
        }

        symtable_exit_scope(s->sym);
    }
}

/* ------------------------------------------------------------------ */
/* Verificação de classes                                               */
/* ------------------------------------------------------------------ */

static void check_class(SemantState *s, ASTNode *node) {
    s->current_class = node->data.class_.name;

    symtable_enter_scope(s->sym);

    /* adiciona atributos da classe ao escopo */
    NodeList *features = node->data.class_.features;
    while (features != NULL) {
        ASTNode *f = features->node;
        if (f->kind == NODE_ATTR)
            symtable_add(s->sym, f->data.attr.name, f->data.attr.type);
        features = features->next;
    }

    /* verifica cada feature */
    features = node->data.class_.features;
    while (features != NULL) {
        check_feature(s, features->node);
        features = features->next;
    }

    symtable_exit_scope(s->sym);
}

/* ------------------------------------------------------------------ */
/* Ponto de entrada                                                     */
/* ------------------------------------------------------------------ */

int semant_check(SemantState *s, ASTNode *program) {
    /* passagem 1: registra classes */
    collect_classes(s, program);

    /* passagem 2: registra métodos e atributos */
    collect_features(s, program);

    /* se há erros estruturais, não adianta continuar */
    if (s->errors > 0) return s->errors;

    /* passagem 3: verifica tipos em cada classe */
    NodeList *classes = program->data.program.classes;
    while (classes != NULL) {
        check_class(s, classes->node);
        classes = classes->next;
    }

    return s->errors;
}