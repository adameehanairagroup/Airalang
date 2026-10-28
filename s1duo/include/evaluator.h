#ifndef AIRA_EVALUATOR_H
#define AIRA_EVALUATOR_H

#include "ast.h"
#include "env.h"

typedef enum {
    EXEC_OK = 0,
    EXEC_RETURN,
    EXEC_BREAK,
    EXEC_CONTINUE,
    EXEC_ERROR
} ExecResult;

typedef struct {
    ExecResult type;
    AiraValue value;
} EvalSignal;

typedef struct {
    Environment* global_env;
    int call_depth;
    bool had_runtime_error;
} Evaluator;

void evaluator_init(Evaluator* eval);
void evaluator_cleanup(Evaluator* eval);
AiraValue eval_expression(Evaluator* eval, Environment* env, ExprNode* expr);
EvalSignal eval_statement(Evaluator* eval, Environment* env, StmtNode* stmt);
void eval_program(Evaluator* eval, StmtNode* program);

#endif /* AIRA_EVALUATOR_H */
