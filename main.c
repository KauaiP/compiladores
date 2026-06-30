#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "semant.h"
#include "classenv.h"
#include "codegen.h"
#include "intern.h"

int main(int argc, char **argv) {

    // lê o arquivo fonte
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *src = malloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);

    // fases do compilador
    Lexer   *l = new_lexer(src);
    Parser  *p = new_parser(l);
    ASTNode *ast = parse(p);

    SemantState *s = semant_new();
    int errors = semant_check(s, ast);
    if (errors > 0) {
        fprintf(stderr, "%d erro(s) semântico(s) — abortando\n", errors);
        return 1;
    }

    // geração de código para stdout
    CodeGen *cg = codegen_new(s->env, stdout);
    codegen_run(cg, ast);

    codegen_free(cg);
    semant_free(s);
    free_parser(p);
    free_lexer(l);
    free(src);
    intern_free();
    return 0;
}