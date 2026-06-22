#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "classenv.h"
#include <stdio.h>

typedef struct codegen CodeGen;

CodeGen *codegen_new(ClassEnv *env, FILE *out);
void     codegen_free(CodeGen *cg);
void     codegen_run(CodeGen *cg, ASTNode *program);

#endif