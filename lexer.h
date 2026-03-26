#ifndef LEXER_H
#define LEXER_H

typedef enum {
    // Constants:
    INT, 
    STR, 
    BOLL,

    // Keywords:
    CLASS,
    ELSE,
    FALSE,
    FI,
    IF,
    IN,
    INHERITS,
    ISVOID,
    LET,
    LOOP,
    POOL,
    THEN,
    WHILE,
    CASE,
    ESAC,
    NEW,
    OF,
    NOT,
    TRUE,

    // Identifiers:
    TYPE_ID, //começam com maiúscula
    OBJECT_ID, // começam com minúscula

    // Precedence
    ASSIGN,         // <-
    LE,             // <=
    DARROW,         // =>
    LT,             // <
    EQ,             // =
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    DOT,            // .
    STATIC_DISPACH, // @
    TILDE,          // ~

    //literal de String
    STR_LIT,

    LBRACE,         // {
    RBRACE,         // }
    LPAREN,         // (
    RPAREN,         // )
    COLON,          // :
    SEMICOLON,      // ;
    COMMA,          // ,

    // Para controle do código:
    TOKEN_EOF,
    ERROR

}TokenType;

typedef struct token
{
    TokenType type;
    char* value;
    int line;
}Token;

typedef struct lexer Lexer;

Lexer* new_lexer(const char* source);
void free_lexer(Lexer* l);
Token next_token(Lexer* l);
void free_token(Token t);

#endif