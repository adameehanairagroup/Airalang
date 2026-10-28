#ifndef AIRA_LEXER_H
#define AIRA_LEXER_H

#include "token.h"

typedef struct {
    const char* source;
    const char* filename;
    size_t length;
    size_t current;
    int line;
    int col;
} Lexer;

void lexer_init(Lexer* lexer, const char* source, const char* filename);
Token lexer_next_token(Lexer* lexer);

#endif /* AIRA_LEXER_H */
