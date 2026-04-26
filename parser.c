#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

/* ------------------------------------------------------------------ */
/* Struct interno                                                       */
/* ------------------------------------------------------------------ */

struct parser {
    Lexer *lexer;
    Token current;
    Token next;
};

/* ------------------------------------------------------------------ */
/* Funções auxiliares                                                   */
/* ------------------------------------------------------------------ */

static void advance(Parser *p) {
    free_token(p->current);
    p->current = p->next;
    p->next = next_token(p->lexer);
}

static Token peek(Parser *p) {
    return p->current;
}

static int check(Parser *p, TokenType type) {
    return p->current.type == type;
}

static int match(Parser *p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return 1;
    }
    return 0;
}

static Token expect(Parser *p, TokenType type, const char *msg) {
    if (!check(p, type)) {
        fprintf(stderr, "Erro linha %d: %s\n", p->current.line, msg);
        exit(1);
    }
    Token t = p->current;
    advance(p);
    return t;
}

/* ------------------------------------------------------------------ */
/* Criação do parser                                                    */
/* ------------------------------------------------------------------ */

Parser *new_parser(Lexer *l) {
    Parser *p = malloc(sizeof(Parser));
    p->lexer = l;
    p->current = next_token(l);
    p->next = next_token(l);
    return p;
}

void free_parser(Parser *p) {
    free_token(p->current);
    free_token(p->next);
    free(p);
}

/* ------------------------------------------------------------------ */
/* Forward declarations                                                 */
/* ------------------------------------------------------------------ */

static ASTNode *parse_expression(Parser *p, int min_bp);
static ASTNode *parse_class(Parser *p);
static ASTNode *parse_feature(Parser *p);

/* ------------------------------------------------------------------ */
/* Pratt parser — binding powers                                        */
/* ------------------------------------------------------------------ */

static int binding_power(TokenType type) {
    switch (type) {
        case ASSIGN:    return 10;
        case NOT:       return 20;
        case LT:
        case LE:
        case EQ:        return 30;
        case PLUS:
        case MINUS:     return 40;
        case STAR:
        case SLASH:     return 50;
        case ISVOID:    return 60;
        case TILDE:     return 70;
        case AT:
        case DOT:       return 80;
        default:        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Pratt parser — nud (início de expressão)                            */
/* ------------------------------------------------------------------ */

static ASTNode *parse_nud(Parser *p) {
    Token t = peek(p);
    int line = t.line;

    /* Literais */
    if (check(p, INT)) {
        int value = atoi(t.value);
        advance(p);
        return ast_new_int(line, value);
    }

    if (check(p, STR_LIT)) {
        char *value = strdup(t.value);
        advance(p);
        ASTNode *node = ast_new_string(line, value);
        free(value);
        return node;
    }

    if (check(p, BOOL)) {
        int value = strcmp(t.value, "true") == 0 ? 1 : 0;
        advance(p);
        return ast_new_bool(line, value);
    }

    /* Identificador — pode ser ID simples, assign ou self_dispatch */
    if (check(p, OBJECT_ID)) {
        char *name = strdup(t.value);
        advance(p);

        /* assign: ID <- expr */
        if (check(p, ASSIGN)) {
            advance(p);
            ASTNode *expr = parse_expression(p, 0);
            ASTNode *node = ast_new_assign(line, name, expr);
            free(name);
            return node;
        }

        /* self_dispatch: ID( args ) */
        if (check(p, LPAREN)) {
            advance(p);
            NodeList *args = NULL;
            while (!check(p, RPAREN)) {
                args = ast_list_append(args, parse_expression(p, 0));
                if (!match(p, COMMA)) break;
            }
            expect(p, RPAREN, "esperado ')' após argumentos");
            ASTNode *node = ast_new_self_dispatch(line, name, args);
            free(name);
            return node;
        }

        ASTNode *node = ast_new_id(line, name);
        free(name);
        return node;
    }

    /* new TYPE */
    if (check(p, NEW)) {
        advance(p);
        Token type = expect(p, TYPE_ID, "esperado tipo após 'new'");
        return ast_new_new(line, type.value);
    }

    /* isvoid expr */
    if (check(p, ISVOID)) {
        advance(p);
        ASTNode *expr = parse_expression(p, binding_power(ISVOID));
        return ast_new_isvoid(line, expr);
    }

    /* ~ expr */
    if (check(p, TILDE)) {
        advance(p);
        ASTNode *expr = parse_expression(p, binding_power(TILDE));
        return ast_new_unary(NODE_NEG, line, expr);
    }

    /* not expr */
    if (check(p, NOT)) {
        advance(p);
        ASTNode *expr = parse_expression(p, binding_power(NOT));
        return ast_new_unary(NODE_NOT, line, expr);
    }

    /* ( expr ) */
    if (check(p, LPAREN)) {
        advance(p);
        ASTNode *expr = parse_expression(p, 0);
        expect(p, RPAREN, "esperado ')' após expressão");
        return expr;
    }

    /* { expr; expr; ... } */
    if (check(p, LBRACE)) {
        advance(p);
        NodeList *exprs = NULL;
        while (!check(p, RBRACE)) {
            exprs = ast_list_append(exprs, parse_expression(p, 0));
            expect(p, SEMICOLON, "esperado ';' após expressão no bloco");
        }
        expect(p, RBRACE, "esperado '}' para fechar bloco");
        return ast_new_block(line, exprs);
    }

    /* if expr then expr else expr fi */
    if (check(p, IF)) {
        advance(p);
        ASTNode *cond = parse_expression(p, 0);
        expect(p, THEN, "esperado 'then'");
        ASTNode *then = parse_expression(p, 0);
        expect(p, ELSE, "esperado 'else'");
        ASTNode *else_ = parse_expression(p, 0);
        expect(p, FI, "esperado 'fi'");
        return ast_new_if(line, cond, then, else_);
    }

    /* while expr loop expr pool */
    if (check(p, WHILE)) {
        advance(p);
        ASTNode *cond = parse_expression(p, 0);
        expect(p, LOOP, "esperado 'loop'");
        ASTNode *body = parse_expression(p, 0);
        expect(p, POOL, "esperado 'pool'");
        return ast_new_while(line, cond, body);
    }

    /* let ID : TYPE [<- expr] [, ...] in expr */
    if (check(p, LET)) {
        advance(p);
        NodeList *bindings = NULL;
        do {
            int bline = p->current.line;
            Token name = expect(p, OBJECT_ID, "esperado identificador no let");
            expect(p, COLON, "esperado ':' no let");
            Token type = expect(p, TYPE_ID, "esperado tipo no let");
            ASTNode *init = NULL;
            if (match(p, ASSIGN))
                init = parse_expression(p, 0);
            bindings = ast_list_append(bindings, ast_new_let_binding(bline, name.value, type.value, init));
        } while (match(p, COMMA));
        expect(p, IN, "esperado 'in' após bindings do let");
        ASTNode *body = parse_expression(p, 0);
        return ast_new_let(line, bindings, body);
    }

    /* case expr of ID : TYPE => expr; ... esac */
    if (check(p, CASE)) {
        advance(p);
        ASTNode *expr = parse_expression(p, 0);
        expect(p, OF, "esperado 'of' após expressão do case");
        NodeList *branches = NULL;
        while (!check(p, ESAC)) {
            int bline = p->current.line;
            Token name = expect(p, OBJECT_ID, "esperado identificador no branch");
            expect(p, COLON, "esperado ':' no branch");
            Token type = expect(p, TYPE_ID, "esperado tipo no branch");
            expect(p, DARROW, "esperado '=>' no branch");
            ASTNode *body = parse_expression(p, 0);
            expect(p, SEMICOLON, "esperado ';' após branch");
            branches = ast_list_append(branches, ast_new_case_branch(bline, name.value, type.value, body));
        }
        expect(p, ESAC, "esperado 'esac'");
        return ast_new_case(line, expr, branches);
    }

    fprintf(stderr, "Erro linha %d: expressão inesperada\n", line);
    exit(1);
}

/* ------------------------------------------------------------------ */
/* Pratt parser — led (operador no meio da expressão)                  */
/* ------------------------------------------------------------------ */

static ASTNode *parse_led(Parser *p, ASTNode *left) {
    Token t = peek(p);
    int line = t.line;

    /* Operadores binários */
    switch (t.type) {
        case PLUS:  advance(p); return ast_new_binary(NODE_PLUS,  line, left, parse_expression(p, binding_power(PLUS)));
        case MINUS: advance(p); return ast_new_binary(NODE_MINUS, line, left, parse_expression(p, binding_power(MINUS)));
        case STAR:  advance(p); return ast_new_binary(NODE_MUL,   line, left, parse_expression(p, binding_power(STAR)));
        case SLASH: advance(p); return ast_new_binary(NODE_DIV,   line, left, parse_expression(p, binding_power(SLASH)));
        case LT:    advance(p); return ast_new_binary(NODE_LT,    line, left, parse_expression(p, binding_power(LT)));
        case LE:    advance(p); return ast_new_binary(NODE_LE,    line, left, parse_expression(p, binding_power(LE)));
        case EQ:    advance(p); return ast_new_binary(NODE_EQ,    line, left, parse_expression(p, binding_power(EQ)));
        default: break;
    }

    /* dispatch: expr.ID( args ) */
    if (t.type == DOT) {
        advance(p);
        Token method = expect(p, OBJECT_ID, "esperado nome do método após '.'");
        expect(p, LPAREN, "esperado '(' após nome do método");
        NodeList *args = NULL;
        while (!check(p, RPAREN)) {
            args = ast_list_append(args, parse_expression(p, 0));
            if (!match(p, COMMA)) break;
        }
        expect(p, RPAREN, "esperado ')' após argumentos");
        return ast_new_dispatch(line, left, method.value, args);
    }

    /* static dispatch: expr@TYPE.ID( args ) */
    if (t.type == AT) {
        advance(p);
        Token type = expect(p, TYPE_ID, "esperado tipo após '@'");
        expect(p, DOT, "esperado '.' após tipo no dispatch estático");
        Token method = expect(p, OBJECT_ID, "esperado nome do método");
        expect(p, LPAREN, "esperado '('");
        NodeList *args = NULL;
        while (!check(p, RPAREN)) {
            args = ast_list_append(args, parse_expression(p, 0));
            if (!match(p, COMMA)) break;
        }
        expect(p, RPAREN, "esperado ')'");
        return ast_new_static_dispatch(line, left, type.value, method.value, args);
    }

    fprintf(stderr, "Erro linha %d: operador inesperado\n", line);
    exit(1);
}

/* ------------------------------------------------------------------ */
/* parse_expression — loop central do Pratt                            */
/* ------------------------------------------------------------------ */

static ASTNode *parse_expression(Parser *p, int min_bp) {
    ASTNode *left = parse_nud(p);

    while (binding_power(p->current.type) > min_bp) {
        left = parse_led(p, left);
    }

    return left;
}

/* ------------------------------------------------------------------ */
/* Recursive descent — estruturas da linguagem                         */
/* ------------------------------------------------------------------ */

static ASTNode *parse_formal(Parser *p) {
    int line = p->current.line;
    Token name = expect(p, OBJECT_ID, "esperado nome do parâmetro");
    expect(p, COLON, "esperado ':' após nome do parâmetro");
    Token type = expect(p, TYPE_ID, "esperado tipo do parâmetro");
    return ast_new_formal(line, name.value, type.value);
}

static ASTNode *parse_feature(Parser *p) {
    int line = p->current.line;
    Token name = expect(p, OBJECT_ID, "esperado nome de feature");
    expect(p, COLON, "esperado ':'");

    /* atributo: ID : TYPE [<- expr] */
    if (check(p, TYPE_ID)) {
        Token type = peek(p);
        advance(p);
        ASTNode *init = NULL;
        if (match(p, ASSIGN))
            init = parse_expression(p, 0);
        return ast_new_attr(line, name.value, type.value, init);
    }

    /* método: ID( formals ) : TYPE { expr } */
    expect(p, LPAREN, "esperado '(' no método");
    NodeList *formals = NULL;
    while (!check(p, RPAREN)) {
        formals = ast_list_append(formals, parse_formal(p));
        if (!match(p, COMMA)) break;
    }
    expect(p, RPAREN, "esperado ')' após parâmetros");
    expect(p, COLON, "esperado ':' após parâmetros");
    Token return_type = expect(p, TYPE_ID, "esperado tipo de retorno");
    expect(p, LBRACE, "esperado '{' no corpo do método");
    ASTNode *body = parse_expression(p, 0);
    expect(p, RBRACE, "esperado '}' para fechar método");
    return ast_new_method(line, name.value, formals, return_type.value, body);
}

static ASTNode *parse_class(Parser *p) {
    int line = p->current.line;
    expect(p, CLASS, "esperado 'class'");
    Token name = expect(p, TYPE_ID, "esperado nome da classe");

    char *parent = NULL;
    if (match(p, INHERITS)) {
        Token parent_tok = expect(p, TYPE_ID, "esperado nome da superclasse");
        parent = parent_tok.value;
    }

    expect(p, LBRACE, "esperado '{' após nome da classe");
    NodeList *features = NULL;
    while (!check(p, RBRACE)) {
        features = ast_list_append(features, parse_feature(p));
        expect(p, SEMICOLON, "esperado ';' após feature");
    }
    expect(p, RBRACE, "esperado '}' para fechar classe");
    expect(p, SEMICOLON, "esperado ';' após '}'");

    return ast_new_class(line, name.value, parent, features);
}

/* ------------------------------------------------------------------ */
/* Ponto de entrada                                                     */
/* ------------------------------------------------------------------ */

ASTNode *parse(Parser *p) {
    int line = p->current.line;
    NodeList *classes = NULL;
    while (!check(p, TOKEN_EOF)) {
        classes = ast_list_append(classes, parse_class(p));
    }
    return ast_new_program(line, classes);
};