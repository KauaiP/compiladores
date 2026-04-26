#include <stdio.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

int main(void) {
    const char *src = "class Main {\n"
                      "    x : Int <- 42;\n"
                      "};\n";

    Lexer  *l = new_lexer(src);
    Parser *p = new_parser(l);
    ASTNode *ast = parse(p);

    if (ast != NULL && ast->kind == NODE_PROGRAM)
        printf("Parse OK\n");
    else
        printf("Parse falhou\n");

    free_parser(p);
    free_lexer(l);
    return 0;
}