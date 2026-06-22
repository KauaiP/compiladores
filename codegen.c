#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

struct codegen {
    ClassEnv *env;      // para consultar tipos e métodos
    FILE     *out;      // arquivo .bril de saída
    int       tmp_count;   // contador de temporários
    int       label_count; // contador de labels
    const char *current_class;
};

// Gera um novo nome de temporário único: tmp0, tmp1, ...
static char *new_tmp(CodeGen *cg) {
    char *name = malloc(32);
    snprintf(name, 32, "tmp%d", cg->tmp_count++);
    return name;
}

// Gera um novo nome de label único: lbl0, lbl1, ...
static char *new_label(CodeGen *cg) {
    char *name = malloc(32);
    snprintf(name, 32, "lbl%d", cg->label_count++);
    return name;
}

CodeGen *codegen_new(ClassEnv *env, FILE *out) {
    CodeGen *cg = malloc(sizeof(CodeGen));
    cg->env         = env;
    cg->out         = out;
    cg->tmp_count   = 0;
    cg->label_count = 0;
    return cg;
}

void codegen_free(CodeGen *cg) {
    free(cg);
}

// Emite um label:  .nome:
static void emit_label(CodeGen *cg, const char *name) {
    fprintf(cg->out, ".%s:\n", name);
}

// Emite um salto incondicional:  jmp .label;
static void emit_jmp(CodeGen *cg, const char *label) {
    fprintf(cg->out, "  jmp .%s;\n", label);
}

// Emite um salto condicional:  br cond .then .else;
static void emit_br(CodeGen *cg, const char *cond,
                    const char *lthen, const char *lelse) {
    fprintf(cg->out, "  br %s .%s .%s;\n", cond, lthen, lelse);
}

// Emite uma constante inteira:  dest: int = const valor;
static char *emit_const_int(CodeGen *cg, int value) {
    char *dest = new_tmp(cg);
    fprintf(cg->out, "  %s: int = const %d;\n", dest, value);
    return dest;
}

// Emite uma constante booleana:  dest: bool = const true/false;
static char *emit_const_bool(CodeGen *cg, int value) {
    char *dest = new_tmp(cg);
    fprintf(cg->out, "  %s: bool = const %s;\n",
            dest, value ? "true" : "false");
    return dest;
}

// Emite uma operação binária:  dest: type = op left right;
static char *emit_binop(CodeGen *cg, const char *type,
                        const char *op,
                        const char *left, const char *right) {
    char *dest = new_tmp(cg);
    fprintf(cg->out, "  %s: %s = %s %s %s;\n",
            dest, type, op, left, right);
    return dest;
}

// Emite uma atribuição de identidade:  dest: type = id src;
static char *emit_id(CodeGen *cg, const char *type, const char *src) {
    char *dest = new_tmp(cg);
    fprintf(cg->out, "  %s: %s = id %s;\n", dest, type, src);
    return dest;
}

static const char *infer_type(CodeGen *cg, ASTNode *node) {
    if (node == NULL) return "Object";
    switch (node->kind) {
        case NODE_INT:    return "Int";
        case NODE_BOOL:   return "Bool";
        case NODE_STRING: return "String";
        case NODE_NEW:    return node->data.new_.type;
        case NODE_ID: {
            // consulta ClassEnv para atributos — simplificação
            return "Object";
        }
        default: return "Object";
    }
}

static char *emit_expr(CodeGen *cg, ASTNode *node) {
    if (node == NULL) return emit_const_int(cg, 0);

    switch (node->kind) {

        // --- Literais ---
        case NODE_INT:
            return emit_const_int(cg, node->data.int_lit.value);

        case NODE_BOOL:
            return emit_const_bool(cg, node->data.bool_lit.value);

        case NODE_STRING: {
            // Bril não tem string nativo — emitimos 0 como placeholder
            return emit_const_int(cg, 0);
        }

        // --- Identificador ---
        case NODE_ID: {
            // apenas referencia o temporário que já existe com esse nome
            return strdup(node->data.id.name);
        }

        // --- Operadores binários aritméticos ---
        case NODE_PLUS: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "int", "add", l, r);
            free(l); free(r);
            return d;
        }
        case NODE_MINUS: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "int", "sub", l, r);
            free(l); free(r);
            return d;
        }
        case NODE_MUL: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "int", "mul", l, r);
            free(l); free(r);
            return d;
        }
        case NODE_DIV: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "int", "div", l, r);
            free(l); free(r);
            return d;
        }

        // --- Comparações ---
        case NODE_LT: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "bool", "lt", l, r);
            free(l); free(r);
            return d;
        }
        case NODE_LE: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "int", "le", l, r);
            free(l); free(r);
            return d;
        }
        case NODE_EQ: {
            char *l = emit_expr(cg, node->data.binary.left_expr);
            char *r = emit_expr(cg, node->data.binary.right_expr);
            char *d = emit_binop(cg, "bool", "eq", l, r);
            free(l); free(r);
            return d;
        }

        // --- Unários ---
        case NODE_NEG: {
            char *operand = emit_expr(cg, node->data.unary.expr);
            char *zero    = emit_const_int(cg, 0);
            char *d       = emit_binop(cg, "int", "sub", zero, operand);
            free(operand); free(zero);
            return d;
        }
        case NODE_NOT: {
            char *operand = emit_expr(cg, node->data.unary.expr);
            char *d       = new_tmp(cg);
            fprintf(cg->out, "  %s: bool = not %s;\n", d, operand);
            free(operand);
            return d;
        }
        case NODE_ISVOID: {
            // Simplificação: isvoid sempre false para tipos primitivos
            return emit_const_bool(cg, 0);
        }

        // --- Assign ---
        case NODE_ASSIGN: {
            char *val = emit_expr(cg, node->data.assign.expr);
            // copia o valor para uma variável com o nome do identificador
            fprintf(cg->out, "  %s: int = id %s;\n",
                    node->data.assign.name, val);
            free(val);
            return strdup(node->data.assign.name);
        }

        // --- Bloco ---
        case NODE_BLOCK: {
            char *last = NULL;
            NodeList *exprs = node->data.block.exprs;
            while (exprs != NULL) {
                free(last);
                last = emit_expr(cg, exprs->node);
                exprs = exprs->next;
            }
            return last ? last : emit_const_int(cg, 0);
        }

        // --- If ---
        case NODE_IF: {
            char *lthen = new_label(cg);
            char *lelse = new_label(cg);
            char *lend  = new_label(cg);
            char *result = new_tmp(cg);

            char *cond = emit_expr(cg, node->data.if_.expr_cond);
            emit_br(cg, cond, lthen, lelse);
            free(cond);

            emit_label(cg, lthen);
            char *then = emit_expr(cg, node->data.if_.expr_then);
            fprintf(cg->out, "  %s: int = id %s;\n", result, then);
            free(then);
            emit_jmp(cg, lend);

            emit_label(cg, lelse);
            char *els = emit_expr(cg, node->data.if_.expr_else_);
            fprintf(cg->out, "  %s: int = id %s;\n", result, els);
            free(els);
            emit_jmp(cg, lend);

            emit_label(cg, lend);
            free(lthen); free(lelse); free(lend);
            return result;
        }

        // --- While ---
        case NODE_WHILE: {
            char *lcond = new_label(cg);
            char *lbody = new_label(cg);
            char *lend  = new_label(cg);

            emit_jmp(cg, lcond);
            emit_label(cg, lcond);
            char *cond = emit_expr(cg, node->data.while_.expr_cond);
            emit_br(cg, cond, lbody, lend);
            free(cond);

            emit_label(cg, lbody);
            char *body = emit_expr(cg, node->data.while_.expr);
            free(body);
            emit_jmp(cg, lcond);

            emit_label(cg, lend);
            free(lcond); free(lbody); free(lend);
            return emit_const_int(cg, 0); // while retorna Object ~ 0
        }

        // --- Let ---
        case NODE_LET: {
            NodeList *bindings = node->data.let.binding;
            while (bindings != NULL) {
                ASTNode *b = bindings->node;
                if (b->data.let_binding.expr != NULL) {
                    char *val = emit_expr(cg, b->data.let_binding.expr);
                    fprintf(cg->out, "  %s: int = id %s;\n",
                            b->data.let_binding.name, val);
                    free(val);
                } else {
                    // sem inicialização — default 0
                    fprintf(cg->out, "  %s: int = const 0;\n",
                            b->data.let_binding.name);
                }
                bindings = bindings->next;
            }
            return emit_expr(cg, node->data.let.expr);
        }

        // --- New ---
        case NODE_NEW: {
            // Simplificação: new retorna 0 como handle opaco
            return emit_const_int(cg, 0);
        }

        // --- Self dispatch ---
        case NODE_SELF_DISPATCH: {
            // Gera chamada: dest: int = call @ClassName_method self arg1 ...;
            char *dest = new_tmp(cg);
            fprintf(cg->out, "  %s: int = call @%s_%s self",
                    dest,
                    cg->current_class,
                    node->data.self_dispatch.method);
            NodeList *args = node->data.self_dispatch.args;
            while (args != NULL) {
                char *a = emit_expr(cg, args->node);
                fprintf(cg->out, " %s", a);
                free(a);
                args = args->next;
            }
            fprintf(cg->out, ";\n");
            return dest;
        }

        // --- Dispatch dinâmico ---
        case NODE_DISPATCH: {
            const char *obj_class = infer_type(cg, node->data.dispatch.expr_object);
            char *obj  = emit_expr(cg, node->data.dispatch.expr_object);
            char *dest = new_tmp(cg);
            fprintf(cg->out, "  %s: int = call @%s_%s %s",
                dest,
                obj_class,                       // ← tipo correto do objeto
                node->data.dispatch.method,
                obj);
            free(obj);
            NodeList *args = node->data.dispatch.args;
            while (args != NULL) {
                char *a = emit_expr(cg, args->node);
                fprintf(cg->out, " %s", a);
                free(a);
                args = args->next;
            }
            fprintf(cg->out, ";\n");
            return dest;
        }

        default:
            return emit_const_int(cg, 0);
    }
}

static void emit_method(CodeGen *cg, const char *class_name, ASTNode *node) {
    cg->current_class = class_name;

    // cabeçalho da função: @ClassName_method(self: int, param: int, ...)
    fprintf(cg->out, "@%s_%s(self: int",
            class_name, node->data.method.name);

    NodeList *formals = node->data.method.formals;
    while (formals != NULL) {
        fprintf(cg->out, ", %s: int", formals->node->data.formal.name);
        formals = formals->next;
    }
    fprintf(cg->out, ") {\n");

    char *result = emit_expr(cg, node->data.method.expr);
    fprintf(cg->out, "  ret %s;\n", result);
    free(result);

    fprintf(cg->out, "}\n\n");
}

static void emit_class(CodeGen *cg, ASTNode *node) {
    const char *class_name = node->data.class_.name;
    NodeList *features = node->data.class_.features;
    while (features != NULL) {
        ASTNode *f = features->node;
        if (f->kind == NODE_METHOD)
            emit_method(cg, class_name, f);
        features = features->next;
    }
}

void codegen_run(CodeGen *cg, ASTNode *program) {
    // gera todas as classes
    NodeList *classes = program->data.program.classes;
    while (classes != NULL) {
        emit_class(cg, classes->node);
        classes = classes->next;
    }

    // gera o @main que chama Main_main
    fprintf(cg->out, "@main {\n");
    fprintf(cg->out, "  self: int = const 0;\n");
    fprintf(cg->out, "  ret: int = call @Main_main self;\n");
    fprintf(cg->out, "}\n");
}