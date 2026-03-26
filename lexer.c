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

// Adicionado um novo parâmetro line para aumentar a precisão de erro do lexer_next
static Token skip_block_comment(Lexer *l, int start_line){

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
        return token_error("Erro dentro do bloco de comentario", start_line);
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

static Token read_number(Lexer *l) {

    int start = l->pos;
    int line  = l->line;
    while (!at_end(l) && isdigit(peek(l)))
        advance(l);
    int len = l->pos - start;
    char *s = strndup(l->src + start, len); // Desloca para o começo do número e copia o seu tamanho
    return make_token(INT, s, line);
}

// Struct anônima
static struct { 
    const char *word; 
    TokenType type; 
} KEYWORDS[] = {
    {"class",    CLASS},    {"else",   ELSE},
    {"fi",       FI},       {"if",     IF},
    {"in",       IN},       {"inherits",INHERITS},
    {"isvoid",   ISVOID},   {"let",    LET},
    {"loop",     LOOP},     {"pool",   POOL},
    {"then",     THEN},     {"while",  WHILE},
    {"case",     CASE},     {"esac",   ESAC},
    {"new",      NEW},      {"of",     OF},
    {"not",      NOT},
};

 // Define o N como tamanho total do vetor em bytes (padrão de C)
#define N_KEYWORDS (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

static Token read_ident(Lexer *l) {

    int start      = l->pos;
    int start_line = l->line;
    int first_upper = isupper(peek(l));

    // Avança carcteres de partes de ids
    while (!at_end(l) && (isalnum(peek(l)) || peek(l) == '_'))
        advance(l);

    int len = l->pos - start;
    char *raw = strndup(l->src + start, len);

    // Cria uma versão minúscula do identificador (case insensitive)
    char lower[256];
    for (int i = 0; i < len && i < 255; i++)
        lower[i] = tolower(raw[i]);
    lower[len] = '\0';

    if (!first_upper) {
        if (strcmp(lower, "true")  == 0) { // RETORNA 0 SE FOR IGUAL (POR ALGUM MOTIVO)
            free(raw); 
            return make_token(BOOL, strdup("true"),  start_line); 
        }
        if (strcmp(lower, "false") == 0) { 
            free(raw); 
            return make_token(BOOL, strdup("false"), start_line); 
        }

        // Keywords são case-insensitive
        for (size_t i = 0; i < N_KEYWORDS; i++) {
            if (strcmp(lower, KEYWORDS[i].word) == 0) { // Verifica se a palavra minúscula bate com a keyword
                free(raw);
                return make_token(KEYWORDS[i].type, NULL, start_line);
            }
        }
        // Se não é bool nem keyword, é um id de variável/método
        return make_token(OBJECT_ID, raw, start_line);
    }

    // Começa com maiúscula: TypeID (Verificar Self-Type depois)
    return make_token(TYPE_ID, raw, start_line);
}

// Função principal, a cada chamada, pula espaços/comentários e retorna o próximo token da entrada
Token lexer_next(Lexer *l) {
    while (1) {
        while (!at_end(l) && isspace(peek(l)))
            advance(l);

        if (at_end(l))
            return make_token(TOKEN_EOF, NULL, l->line);

        int line = l->line;
        char c = peek(l);

        if (c == '-' && peek2(l) == '-') {
            skip_line_comment(l);
            continue;
        }

        if (c == '(' && peek2(l) == '*') {
            Token t = skip_block_comment(l, line);
            if (t.type == ERROR) return t;
            continue;
        }

        // Operador *) fora de comentário é um erro
        if (c == '*' && peek2(l) == ')') {
            advance(l); advance(l);
            return token_error("Unmatched *)", line);
        }

        // String 
        if (c == '"') 
            return read_literal_string(l);

        // Número 
        if (isdigit(c)) 
            return read_number(l);

        // Identificador / keyword
        if (isalpha(c) || c == '_') 
            return read_ident(l);

        // Operadores multi-char (símbolos de operações/estrutura de código)
        advance(l);
        switch (c) {
            case '<':
                if (peek(l) == '-') { 
                    advance(l); 
                    return make_token(ASSIGN,  NULL, line); 
                }
                if (peek(l) == '=') { 
                    advance(l); 
                    return make_token(LE, NULL, line); 
                }
                return make_token(LT, NULL, line);
            case '=':
                if (peek(l) == '>') { 
                    advance(l); 
                    return make_token(DARROW,  NULL, line); 
                }
                return make_token(EQ, NULL, line);
            case '+': 
                return make_token(PLUS, NULL, line);
            case '-': 
                return make_token(MINUS, NULL, line);
            case '*': 
                return make_token(STAR, NULL, line);
            case '/': 
                return make_token(SLASH, NULL, line);
            case '~': 
                return make_token(TILDE, NULL, line);
            case '@': 
                return make_token(AT, NULL, line);
            case '.': 
                return make_token(DOT, NULL, line);
            case '{': 
                return make_token(LBRACE, NULL, line);
            case '}': 
                return make_token(RBRACE, NULL, line);
            case '(': 
                return make_token(LPAREN, NULL, line);
            case ')': 
                return make_token(RPAREN, NULL, line);
            case ':': 
                return make_token(COLON, NULL, line);
            case ';': 
                return make_token(SEMICOLON, NULL, line);
            case ',': 
                return make_token(COMMA, NULL, line);
            default: {
                char msg[32];
                snprintf(msg, sizeof(msg), "Unexpected character: %c", c);
                return token_error(msg, line);
            }
        }
    }
}

void token_free(Token t) { 
    free(t.value); 
}