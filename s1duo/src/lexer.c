#include "../include/lexer.h"

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_LET: return "let";
        case TOKEN_SAY: return "say";
        case TOKEN_PRINT: return "print";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FOR: return "for";
        case TOKEN_IN: return "in";
        case TOKEN_FN: return "fn";
        case TOKEN_RETURN: return "return";
        case TOKEN_IMPORT: return "import";
        case TOKEN_FROM: return "from";
        case TOKEN_TRUE: return "true";
        case TOKEN_FALSE: return "false";
        case TOKEN_NULL: return "null";
        case TOKEN_AND: return "and";
        case TOKEN_OR: return "or";
        case TOKEN_NOT: return "not";
        case TOKEN_TRY: return "try";
        case TOKEN_CATCH: return "catch";
        case TOKEN_THROW: return "throw";
        case TOKEN_CLASS: return "class";
        case TOKEN_NEW: return "new";
        case TOKEN_THIS: return "this";
        case TOKEN_BREAK: return "break";
        case TOKEN_CONTINUE: return "continue";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_PERCENT: return "%";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_PLUS_ASSIGN: return "+=";
        case TOKEN_MINUS_ASSIGN: return "-=";
        case TOKEN_STAR_ASSIGN: return "*=";
        case TOKEN_SLASH_ASSIGN: return "/=";
        case TOKEN_MOD_ASSIGN: return "%=";
        case TOKEN_EQUAL_EQUAL: return "==";
        case TOKEN_BANG_EQUAL: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_BANG: return "!";
        case TOKEN_AND_AND: return "&&";
        case TOKEN_OR_OR: return "||";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_COMMA: return ",";
        case TOKEN_DOT: return ".";
        case TOKEN_COLON: return ":";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_AT: return "@";
        default: return "UNKNOWN";
    }
}

void token_free(Token* token) {
    if (token && token->text) {
        aira_free(token->text);
        token->text = NULL;
    }
}

void lexer_init(Lexer* lexer, const char* source, const char* filename) {
    lexer->source = source;
    lexer->filename = filename;
    lexer->length = source ? strlen(source) : 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->col = 1;
}

static bool is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->length;
}

static char peek(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    char c = lexer->source[lexer->current++];
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    advance(lexer);
    return true;
}

static Token make_token(Lexer* lexer, TokenType type, const char* text, int line, int col) {
    Token token;
    token.type = type;
    token.text = text ? aira_strdup(text) : NULL;
    token.line = line;
    token.col = col;
    token.num_val = 0.0;
    return token;
}

static Token make_error_token(Lexer* lexer, const char* message, int line, int col) {
    Token token;
    token.type = TOKEN_ERROR;
    token.text = aira_strdup(message);
    token.line = line;
    token.col = col;
    token.num_val = 0.0;
    return token;
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance(lexer);
        } else if (c == '#') {
            /* Single line python style comment */
            while (!is_at_end(lexer) && peek(lexer) != '\n') {
                advance(lexer);
            }
        } else if (c == '/' && peek_next(lexer) == '/') {
            /* Single line C style comment */
            advance(lexer);
            advance(lexer);
            while (!is_at_end(lexer) && peek(lexer) != '\n') {
                advance(lexer);
            }
        } else if (c == '/' && peek_next(lexer) == '*') {
            /* Multi-line comment */
            advance(lexer);
            advance(lexer);
            while (!is_at_end(lexer)) {
                if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                    advance(lexer);
                    advance(lexer);
                    break;
                }
                advance(lexer);
            }
        } else {
            break;
        }
    }
}

static TokenType check_keyword(const char* text) {
    if (strcmp(text, "let") == 0) return TOKEN_LET;
    if (strcmp(text, "say") == 0) return TOKEN_SAY;
    if (strcmp(text, "print") == 0) return TOKEN_PRINT;
    if (strcmp(text, "if") == 0) return TOKEN_IF;
    if (strcmp(text, "else") == 0) return TOKEN_ELSE;
    if (strcmp(text, "while") == 0) return TOKEN_WHILE;
    if (strcmp(text, "for") == 0) return TOKEN_FOR;
    if (strcmp(text, "in") == 0) return TOKEN_IN;
    if (strcmp(text, "fn") == 0) return TOKEN_FN;
    if (strcmp(text, "return") == 0) return TOKEN_RETURN;
    if (strcmp(text, "import") == 0) return TOKEN_IMPORT;
    if (strcmp(text, "from") == 0) return TOKEN_FROM;
    if (strcmp(text, "true") == 0) return TOKEN_TRUE;
    if (strcmp(text, "false") == 0) return TOKEN_FALSE;
    if (strcmp(text, "null") == 0) return TOKEN_NULL;
    if (strcmp(text, "and") == 0) return TOKEN_AND;
    if (strcmp(text, "or") == 0) return TOKEN_OR;
    if (strcmp(text, "not") == 0) return TOKEN_NOT;
    if (strcmp(text, "try") == 0) return TOKEN_TRY;
    if (strcmp(text, "catch") == 0) return TOKEN_CATCH;
    if (strcmp(text, "throw") == 0) return TOKEN_THROW;
    if (strcmp(text, "class") == 0) return TOKEN_CLASS;
    if (strcmp(text, "new") == 0) return TOKEN_NEW;
    if (strcmp(text, "this") == 0) return TOKEN_THIS;
    if (strcmp(text, "break") == 0) return TOKEN_BREAK;
    if (strcmp(text, "continue") == 0) return TOKEN_CONTINUE;
    return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Lexer* lexer, int start_line, int start_col) {
    size_t start = lexer->current - 1;
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        if (isalnum((unsigned char)c) || c == '_') {
            advance(lexer);
        } else {
            break;
        }
    }
    size_t len = lexer->current - start;
    char* text = aira_strndup(lexer->source + start, len);
    TokenType type = check_keyword(text);
    Token tok = make_token(lexer, type, text, start_line, start_col);
    aira_free(text);
    return tok;
}

static Token scan_number(Lexer* lexer, int start_line, int start_col) {
    size_t start = lexer->current - 1;
    bool has_dot = false;
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        if (isdigit((unsigned char)c)) {
            advance(lexer);
        } else if (c == '.' && isdigit((unsigned char)peek_next(lexer)) && !has_dot) {
            has_dot = true;
            advance(lexer);
        } else {
            break;
        }
    }
    size_t len = lexer->current - start;
    char* text = aira_strndup(lexer->source + start, len);
    Token tok = make_token(lexer, TOKEN_NUMBER, text, start_line, start_col);
    tok.num_val = strtod(text, NULL);
    aira_free(text);
    return tok;
}

static Token scan_string(Lexer* lexer, char quote, int start_line, int start_col) {
    AiraStringBuilder sb;
    sb_init(&sb);
    
    while (!is_at_end(lexer) && peek(lexer) != quote) {
        char c = advance(lexer);
        if (c == '\\') {
            if (is_at_end(lexer)) break;
            char esc = advance(lexer);
            switch (esc) {
                case 'n': sb_append_char(&sb, '\n'); break;
                case 't': sb_append_char(&sb, '\t'); break;
                case 'r': sb_append_char(&sb, '\r'); break;
                case '\\': sb_append_char(&sb, '\\'); break;
                case '\'': sb_append_char(&sb, '\''); break;
                case '"': sb_append_char(&sb, '"'); break;
                default: sb_append_char(&sb, esc); break;
            }
        } else {
            sb_append_char(&sb, c);
        }
    }

    if (is_at_end(lexer)) {
        sb_free(&sb);
        return make_error_token(lexer, "Unterminated string literal.", start_line, start_col);
    }

    advance(lexer); /* Consume closing quote */
    char* content = sb_finish(&sb);
    Token tok = make_token(lexer, TOKEN_STRING, content, start_line, start_col);
    aira_free(content);
    return tok;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace_and_comments(lexer);

    int start_line = lexer->line;
    int start_col = lexer->col;

    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF, NULL, start_line, start_col);
    }

    char c = advance(lexer);

    if (isalpha((unsigned char)c) || c == '_') {
        return scan_identifier(lexer, start_line, start_col);
    }

    if (isdigit((unsigned char)c)) {
        return scan_number(lexer, start_line, start_col);
    }

    if (c == '"' || c == '\'') {
        return scan_string(lexer, c, start_line, start_col);
    }

    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN, "(", start_line, start_col);
        case ')': return make_token(lexer, TOKEN_RPAREN, ")", start_line, start_col);
        case '{': return make_token(lexer, TOKEN_LBRACE, "{", start_line, start_col);
        case '}': return make_token(lexer, TOKEN_RBRACE, "}", start_line, start_col);
        case '[': return make_token(lexer, TOKEN_LBRACKET, "[", start_line, start_col);
        case ']': return make_token(lexer, TOKEN_RBRACKET, "]", start_line, start_col);
        case ',': return make_token(lexer, TOKEN_COMMA, ",", start_line, start_col);
        case '.': return make_token(lexer, TOKEN_DOT, ".", start_line, start_col);
        case ':': return make_token(lexer, TOKEN_COLON, ":", start_line, start_col);
        case ';': return make_token(lexer, TOKEN_SEMICOLON, ";", start_line, start_col);
        case '@': return make_token(lexer, TOKEN_AT, "@", start_line, start_col);

        case '+':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_PLUS_ASSIGN, "+=", start_line, start_col);
            return make_token(lexer, TOKEN_PLUS, "+", start_line, start_col);
        case '-':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_MINUS_ASSIGN, "-=", start_line, start_col);
            return make_token(lexer, TOKEN_MINUS, "-", start_line, start_col);
        case '*':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_STAR_ASSIGN, "*=", start_line, start_col);
            return make_token(lexer, TOKEN_STAR, "*", start_line, start_col);
        case '/':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_SLASH_ASSIGN, "/=", start_line, start_col);
            return make_token(lexer, TOKEN_SLASH, "/", start_line, start_col);
        case '%':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_MOD_ASSIGN, "%=", start_line, start_col);
            return make_token(lexer, TOKEN_PERCENT, "%", start_line, start_col);

        case '=':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_EQUAL_EQUAL, "==", start_line, start_col);
            return make_token(lexer, TOKEN_ASSIGN, "=", start_line, start_col);

        case '!':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_BANG_EQUAL, "!=", start_line, start_col);
            return make_token(lexer, TOKEN_BANG, "!", start_line, start_col);

        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_LESS_EQUAL, "<=", start_line, start_col);
            return make_token(lexer, TOKEN_LESS, "<", start_line, start_col);

        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_GREATER_EQUAL, ">=", start_line, start_col);
            return make_token(lexer, TOKEN_GREATER, ">", start_line, start_col);

        case '&':
            if (match(lexer, '&')) return make_token(lexer, TOKEN_AND_AND, "&&", start_line, start_col);
            break;

        case '|':
            if (match(lexer, '|')) return make_token(lexer, TOKEN_OR_OR, "||", start_line, start_col);
            break;
    }

    char err_buf[64];
    snprintf(err_buf, sizeof(err_buf), "Unexpected character '%c' (0x%02x)", c, (unsigned char)c);
    return make_error_token(lexer, err_buf, start_line, start_col);
}
