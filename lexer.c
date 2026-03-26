#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct lexer{
    const char *src;
    int line;
    int pos;
};

Lexer *new_lexer(const char *source){

    Lexer *new = malloc(sizeof(Lexer));
    new->src = source;
    new->line = 1;
    new->pos = 0;
    return new;
}

void free_lexer(Lexer *l){

    free(l);
}

static char peek(Lexer *l){ // aqui estamos olhando (retornando) o caracter atual

    return l->src[l->pos];
}

static char peek_next(Lexer *l){ //aqui estamos olhando (retornando) o carcter poestrior para análise

    return l->src[l->pos + 1];
}

static void advanced(Lexer *l){

    l->pos++;
    char c = l->src[l->pos];
    if (c == '\n')
    {
        l->line++;
    }
}

static int at_end(Lexer *l){

    return l->src[l->pos] == '\0';
}

static Token make_token(TokenType type, char *value, int line){ //cria um token para análise

    Token t;
    t.type = type;
    t.value = value;
    t.line = line;
    return t;
}

static Token token_error(const char *msg, int line){ //cria um token de erro

    return make_token(ERROR, strdup(msg), line);
}

//strdup() é uma função que aloca uma string em outra variável. OBS: tem que fazer o free() nessa nova variável

// A sessão seguinte é voltada para as funções que ignoram comentários

static void skip_line_comment(Lexer *l){

    while (!at_end(l) && peek(l) != '\n')
    {
        advanced(l);
    }   
}

static Token skip_block_comment(Lexer *l){

    int level = 1;
    advanced(l);
    advanced(l);
    // consome "(" e o "*"

    while(!at_end(l) && level > 0){

        if (peek(l) == '(' && peek_next(l) == '*')
        {
            level++; //estamos aumentando mais um nível no comentário
        }
        else if (peek(l) == '*' && peek_next(l) == ')'){
            level--;
        }
        else
        {
            advanced(l);
        }
        
    }

    if (level > 0)
    {
        return token_error("Erro dentro do bloco de comentario", l->line);
    }

    return make_token(TOKEN_EOF, NULL, l->line); 
    /*retorna um token vazio, apenhas dizendo que está
    tudo ok dentro do bloco de comentários */
    
}

static Token read_literal_string(Lexer *l){

    int start_line = l->line;
    advanced(l); // consumindo o caracter ' " '

    char buffer[1024];
    int len = 0;

    while (!at_end(l) && peek(l) != '"')
    {
        if (len >= (int)sizeof(buffer) - 1)
        {
            return token_error("String muito longa", start_line);
        }

        char c = peek(l);

        if (c == '\0')
        {
            return token_error("Caracter nulo no meio da string", start_line); // melhorar o nome de erro, talvez seja "string mal terminada"
        }

        if (c == '\n')
        {
            return token_error("Literal de string não terminado", start_line);
        }
        
        if (c == '\\')
        {
            advanced(l);
            char next_char = peek(l);

            switch (next_char)
            {
            case 'n': buffer[len++] = '\n'; break;
            case 't': buffer[len++] = '\t'; break;
            case 'b': buffer[len++] = '\b'; break;
            case 'f': buffer[len++] = '\f'; break;
            case '\\': buffer[len++] = '\\'; break;
            case '"': buffer[len++] = '"'; break;
            case '\n': buffer[len++] = '\n'; break;    
            
            default: buffer[len++] = next_char; break;

            }
            advanced(l);
        }

        else
        {
            buffer[len++] = c;
            advanced(l);
        }  
    }

    if (at_end(l))
    {
        return token_error("Literal de string nao terminado", start_line);
    }

    advanced(l); //consome o '"' no final
    buffer[len] = '\0';
    make_token(STR_LIT, strdup(buffer), start_line);

}


