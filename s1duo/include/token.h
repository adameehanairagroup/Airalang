#ifndef AIRA_TOKEN_H
#define AIRA_TOKEN_H

#include "common.h"

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    /* Literals */
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,

    /* Keywords */
    TOKEN_LET,
    TOKEN_SAY,
    TOKEN_PRINT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_IMPORT,
    TOKEN_FROM,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_THROW,
    TOKEN_CLASS,
    TOKEN_NEW,
    TOKEN_THIS,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    /* Arithmetic & Assignment */
    TOKEN_PLUS,         /* + */
    TOKEN_MINUS,        /* - */
    TOKEN_STAR,         /* * */
    TOKEN_SLASH,        /* / */
    TOKEN_PERCENT,      /* % */
    TOKEN_ASSIGN,       /* = */
    TOKEN_PLUS_ASSIGN,  /* += */
    TOKEN_MINUS_ASSIGN, /* -= */
    TOKEN_STAR_ASSIGN,  /* *= */
    TOKEN_SLASH_ASSIGN, /* /= */
    TOKEN_MOD_ASSIGN,   /* %= */

    /* Comparison & Logical */
    TOKEN_EQUAL_EQUAL,  /* == */
    TOKEN_BANG_EQUAL,   /* != */
    TOKEN_LESS,         /* < */
    TOKEN_LESS_EQUAL,   /* <= */
    TOKEN_GREATER,      /* > */
    TOKEN_GREATER_EQUAL,/* >= */
    TOKEN_BANG,         /* ! */
    TOKEN_AND_AND,      /* && */
    TOKEN_OR_OR,        /* || */

    /* Delimiters & Punctuation */
    TOKEN_LPAREN,       /* ( */
    TOKEN_RPAREN,       /* ) */
    TOKEN_LBRACE,       /* { */
    TOKEN_RBRACE,       /* } */
    TOKEN_LBRACKET,     /* [ */
    TOKEN_RBRACKET,     /* ] */
    TOKEN_COMMA,        /* , */
    TOKEN_DOT,          /* . */
    TOKEN_COLON,        /* : */
    TOKEN_SEMICOLON,    /* ; */
    TOKEN_AT            /* @ */
} TokenType;

typedef struct {
    TokenType type;
    char* text;
    int line;
    int col;
    double num_val;
} Token;

const char* token_type_name(TokenType type);
void token_free(Token* token);

#endif /* AIRA_TOKEN_H */
