#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct parser Parser;

Parser *new_parser(Lexer *l);
void free_parser(Parser* p);
ASTNode *parse(Parser *p);

#endif