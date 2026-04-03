#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv) {

    FILE *arquivo = fopen("teste.txt", "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir");
        return 1;
    }

    fseek(arquivo, 0, SEEK_END);
    long size = ftell(arquivo);
    rewind(arquivo);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, arquivo);
    buffer[size] = '\0';

    fclose(arquivo);

    Lexer *l = new_lexer(buffer);
    Token  t;

    while ((t = next_token(l)).type != TOKEN_EOF) {
        printf("Linha:%d tipo:%d valor:%s\n",
               t.line,
               t.type,
               t.value);
        free_token(t);
    }
    free_lexer(l);
    free(buffer);

    return 0;
}