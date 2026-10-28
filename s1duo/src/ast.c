#include "../include/ast.h"

ExprNode* expr_literal(AiraValue val, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_LITERAL;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.literal = val;
    return e;
}

ExprNode* expr_identifier(const char* name, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_IDENTIFIER;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.identifier = aira_strdup(name);
    return e;
}

ExprNode* expr_binary(TokenType op, ExprNode* left, ExprNode* right, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_BINARY;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.binary.op = op;
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

ExprNode* expr_unary(TokenType op, ExprNode* right, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_UNARY;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.unary.op = op;
    e->as.unary.right = right;
    return e;
}

ExprNode* expr_call(ExprNode* callee, ExprNode** args, int count, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_CALL;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.call.callee = callee;
    e->as.call.args = args;
    e->as.call.arg_count = count;
    return e;
}

ExprNode* expr_index(ExprNode* target, ExprNode* index, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_INDEX;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.index_access.target = target;
    e->as.index_access.index = index;
    return e;
}

ExprNode* expr_member(ExprNode* target, const char* member, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_MEMBER;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.member_access.target = target;
    e->as.member_access.member = aira_strdup(member);
    return e;
}

ExprNode* expr_list(ExprNode** elements, int count, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_LIST;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.list.elements = elements;
    e->as.list.count = count;
    return e;
}

ExprNode* expr_dict(char** keys, ExprNode** values, int count, int line, int col) {
    ExprNode* e = (ExprNode*)aira_malloc(sizeof(ExprNode));
    e->type = EXPR_DICT;
    e->line = line;
    e->col = col;
    e->canary = AIRA_CANARY_VALUE;
    e->as.dict.keys = keys;
    e->as.dict.values = values;
    e->as.dict.count = count;
    return e;
}

StmtNode* stmt_program(StmtNode** statements, int count, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_PROGRAM;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.program.statements = statements;
    s->as.program.count = count;
    return s;
}

StmtNode* stmt_let(const char* name, ExprNode* init_expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_LET;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.let_stmt.name = aira_strdup(name);
    s->as.let_stmt.init_expr = init_expr;
    return s;
}

StmtNode* stmt_assign(const char* name, TokenType op, ExprNode* expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_ASSIGN;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.assign.name = aira_strdup(name);
    s->as.assign.op = op;
    s->as.assign.expr = expr;
    return s;
}

StmtNode* stmt_index_assign(ExprNode* target, ExprNode* index, ExprNode* expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_INDEX_ASSIGN;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.index_assign.target = target;
    s->as.index_assign.index = index;
    s->as.index_assign.expr = expr;
    return s;
}

StmtNode* stmt_member_assign(ExprNode* target, const char* member, ExprNode* expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_MEMBER_ASSIGN;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.member_assign.target = target;
    s->as.member_assign.member = aira_strdup(member);
    s->as.member_assign.expr = expr;
    return s;
}

StmtNode* stmt_say(ExprNode** expressions, int count, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_SAY;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.say_stmt.expressions = expressions;
    s->as.say_stmt.count = count;
    return s;
}

StmtNode* stmt_print(ExprNode** expressions, int count, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_PRINT;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.print_stmt.expressions = expressions;
    s->as.print_stmt.count = count;
    return s;
}

StmtNode* stmt_if(ExprNode* condition, StmtNode* then_b, StmtNode* else_b, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_IF;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.if_stmt.condition = condition;
    s->as.if_stmt.then_branch = then_b;
    s->as.if_stmt.else_branch = else_b;
    return s;
}

StmtNode* stmt_while(ExprNode* condition, StmtNode* body, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_WHILE;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.while_stmt.condition = condition;
    s->as.while_stmt.body = body;
    return s;
}

StmtNode* stmt_for_in(const char* var_name, ExprNode* iterable, StmtNode* body, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_FOR_IN;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.for_in_stmt.var_name = aira_strdup(var_name);
    s->as.for_in_stmt.iterable = iterable;
    s->as.for_in_stmt.body = body;
    return s;
}

StmtNode* stmt_fn_def(const char* name, char** params, int param_count, StmtNode* body, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_FN_DEF;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.fn_def.name = aira_strdup(name);
    s->as.fn_def.params = params;
    s->as.fn_def.param_count = param_count;
    s->as.fn_def.body = body;
    return s;
}

StmtNode* stmt_return(ExprNode* expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_RETURN;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.return_stmt.expr = expr;
    return s;
}

StmtNode* stmt_break(int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_BREAK;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    return s;
}

StmtNode* stmt_continue(int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_CONTINUE;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    return s;
}

StmtNode* stmt_block(StmtNode** statements, int count, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_BLOCK;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.block.statements = statements;
    s->as.block.count = count;
    return s;
}

StmtNode* stmt_expr(ExprNode* expr, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_EXPR;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.expr_stmt.expr = expr;
    return s;
}

StmtNode* stmt_import(const char* mod, const char* source, const char* alias, int line, int col) {
    StmtNode* s = (StmtNode*)aira_malloc(sizeof(StmtNode));
    s->type = STMT_IMPORT;
    s->line = line;
    s->col = col;
    s->canary = AIRA_CANARY_VALUE;
    s->as.import_stmt.module_name = aira_strdup(mod);
    s->as.import_stmt.source = source ? aira_strdup(source) : NULL;
    s->as.import_stmt.alias = alias ? aira_strdup(alias) : NULL;
    return s;
}

void expr_free(ExprNode* expr) {
    if (!expr) return;
    switch (expr->type) {
        case EXPR_IDENTIFIER:
            aira_free(expr->as.identifier);
            break;
        case EXPR_BINARY:
            expr_free(expr->as.binary.left);
            expr_free(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            expr_free(expr->as.unary.right);
            break;
        case EXPR_CALL:
            expr_free(expr->as.call.callee);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                expr_free(expr->as.call.args[i]);
            }
            aira_free(expr->as.call.args);
            break;
        case EXPR_INDEX:
            expr_free(expr->as.index_access.target);
            expr_free(expr->as.index_access.index);
            break;
        case EXPR_MEMBER:
            expr_free(expr->as.member_access.target);
            aira_free(expr->as.member_access.member);
            break;
        case EXPR_LIST:
            for (int i = 0; i < expr->as.list.count; i++) {
                expr_free(expr->as.list.elements[i]);
            }
            aira_free(expr->as.list.elements);
            break;
        case EXPR_DICT:
            for (int i = 0; i < expr->as.dict.count; i++) {
                aira_free(expr->as.dict.keys[i]);
                expr_free(expr->as.dict.values[i]);
            }
            aira_free(expr->as.dict.keys);
            aira_free(expr->as.dict.values);
            break;
        default:
            break;
    }
    aira_free(expr);
}

void stmt_free(StmtNode* stmt) {
    if (!stmt) return;
    switch (stmt->type) {
        case STMT_PROGRAM:
            for (int i = 0; i < stmt->as.program.count; i++) {
                stmt_free(stmt->as.program.statements[i]);
            }
            aira_free(stmt->as.program.statements);
            break;
        case STMT_LET:
            aira_free(stmt->as.let_stmt.name);
            expr_free(stmt->as.let_stmt.init_expr);
            break;
        case STMT_ASSIGN:
            aira_free(stmt->as.assign.name);
            expr_free(stmt->as.assign.expr);
            break;
        case STMT_INDEX_ASSIGN:
            expr_free(stmt->as.index_assign.target);
            expr_free(stmt->as.index_assign.index);
            expr_free(stmt->as.index_assign.expr);
            break;
        case STMT_MEMBER_ASSIGN:
            expr_free(stmt->as.member_assign.target);
            aira_free(stmt->as.member_assign.member);
            expr_free(stmt->as.member_assign.expr);
            break;
        case STMT_SAY:
            for (int i = 0; i < stmt->as.say_stmt.count; i++) {
                expr_free(stmt->as.say_stmt.expressions[i]);
            }
            aira_free(stmt->as.say_stmt.expressions);
            break;
        case STMT_PRINT:
            for (int i = 0; i < stmt->as.print_stmt.count; i++) {
                expr_free(stmt->as.print_stmt.expressions[i]);
            }
            aira_free(stmt->as.print_stmt.expressions);
            break;
        case STMT_IF:
            expr_free(stmt->as.if_stmt.condition);
            stmt_free(stmt->as.if_stmt.then_branch);
            stmt_free(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            expr_free(stmt->as.while_stmt.condition);
            stmt_free(stmt->as.while_stmt.body);
            break;
        case STMT_FOR_IN:
            aira_free(stmt->as.for_in_stmt.var_name);
            expr_free(stmt->as.for_in_stmt.iterable);
            stmt_free(stmt->as.for_in_stmt.body);
            break;
        case STMT_FN_DEF:
            aira_free(stmt->as.fn_def.name);
            for (int i = 0; i < stmt->as.fn_def.param_count; i++) {
                aira_free(stmt->as.fn_def.params[i]);
            }
            aira_free(stmt->as.fn_def.params);
            stmt_free(stmt->as.fn_def.body);
            break;
        case STMT_RETURN:
            expr_free(stmt->as.return_stmt.expr);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.count; i++) {
                stmt_free(stmt->as.block.statements[i]);
            }
            aira_free(stmt->as.block.statements);
            break;
        case STMT_EXPR:
            expr_free(stmt->as.expr_stmt.expr);
            break;
        case STMT_IMPORT:
            aira_free(stmt->as.import_stmt.module_name);
            if (stmt->as.import_stmt.source) aira_free(stmt->as.import_stmt.source);
            if (stmt->as.import_stmt.alias) aira_free(stmt->as.import_stmt.alias);
            break;
        default:
            break;
    }
    aira_free(stmt);
}
