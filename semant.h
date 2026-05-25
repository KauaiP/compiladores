#ifndef SEMANT_H
#define SEMANT_H

#include "ast.h"
#include "symtable.h"
#include "classenv.h"

typedef struct {
    ClassEnv *env;
    SymTable *sym;
    char *current_class;   /* classe sendo verificada no momento */
    int errors;
} SemantState;

SemantState *semant_new(void);
void semant_free(SemantState *s);

/* ponto de entrada (retorna número de erros encontrados) */
int semant_check(SemantState *s, ASTNode *program);

#endif