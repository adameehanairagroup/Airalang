#ifndef AIRA_PARSER_H
#define AIRA_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
    const char* filename;
} Parser;

void parser_init(Parser* parser, const char* source, const char* filename);
StmtNode* parser_parse(Parser* parser);

#endif /* AIRA_PARSER_H */
