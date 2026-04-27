#include <stdio.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

void run_test(const char *label, const char *src) {
    printf("=== %s ===\n", label);
    Lexer  *l = new_lexer(src);
    Parser *p = new_parser(l);
    ASTNode *ast = parse(p);
    if (ast != NULL && ast->kind == NODE_PROGRAM)
        printf("Parse OK\n\n");
    else
        printf("Parse falhou\n\n");
    free_parser(p);
    free_lexer(l);
}

int main(void) {

    run_test("tudo funcionando",
                "class Utility {\n"
        "    modulo(num: Int, mod: Int): Int {\n"
        "        num - (num / mod) * mod\n"
        "    };\n"
        "};\n"
        "\n"
        "class Random {\n"
        "    a: Int <- 1103515245;\n"
        "    c: Int <- 12345;\n"
        "    last: Int <- 1;\n"
        "\n"
        "    setSeed(seed: Int): Int {\n"
        "        last <- seed\n"
        "    };\n"
        "\n"
        "    next(): Int {\n"
        "        last <- last * a + c\n"
        "    };\n"
        "};\n"
        "\n"
        "class Main inherits IO {\n"
        "    main(): Object {\n"
        "        let i: Int,\n"
        "            min: Int <- 1,\n"
        "            max: Int <- 0\n"
        "        in {\n"
        "            out_string(\"inicio\\n\");\n"
        "            while not min < max loop {\n"
        "                min <- in_int();\n"
        "                max <- in_int();\n"
        "                if not min < max then\n"
        "                    out_string(\"erro\\n\")\n"
        "                else\n"
        "                    self\n"
        "                fi;\n"
        "            } pool;\n"
        "            i <- i + 1;\n"
        "        }\n"
        "    };\n"
        "};\n"
    );
    
    run_test("faltando fi",
        "class Main {\n"
        "    main(): Object {\n"
        "        if 1 < 2 then 1 else 2\n"  /* sem fi */
        "    };\n"
        "};\n"
    );
    
    run_test("faltando fechamento de bloco",
        "class Main {\n"
        "    main(): Object {\n"
        "        { 1; 2;\n"  /* sem } */
        "    };\n"
        "};\n"
    );

    run_test("faltando pool",
        "class Main {\n"
        "    main(): Object {\n"
        "        while true loop 1\n"  /* sem pool */
        "    };\n"
        "};\n"
    );

    run_test("faltando in no let",
        "class Main {\n"
        "    main(): Object {\n"
        "        let x: Int <- 1\n"  /* sem in */
        "    };\n"
        "};\n"
    );

    run_test("faltando esac",
        "class Main {\n"
        "    main(): Object {\n"
        "        case x of\n"
        "            x: Int => 1;\n"  /* sem esac */
        "    };\n"
        "};\n"
    );

    run_test("faltando ponto e virgula apos feature",
        "class Main {\n"
        "    main(): Object { 1 }\n"  /* sem ; após feature */
        "};\n"
    );

    return 0;
}