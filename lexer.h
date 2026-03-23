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


    LBRACE,         // {
    RBRACE,         // }
    LPAREN,         // (
    RPAREN,         // )
    COLON,          // :
    SEMICOLON,      // ;
    COMMA,          // ,

    // Para controle do código:
    EOF,
    ERROR

}Token;

#endif