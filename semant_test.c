#include <stdio.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symtable.h"
#include "classenv.h"
#include "semant.h"

void run_test(const char *label, const char *src, int expect_errors) {
    printf("=== %s ===\n", label);

    Lexer *l = new_lexer(src);
    Parser *p = new_parser(l);
    ASTNode *ast = parse(p);
    SemantState *s = semant_new();

    int errors = semant_check(s, ast);

    if (errors == expect_errors)
        printf("OK — %d erro(s) esperado(s) e encontrado(s)\n\n", errors);
    else
        printf("FALHOU — esperado %d erro(s), encontrado %d\n\n",
               expect_errors, errors);

    semant_free(s);
    free_parser(p);
    free_lexer(l);
}

int main(void) {

    /* programa correto simples */
    run_test("classe simples",
        "class Main {\n"
        "    x : Int <- 42;\n"
        "    get() : Int { x };\n"
        "};\n",
        0
    );

    /* herança válida */
    run_test("herança válida",
        "class Animal {\n"
        "    falar() : String { \"...\" };\n"
        "};\n"
        "class Gato inherits Animal {\n"
        "    falar() : String { \"miau\" };\n"
        "};\n",
        0
    );

    /* herança de tipo primitivo */
    run_test("herança de Int — deve falhar",
        "class Erro inherits Int {\n"
        "};\n",
        1
    );

    /* classe redefinida */
    run_test("classe redefinida — deve falhar",
        "class Foo { };\n"
        "class Foo { };\n",
        1
    );

    /* variável não declarada */
    run_test("variável não declarada — deve falhar",
        "class Main {\n"
        "    main() : Int { x };\n"
        "};\n",
        1
    );

    /* tipo errado na atribuição */
    run_test("tipo errado na atribuição — deve falhar",
        "class Main {\n"
        "    x : Int <- 0;\n"
        "    main() : Object { x <- \"string\" };\n"
        "};\n",
        1
    );

    /* if com condição não Bool */
    run_test("if com condição não Bool — deve falhar",
        "class Main {\n"
        "    main() : Object { if 42 then 1 else 2 fi };\n"
        "};\n",
        1
    );

    /* while com condição não Bool */
    run_test("while com condição não Bool — deve falhar",
        "class Main {\n"
        "    main() : Object { while 1 loop 2 pool };\n"
        "};\n",
        1
    );

    /* aritmética com tipos errados */
    run_test("aritmética com String — deve falhar",
        "class Main {\n"
        "    main() : Object { \"a\" + \"b\" };\n"
        "};\n",
        1
    );

    /* let correto */
    run_test("let correto",
        "class Main {\n"
        "    main() : Int {\n"
        "        let x : Int <- 1, y : Int <- 2 in x + y\n"
        "    };\n"
        "};\n",
        0
    );

    /* let com tipo errado na inicialização */
    run_test("let com tipo errado — deve falhar",
        "class Main {\n"
        "    main() : Object {\n"
        "        let x : Int <- \"erro\" in x\n"
        "    };\n"
        "};\n",
        1
    );

    /* tipo de retorno errado */
    run_test("tipo de retorno errado — deve falhar",
        "class Main {\n"
        "    main() : Int { \"nao sou int\" };\n"
        "};\n",
        1
    );

    /* new de tipo inexistente */
    run_test("new de tipo inexistente — deve falhar",
        "class Main {\n"
        "    main() : Object { new TipoFantasma };\n"
        "};\n",
        1
    );

    /* dispatch correto */
    run_test("dispatch correto",
        "class Calc {\n"
        "    soma(a : Int, b : Int) : Int { a + b };\n"
        "};\n"
        "class Main {\n"
        "    main() : Int {\n"
        "        let c : Calc <- new Calc in c.soma(1, 2)\n"
        "    };\n"
        "};\n",
        0
    );

    /* dispatch com argumentos errados */
    run_test("dispatch com argumento errado — deve falhar",
        "class Calc {\n"
        "    soma(a : Int, b : Int) : Int { a + b };\n"
        "};\n"
        "class Main {\n"
        "    main() : Int {\n"
        "        let c : Calc <- new Calc in c.soma(\"x\", 2)\n"
        "    };\n"
        "};\n",
        1
    );

    /* case correto */
    run_test("case correto",
        "class Main {\n"
        "    main() : Object {\n"
        "        case 42 of\n"
        "            x : Int => x + 1;\n"
        "            y : Object => y;\n"
        "        esac\n"
        "    };\n"
        "};\n",
        0
    );

    return 0;
}