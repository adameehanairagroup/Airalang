#include "../include/parser.h"

static void advance(Parser* parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = lexer_next_token(&parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        aira_error_at(parser->filename, parser->current.line, parser->current.col, "%s", parser->current.text);
        parser->had_error = true;
    }
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    aira_error_at(parser->filename, parser->current.line, parser->current.col, "%s", message);
    parser->had_error = true;
}

static void match_semicolon(Parser* parser) {
    if (match(parser, TOKEN_SEMICOLON)) return;
    /* Lenient if next is RBRACE or EOF */
    if (check(parser, TOKEN_RBRACE) || check(parser, TOKEN_EOF)) return;
}

/* Forward declarations */
static ExprNode* expression(Parser* parser);
static StmtNode* statement(Parser* parser);
static StmtNode* block(Parser* parser);

/* Expression Parsing with Precedence Climbing */

static ExprNode* primary(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;

    if (match(parser, TOKEN_NUMBER)) {
        double val = parser->previous.num_val;
        if (val == (int64_t)val) {
            return expr_literal(val_int((int64_t)val), line, col);
        } else {
            return expr_literal(val_float(val), line, col);
        }
    }

    if (match(parser, TOKEN_STRING)) {
        return expr_literal(val_string(parser->previous.text), line, col);
    }

    if (match(parser, TOKEN_TRUE)) {
        return expr_literal(val_bool(true), line, col);
    }

    if (match(parser, TOKEN_FALSE)) {
        return expr_literal(val_bool(false), line, col);
    }

    if (match(parser, TOKEN_NULL)) {
        return expr_literal(val_null(), line, col);
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        return expr_identifier(parser->previous.text, line, col);
    }

    if (match(parser, TOKEN_LPAREN)) {
        ExprNode* expr = expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression.");
        return expr;
    }

    /* List Literal: [1, 2, 3] */
    if (match(parser, TOKEN_LBRACKET)) {
        int cap = 8;
        int count = 0;
        ExprNode** elements = (ExprNode**)aira_malloc(sizeof(ExprNode*) * cap);

        if (!check(parser, TOKEN_RBRACKET)) {
            do {
                if (count >= cap) {
                    cap *= 2;
                    elements = (ExprNode**)aira_realloc(elements, sizeof(ExprNode*) * cap);
                }
                elements[count++] = expression(parser);
            } while (match(parser, TOKEN_COMMA));
        }
        consume(parser, TOKEN_RBRACKET, "Expected ']' after list elements.");
        return expr_list(elements, count, line, col);
    }

    /* Dict Literal: {"key": value} */
    if (match(parser, TOKEN_LBRACE)) {
        int cap = 8;
        int count = 0;
        char** keys = (char**)aira_malloc(sizeof(char*) * cap);
        ExprNode** values = (ExprNode**)aira_malloc(sizeof(ExprNode*) * cap);

        if (!check(parser, TOKEN_RBRACE)) {
            do {
                if (count >= cap) {
                    cap *= 2;
                    keys = (char**)aira_realloc(keys, sizeof(char*) * cap);
                    values = (ExprNode**)aira_realloc(values, sizeof(ExprNode*) * cap);
                }
                if (!match(parser, TOKEN_STRING) && !match(parser, TOKEN_IDENTIFIER)) {
                    aira_error_at(parser->filename, parser->current.line, parser->current.col, "Expected string or identifier key in dictionary.");
                    break;
                }
                keys[count] = aira_strdup(parser->previous.text);
                consume(parser, TOKEN_COLON, "Expected ':' after dictionary key.");
                values[count] = expression(parser);
                count++;
            } while (match(parser, TOKEN_COMMA));
        }
        consume(parser, TOKEN_RBRACE, "Expected '}' after dictionary.");
        return expr_dict(keys, values, count, line, col);
    }

    aira_error_at(parser->filename, parser->current.line, parser->current.col, "Unexpected token '%s' in expression.", parser->current.text ? parser->current.text : "EOF");
    parser->had_error = true;
    advance(parser);
    return expr_literal(val_null(), line, col);
}

static ExprNode* call_or_access(Parser* parser) {
    ExprNode* expr = primary(parser);

    for (;;) {
        int line = parser->current.line;
        int col = parser->current.col;

        if (match(parser, TOKEN_LPAREN)) {
            /* Function call */
            int cap = 4;
            int count = 0;
            ExprNode** args = (ExprNode**)aira_malloc(sizeof(ExprNode*) * cap);

            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    if (count >= cap) {
                        cap *= 2;
                        args = (ExprNode**)aira_realloc(args, sizeof(ExprNode*) * cap);
                    }
                    args[count++] = expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            consume(parser, TOKEN_RPAREN, "Expected ')' after function arguments.");
            expr = expr_call(expr, args, count, line, col);
        } else if (match(parser, TOKEN_LBRACKET)) {
            /* Index access */
            ExprNode* index = expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after index.");
            expr = expr_index(expr, index, line, col);
        } else if (match(parser, TOKEN_DOT)) {
            /* Member access */
            consume(parser, TOKEN_IDENTIFIER, "Expected member name after '.'.");
            expr = expr_member(expr, parser->previous.text, line, col);
        } else {
            break;
        }
    }
    return expr;
}

static ExprNode* unary(Parser* parser) {
    if (match(parser, TOKEN_BANG) || match(parser, TOKEN_NOT) || match(parser, TOKEN_MINUS) || match(parser, TOKEN_PLUS)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = unary(parser);
        return expr_unary(op, right, line, col);
    }
    return call_or_access(parser);
}

static ExprNode* factor(Parser* parser) {
    ExprNode* expr = unary(parser);

    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) || match(parser, TOKEN_PERCENT)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = unary(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* term(Parser* parser) {
    ExprNode* expr = factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = factor(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* comparison(Parser* parser) {
    ExprNode* expr = term(parser);

    while (match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL) ||
           match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = term(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* equality(Parser* parser) {
    ExprNode* expr = comparison(parser);

    while (match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = comparison(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* logical_and(Parser* parser) {
    ExprNode* expr = equality(parser);

    while (match(parser, TOKEN_AND_AND) || match(parser, TOKEN_AND)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = equality(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* logical_or(Parser* parser) {
    ExprNode* expr = logical_and(parser);

    while (match(parser, TOKEN_OR_OR) || match(parser, TOKEN_OR)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        int col = parser->previous.col;
        ExprNode* right = logical_and(parser);
        expr = expr_binary(op, expr, right, line, col);
    }
    return expr;
}

static ExprNode* expression(Parser* parser) {
    return logical_or(parser);
}

/* Statement Parsing */

static StmtNode* let_declaration(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    consume(parser, TOKEN_IDENTIFIER, "Expected variable name after 'let'.");
    char* name = aira_strdup(parser->previous.text);

    ExprNode* init = NULL;
    if (match(parser, TOKEN_ASSIGN)) {
        init = expression(parser);
    } else {
        init = expr_literal(val_null(), line, col);
    }
    match_semicolon(parser);
    StmtNode* s = stmt_let(name, init, line, col);
    aira_free(name);
    return s;
}

static StmtNode* say_or_print_statement(Parser* parser, bool is_say) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    int cap = 4;
    int count = 0;
    ExprNode** exprs = (ExprNode**)aira_malloc(sizeof(ExprNode*) * cap);

    /* Check if print has parentheses like print("hello") */
    bool has_paren = match(parser, TOKEN_LPAREN);

    if (!has_paren || !check(parser, TOKEN_RPAREN)) {
        do {
            if (count >= cap) {
                cap *= 2;
                exprs = (ExprNode**)aira_realloc(exprs, sizeof(ExprNode*) * cap);
            }
            exprs[count++] = expression(parser);
        } while (match(parser, TOKEN_COMMA));
    }

    if (has_paren) {
        consume(parser, TOKEN_RPAREN, "Expected ')' after arguments.");
    }

    match_semicolon(parser);
    if (is_say) {
        return stmt_say(exprs, count, line, col);
    } else {
        return stmt_print(exprs, count, line, col);
    }
}

static StmtNode* if_statement(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    bool has_paren = match(parser, TOKEN_LPAREN);
    ExprNode* condition = expression(parser);
    if (has_paren) {
        consume(parser, TOKEN_RPAREN, "Expected ')' after if condition.");
    }

    StmtNode* then_branch = statement(parser);
    StmtNode* else_branch = NULL;

    if (match(parser, TOKEN_ELSE)) {
        else_branch = statement(parser);
    }
    return stmt_if(condition, then_branch, else_branch, line, col);
}

static StmtNode* while_statement(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    bool has_paren = match(parser, TOKEN_LPAREN);
    ExprNode* condition = expression(parser);
    if (has_paren) {
        consume(parser, TOKEN_RPAREN, "Expected ')' after while condition.");
    }
    StmtNode* body = statement(parser);
    return stmt_while(condition, body, line, col);
}

static StmtNode* for_statement(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    consume(parser, TOKEN_IDENTIFIER, "Expected variable name after 'for'.");
    char* var_name = aira_strdup(parser->previous.text);

    consume(parser, TOKEN_IN, "Expected 'in' after for variable.");
    ExprNode* iterable = expression(parser);
    StmtNode* body = statement(parser);

    StmtNode* s = stmt_for_in(var_name, iterable, body, line, col);
    aira_free(var_name);
    return s;
}

static StmtNode* fn_declaration(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    consume(parser, TOKEN_IDENTIFIER, "Expected function name after 'fn'.");
    char* name = aira_strdup(parser->previous.text);

    consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
    int cap = 4;
    int count = 0;
    char** params = (char**)aira_malloc(sizeof(char*) * cap);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (count >= cap) {
                cap *= 2;
                params = (char**)aira_realloc(params, sizeof(char*) * cap);
            }
            consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");
            params[count++] = aira_strdup(parser->previous.text);
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
    StmtNode* body = block(parser);

    StmtNode* s = stmt_fn_def(name, params, count, body, line, col);
    aira_free(name);
    return s;
}

static StmtNode* return_statement(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;
    ExprNode* expr = NULL;
    if (!check(parser, TOKEN_SEMICOLON) && !check(parser, TOKEN_RBRACE)) {
        expr = expression(parser);
    }
    match_semicolon(parser);
    return stmt_return(expr, line, col);
}

static StmtNode* import_statement(Parser* parser) {
    int line = parser->previous.line;
    int col = parser->previous.col;

    char* mod_name = NULL;
    if (match(parser, TOKEN_STRING) || match(parser, TOKEN_IDENTIFIER)) {
        mod_name = aira_strdup(parser->previous.text);
    } else {
        aira_error_at(parser->filename, line, col, "Expected module name after 'import'.");
        mod_name = aira_strdup("unknown");
    }

    char* source = NULL;
    if (match(parser, TOKEN_FROM)) {
        if (match(parser, TOKEN_STRING) || match(parser, TOKEN_IDENTIFIER)) {
            source = aira_strdup(parser->previous.text);
        }
    }

    match_semicolon(parser);
    StmtNode* s = stmt_import(mod_name, source, NULL, line, col);
    aira_free(mod_name);
    if (source) aira_free(source);
    return s;
}

static StmtNode* block(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;
    consume(parser, TOKEN_LBRACE, "Expected '{' before block.");

    int cap = 8;
    int count = 0;
    StmtNode** statements = (StmtNode**)aira_malloc(sizeof(StmtNode*) * cap);

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        if (count >= cap) {
            cap *= 2;
            statements = (StmtNode**)aira_realloc(statements, sizeof(StmtNode*) * cap);
        }
        statements[count++] = statement(parser);
    }
    consume(parser, TOKEN_RBRACE, "Expected '}' after block.");
    return stmt_block(statements, count, line, col);
}

static StmtNode* expression_statement(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;

    /* Check for assignment target: ident = expr, ident += expr, target[idx] = expr, target.mem = expr */
    ExprNode* expr = expression(parser);

    if (match(parser, TOKEN_ASSIGN) || match(parser, TOKEN_PLUS_ASSIGN) ||
        match(parser, TOKEN_MINUS_ASSIGN) || match(parser, TOKEN_STAR_ASSIGN) ||
        match(parser, TOKEN_SLASH_ASSIGN) || match(parser, TOKEN_MOD_ASSIGN)) {
        TokenType op = parser->previous.type;

        if (expr->type == EXPR_IDENTIFIER) {
            char* name = aira_strdup(expr->as.identifier);
            expr_free(expr);
            ExprNode* val = expression(parser);
            match_semicolon(parser);
            StmtNode* s = stmt_assign(name, op, val, line, col);
            aira_free(name);
            return s;
        } else if (expr->type == EXPR_INDEX) {
            ExprNode* target = expr->as.index_access.target;
            ExprNode* index = expr->as.index_access.index;
            aira_free(expr);
            ExprNode* val = expression(parser);
            match_semicolon(parser);
            return stmt_index_assign(target, index, val, line, col);
        } else if (expr->type == EXPR_MEMBER) {
            ExprNode* target = expr->as.member_access.target;
            char* mem = aira_strdup(expr->as.member_access.member);
            expr_free(expr);
            ExprNode* val = expression(parser);
            match_semicolon(parser);
            StmtNode* s = stmt_member_assign(target, mem, val, line, col);
            aira_free(mem);
            return s;
        }
    }

    match_semicolon(parser);
    return stmt_expr(expr, line, col);
}

static StmtNode* statement(Parser* parser) {
    if (match(parser, TOKEN_LET)) return let_declaration(parser);
    if (match(parser, TOKEN_SAY)) return say_or_print_statement(parser, true);
    if (match(parser, TOKEN_PRINT)) return say_or_print_statement(parser, false);
    if (match(parser, TOKEN_IF)) return if_statement(parser);
    if (match(parser, TOKEN_WHILE)) return while_statement(parser);
    if (match(parser, TOKEN_FOR)) return for_statement(parser);
    if (match(parser, TOKEN_FN)) return fn_declaration(parser);
    if (match(parser, TOKEN_RETURN)) return return_statement(parser);
    if (match(parser, TOKEN_IMPORT)) return import_statement(parser);
    if (match(parser, TOKEN_BREAK)) {
        match_semicolon(parser);
        return stmt_break(parser->previous.line, parser->previous.col);
    }
    if (match(parser, TOKEN_CONTINUE)) {
        match_semicolon(parser);
        return stmt_continue(parser->previous.line, parser->previous.col);
    }
    if (check(parser, TOKEN_LBRACE)) return block(parser);

    /* Skip @ directives like @airaai */
    if (match(parser, TOKEN_AT)) {
        advance(parser); /* consume directive name */
        if (check(parser, TOKEN_STRING)) advance(parser);
        match_semicolon(parser);
        return stmt_expr(expr_literal(val_null(), 0, 0), 0, 0);
    }

    return expression_statement(parser);
}

void parser_init(Parser* parser, const char* source, const char* filename) {
    lexer_init(&parser->lexer, source, filename);
    parser->filename = filename;
    parser->had_error = false;
    parser->panic_mode = false;
    advance(parser);
}

StmtNode* parser_parse(Parser* parser) {
    int cap = 16;
    int count = 0;
    StmtNode** statements = (StmtNode**)aira_malloc(sizeof(StmtNode*) * cap);

    while (!check(parser, TOKEN_EOF)) {
        if (count >= cap) {
            cap *= 2;
            statements = (StmtNode**)aira_realloc(statements, sizeof(StmtNode*) * cap);
        }
        statements[count++] = statement(parser);
    }

    return stmt_program(statements, count, 1, 1);
}
