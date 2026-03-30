#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv) {

   char buffer[100];
    FILE *arquivo = fopen("teste.txt", "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir");
        return 1;
    }

    // Limpa o buffer com zeros para evitar "lixo" de memória
    memset(buffer, 0, sizeof(buffer));

    // fread(onde_salvar, tamanho_do_item, quantos_itens, arquivo)
    // Lemos 99 para sobrar 1 espaço para o '\0' no final
    size_t bytesLidos = fread(buffer, 1, 99, arquivo);
    
    // Finaliza a string manualmente
    buffer[bytesLidos] = '\0';

    printf("Conteudo total lido:\n%s\n", buffer);
    printf("--------------------\n");
    printf("Bytes lidos: %zu\n", bytesLidos);

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

    return 0;
}