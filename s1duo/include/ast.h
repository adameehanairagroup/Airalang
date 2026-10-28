#ifndef AIRA_AST_H
#define AIRA_AST_H

#include "token.h"
#include "value.h"

typedef enum {
    STMT_PROGRAM,
    STMT_LET,
    STMT_ASSIGN,
    STMT_INDEX_ASSIGN,
    STMT_MEMBER_ASSIGN,
    STMT_SAY,
    STMT_PRINT,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR_IN,
    STMT_FN_DEF,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_BLOCK,
    STMT_EXPR,
    STMT_IMPORT
} StmtType;

typedef enum {
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_MEMBER,
    EXPR_LIST,
    EXPR_DICT
} ExprType;

typedef struct ExprNode ExprNode;
typedef struct StmtNode StmtNode;

struct ExprNode {
    ExprType type;
    int line;
    int col;
    union {
        AiraValue literal;
        char* identifier;
        struct {
            TokenType op;
            ExprNode* left;
            ExprNode* right;
        } binary;
        struct {
            TokenType op;
            ExprNode* right;
        } unary;
        struct {
            ExprNode* callee;
            ExprNode** args;
            int arg_count;
        } call;
        struct {
            ExprNode* target;
            ExprNode* index;
        } index_access;
        struct {
            ExprNode* target;
            char* member;
        } member_access;
        struct {
            ExprNode** elements;
            int count;
        } list;
        struct {
            char** keys;
            ExprNode** values;
            int count;
        } dict;
    } as;
};

struct StmtNode {
    StmtType type;
    int line;
    int col;
    union {
        struct {
            StmtNode** statements;
            int count;
        } program;
        struct {
            char* name;
            ExprNode* init_expr;
        } let_stmt;
        struct {
            char* name;
            TokenType op;
            ExprNode* expr;
        } assign;
        struct {
            ExprNode* target;
            ExprNode* index;
            ExprNode* expr;
        } index_assign;
        struct {
            ExprNode* target;
            char* member;
            ExprNode* expr;
        } member_assign;
        struct {
            ExprNode** expressions;
            int count;
        } say_stmt;
        struct {
            ExprNode** expressions;
            int count;
        } print_stmt;
        struct {
            ExprNode* condition;
            StmtNode* then_branch;
            StmtNode* else_branch;
        } if_stmt;
        struct {
            ExprNode* condition;
            StmtNode* body;
        } while_stmt;
        struct {
            char* var_name;
            ExprNode* iterable;
            StmtNode* body;
        } for_in_stmt;
        struct {
            char* name;
            char** params;
            int param_count;
            StmtNode* body;
        } fn_def;
        struct {
            ExprNode* expr;
        } return_stmt;
        struct {
            StmtNode** statements;
            int count;
        } block;
        struct {
            ExprNode* expr;
        } expr_stmt;
        struct {
            char* module_name;
            char* source;
            char* alias;
        } import_stmt;
    } as;
};

/* AST Constructors */
ExprNode* expr_literal(AiraValue val, int line, int col);
ExprNode* expr_identifier(const char* name, int line, int col);
ExprNode* expr_binary(TokenType op, ExprNode* left, ExprNode* right, int line, int col);
ExprNode* expr_unary(TokenType op, ExprNode* right, int line, int col);
ExprNode* expr_call(ExprNode* callee, ExprNode** args, int count, int line, int col);
ExprNode* expr_index(ExprNode* target, ExprNode* index, int line, int col);
ExprNode* expr_member(ExprNode* target, const char* member, int line, int col);
ExprNode* expr_list(ExprNode** elements, int count, int line, int col);
ExprNode* expr_dict(char** keys, ExprNode** values, int count, int line, int col);

StmtNode* stmt_program(StmtNode** statements, int count, int line, int col);
StmtNode* stmt_let(const char* name, ExprNode* init_expr, int line, int col);
StmtNode* stmt_assign(const char* name, TokenType op, ExprNode* expr, int line, int col);
StmtNode* stmt_index_assign(ExprNode* target, ExprNode* index, ExprNode* expr, int line, int col);
StmtNode* stmt_member_assign(ExprNode* target, const char* member, ExprNode* expr, int line, int col);
StmtNode* stmt_say(ExprNode** expressions, int count, int line, int col);
StmtNode* stmt_print(ExprNode** expressions, int count, int line, int col);
StmtNode* stmt_if(ExprNode* condition, StmtNode* then_b, StmtNode* else_b, int line, int col);
StmtNode* stmt_while(ExprNode* condition, StmtNode* body, int line, int col);
StmtNode* stmt_for_in(const char* var_name, ExprNode* iterable, StmtNode* body, int line, int col);
StmtNode* stmt_fn_def(const char* name, char** params, int param_count, StmtNode* body, int line, int col);
StmtNode* stmt_return(ExprNode* expr, int line, int col);
StmtNode* stmt_break(int line, int col);
StmtNode* stmt_continue(int line, int col);
StmtNode* stmt_block(StmtNode** statements, int count, int line, int col);
StmtNode* stmt_expr(ExprNode* expr, int line, int col);
StmtNode* stmt_import(const char* mod, const char* source, const char* alias, int line, int col);

void expr_free(ExprNode* expr);
void stmt_free(StmtNode* stmt);

#endif /* AIRA_AST_H */
