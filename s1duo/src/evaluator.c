#include "../include/evaluator.h"
#include "../include/modules.h"

void evaluator_init(Evaluator* eval) {
    eval->global_env = env_new(NULL);
    eval->call_depth = 0;
    eval->had_runtime_error = false;
    register_builtin_globals(eval->global_env);
}

void evaluator_cleanup(Evaluator* eval) {
    if (eval->global_env) {
        env_free(eval->global_env);
        eval->global_env = NULL;
    }
}

static EvalSignal signal_ok(void) {
    EvalSignal s;
    s.type = EXEC_OK;
    s.value = val_null();
    return s;
}

static EvalSignal signal_return(AiraValue v) {
    EvalSignal s;
    s.type = EXEC_RETURN;
    s.value = v;
    return s;
}

static EvalSignal signal_break(void) {
    EvalSignal s;
    s.type = EXEC_BREAK;
    s.value = val_null();
    return s;
}

static EvalSignal signal_continue(void) {
    EvalSignal s;
    s.type = EXEC_CONTINUE;
    s.value = val_null();
    return s;
}

AiraValue eval_expression(Evaluator* eval, Environment* env, ExprNode* expr) {
    if (!expr) return val_null();

    switch (expr->type) {
        case EXPR_LITERAL:
            return expr->as.literal;

        case EXPR_IDENTIFIER: {
            AiraValue val;
            if (env_get(env, expr->as.identifier, &val)) {
                return val;
            }
            aira_panic("Runtime Error: Undefined variable '%s' at line %d.", expr->as.identifier, expr->line);
            return val_null();
        }

        case EXPR_UNARY: {
            AiraValue right = eval_expression(eval, env, expr->as.unary.right);
            if (expr->as.unary.op == TOKEN_MINUS) {
                if (right.type == VAL_INT) return val_int(-right.as.integer);
                if (right.type == VAL_FLOAT) return val_float(-right.as.floating);
                aira_panic("Unary '-' requires numeric operand at line %d.", expr->line);
            }
            if (expr->as.unary.op == TOKEN_BANG || expr->as.unary.op == TOKEN_NOT) {
                return val_bool(!val_is_truthy(right));
            }
            return right;
        }

        case EXPR_BINARY: {
            /* Short-circuit logical operators */
            if (expr->as.binary.op == TOKEN_AND || expr->as.binary.op == TOKEN_AND_AND) {
                AiraValue left = eval_expression(eval, env, expr->as.binary.left);
                if (!val_is_truthy(left)) return left;
                return eval_expression(eval, env, expr->as.binary.right);
            }
            if (expr->as.binary.op == TOKEN_OR || expr->as.binary.op == TOKEN_OR_OR) {
                AiraValue left = eval_expression(eval, env, expr->as.binary.left);
                if (val_is_truthy(left)) return left;
                return eval_expression(eval, env, expr->as.binary.right);
            }

            AiraValue left = eval_expression(eval, env, expr->as.binary.left);
            AiraValue right = eval_expression(eval, env, expr->as.binary.right);

            switch (expr->as.binary.op) {
                case TOKEN_PLUS: return val_add(left, right);
                case TOKEN_MINUS: return val_sub(left, right);
                case TOKEN_STAR: return val_mul(left, right);
                case TOKEN_SLASH: return val_div(left, right);
                case TOKEN_PERCENT: return val_mod(left, right);
                case TOKEN_EQUAL_EQUAL: return val_bool(val_equals(left, right));
                case TOKEN_BANG_EQUAL: return val_bool(!val_equals(left, right));
                case TOKEN_LESS: return val_bool(val_compare(left, right) < 0);
                case TOKEN_LESS_EQUAL: return val_bool(val_compare(left, right) <= 0);
                case TOKEN_GREATER: return val_bool(val_compare(left, right) > 0);
                case TOKEN_GREATER_EQUAL: return val_bool(val_compare(left, right) >= 0);
                default:
                    aira_panic("Unsupported binary operator at line %d.", expr->line);
                    return val_null();
            }
        }

        case EXPR_LIST: {
            AiraValue list = val_list();
            for (int i = 0; i < expr->as.list.count; i++) {
                AiraValue el = eval_expression(eval, env, expr->as.list.elements[i]);
                list_append(list.as.list, el);
            }
            return list;
        }

        case EXPR_DICT: {
            AiraValue dict = val_dict();
            for (int i = 0; i < expr->as.dict.count; i++) {
                AiraValue val = eval_expression(eval, env, expr->as.dict.values[i]);
                dict_set(dict.as.dict, expr->as.dict.keys[i], val);
            }
            return dict;
        }

        case EXPR_INDEX: {
            AiraValue target = eval_expression(eval, env, expr->as.index_access.target);
            AiraValue idx = eval_expression(eval, env, expr->as.index_access.index);

            if (target.type == VAL_LIST) {
                int i = (idx.type == VAL_INT) ? (int)idx.as.integer : (int)idx.as.floating;
                return list_get(target.as.list, i);
            }
            if (target.type == VAL_DICT) {
                char* key = val_to_string(idx);
                AiraValue v = dict_get(target.as.dict, key);
                aira_free(key);
                return v;
            }
            if (target.type == VAL_STRING) {
                int i = (idx.type == VAL_INT) ? (int)idx.as.integer : (int)idx.as.floating;
                int len = (int)strlen(target.as.string);
                if (i < 0) i = len + i;
                if (i < 0 || i >= len) aira_panic("String index out of bounds (%d, length %d).", i, len);
                char ch[2] = { target.as.string[i], '\0' };
                return val_string(ch);
            }
            aira_panic("Index operator not supported on type at line %d.", expr->line);
            return val_null();
        }

        case EXPR_MEMBER: {
            AiraValue target = eval_expression(eval, env, expr->as.member_access.target);
            const char* mem = expr->as.member_access.member;

            if (target.type == VAL_MODULE) {
                AiraValue fn = dict_get(target.as.module->members, mem);
                if (fn.type != VAL_NULL) return fn;
                aira_panic("Module '%s' has no member '%s' at line %d.", target.as.module->name, mem, expr->line);
            }
            if (target.type == VAL_DICT) {
                return dict_get(target.as.dict, mem);
            }
            if (target.type == VAL_LIST) {
                if (strcmp(mem, "length") == 0) return val_int(target.as.list->count);
            }
            if (target.type == VAL_STRING) {
                if (strcmp(mem, "length") == 0) return val_int(strlen(target.as.string));
            }
            aira_panic("Member access '%s' not supported on target at line %d.", mem, expr->line);
            return val_null();
        }

        case EXPR_CALL: {
            /* Check if callee is a member method call like list.push(x) or str.upper() */
            if (expr->as.call.callee->type == EXPR_MEMBER) {
                ExprNode* target_expr = expr->as.call.callee->as.member_access.target;
                const char* method_name = expr->as.call.callee->as.member_access.member;
                AiraValue target = eval_expression(eval, env, target_expr);

                if (target.type == VAL_LIST) {
                    if (strcmp(method_name, "push") == 0 && expr->as.call.arg_count > 0) {
                        AiraValue arg = eval_expression(eval, env, expr->as.call.args[0]);
                        list_append(target.as.list, arg);
                        return val_null();
                    }
                    if (strcmp(method_name, "pop") == 0) {
                        return list_pop(target.as.list);
                    }
                    if (strcmp(method_name, "contains") == 0 && expr->as.call.arg_count > 0) {
                        AiraValue arg = eval_expression(eval, env, expr->as.call.args[0]);
                        for (int i = 0; i < target.as.list->count; i++) {
                            if (val_equals(target.as.list->elements[i], arg)) return val_bool(true);
                        }
                        return val_bool(false);
                    }
                }
                if (target.type == VAL_DICT) {
                    if (strcmp(method_name, "get") == 0 && expr->as.call.arg_count > 0) {
                        char* k = val_to_string(eval_expression(eval, env, expr->as.call.args[0]));
                        AiraValue res = dict_get(target.as.dict, k);
                        aira_free(k);
                        if (res.type == VAL_NULL && expr->as.call.arg_count > 1) {
                            return eval_expression(eval, env, expr->as.call.args[1]);
                        }
                        return res;
                    }
                    if (strcmp(method_name, "has") == 0 && expr->as.call.arg_count > 0) {
                        char* k = val_to_string(eval_expression(eval, env, expr->as.call.args[0]));
                        bool h = dict_has(target.as.dict, k);
                        aira_free(k);
                        return val_bool(h);
                    }
                }
                if (target.type == VAL_STRING) {
                    if (strcmp(method_name, "upper") == 0) {
                        char* s = aira_strdup(target.as.string);
                        for (size_t i = 0; s[i]; i++) s[i] = toupper((unsigned char)s[i]);
                        return val_string_take(s);
                    }
                    if (strcmp(method_name, "lower") == 0) {
                        char* s = aira_strdup(target.as.string);
                        for (size_t i = 0; s[i]; i++) s[i] = tolower((unsigned char)s[i]);
                        return val_string_take(s);
                    }
                }
                if (target.type == VAL_MODULE) {
                    AiraValue mod_fn = dict_get(target.as.module->members, method_name);
                    if (mod_fn.type == VAL_BUILTIN_FN) {
                        AiraValue args[16];
                        int count = expr->as.call.arg_count;
                        if (count > 16) count = 16;
                        for (int i = 0; i < count; i++) {
                            args[i] = eval_expression(eval, env, expr->as.call.args[i]);
                        }
                        return mod_fn.as.builtin(count, args);
                    }
                }
            }

            AiraValue callee = eval_expression(eval, env, expr->as.call.callee);

            if (callee.type == VAL_BUILTIN_FN) {
                AiraValue args[32];
                int count = expr->as.call.arg_count;
                if (count > 32) count = 32;
                for (int i = 0; i < count; i++) {
                    args[i] = eval_expression(eval, env, expr->as.call.args[i]);
                }
                return callee.as.builtin(count, args);
            }

            if (callee.type == VAL_FN) {
                AiraFunction* fn = callee.as.fn;
                Environment* local_env = env_new(fn->closure ? fn->closure : eval->global_env);

                int param_count = fn->node->as.fn_def.param_count;
                for (int i = 0; i < param_count; i++) {
                    AiraValue arg_val = (i < expr->as.call.arg_count) ? eval_expression(eval, env, expr->as.call.args[i]) : val_null();
                    env_define(local_env, fn->node->as.fn_def.params[i], arg_val);
                }

                eval->call_depth++;
                EvalSignal sig = eval_statement(eval, local_env, fn->node->as.fn_def.body);
                eval->call_depth--;
                env_free(local_env);

                if (sig.type == EXEC_RETURN) {
                    return sig.value;
                }
                return val_null();
            }

            aira_panic("Attempted to call a non-function value at line %d.", expr->line);
            return val_null();
        }

        default:
            return val_null();
    }
}

EvalSignal eval_statement(Evaluator* eval, Environment* env, StmtNode* stmt) {
    if (!stmt) return signal_ok();

    switch (stmt->type) {
        case STMT_LET: {
            AiraValue val = eval_expression(eval, env, stmt->as.let_stmt.init_expr);
            env_define(env, stmt->as.let_stmt.name, val);
            return signal_ok();
        }

        case STMT_ASSIGN: {
            AiraValue rhs = eval_expression(eval, env, stmt->as.assign.expr);
            const char* name = stmt->as.assign.name;
            TokenType op = stmt->as.assign.op;

            if (op == TOKEN_ASSIGN) {
                if (!env_assign(env, name, rhs)) {
                    aira_panic("Cannot assign to undeclared variable '%s' at line %d.", name, stmt->line);
                }
            } else {
                AiraValue curr;
                if (!env_get(env, name, &curr)) {
                    aira_panic("Variable '%s' not found for compound assignment at line %d.", name, stmt->line);
                }
                AiraValue new_val = val_null();
                if (op == TOKEN_PLUS_ASSIGN) new_val = val_add(curr, rhs);
                else if (op == TOKEN_MINUS_ASSIGN) new_val = val_sub(curr, rhs);
                else if (op == TOKEN_STAR_ASSIGN) new_val = val_mul(curr, rhs);
                else if (op == TOKEN_SLASH_ASSIGN) new_val = val_div(curr, rhs);
                else if (op == TOKEN_MOD_ASSIGN) new_val = val_mod(curr, rhs);

                env_assign(env, name, new_val);
            }
            return signal_ok();
        }

        case STMT_INDEX_ASSIGN: {
            AiraValue target = eval_expression(eval, env, stmt->as.index_assign.target);
            AiraValue idx = eval_expression(eval, env, stmt->as.index_assign.index);
            AiraValue rhs = eval_expression(eval, env, stmt->as.index_assign.expr);

            if (target.type == VAL_LIST) {
                int i = (idx.type == VAL_INT) ? (int)idx.as.integer : (int)idx.as.floating;
                list_set(target.as.list, i, rhs);
            } else if (target.type == VAL_DICT) {
                char* k = val_to_string(idx);
                dict_set(target.as.dict, k, rhs);
                aira_free(k);
            } else {
                aira_panic("Index assignment not supported on type at line %d.", stmt->line);
            }
            return signal_ok();
        }

        case STMT_MEMBER_ASSIGN: {
            AiraValue target = eval_expression(eval, env, stmt->as.member_assign.target);
            AiraValue rhs = eval_expression(eval, env, stmt->as.member_assign.expr);
            if (target.type == VAL_DICT) {
                dict_set(target.as.dict, stmt->as.member_assign.member, rhs);
            } else {
                aira_panic("Member assignment only valid on dictionaries at line %d.", stmt->line);
            }
            return signal_ok();
        }

        case STMT_SAY: {
            for (int i = 0; i < stmt->as.say_stmt.count; i++) {
                if (i > 0) printf(" ");
                AiraValue v = eval_expression(eval, env, stmt->as.say_stmt.expressions[i]);
                val_print(v);
            }
            printf("\n");
            fflush(stdout);
            return signal_ok();
        }

        case STMT_PRINT: {
            for (int i = 0; i < stmt->as.print_stmt.count; i++) {
                if (i > 0) printf(" ");
                AiraValue v = eval_expression(eval, env, stmt->as.print_stmt.expressions[i]);
                val_print(v);
            }
            fflush(stdout);
            return signal_ok();
        }

        case STMT_IF: {
            AiraValue cond = eval_expression(eval, env, stmt->as.if_stmt.condition);
            if (val_is_truthy(cond)) {
                return eval_statement(eval, env, stmt->as.if_stmt.then_branch);
            } else if (stmt->as.if_stmt.else_branch) {
                return eval_statement(eval, env, stmt->as.if_stmt.else_branch);
            }
            return signal_ok();
        }

        case STMT_WHILE: {
            while (val_is_truthy(eval_expression(eval, env, stmt->as.while_stmt.condition))) {
                EvalSignal sig = eval_statement(eval, env, stmt->as.while_stmt.body);
                if (sig.type == EXEC_BREAK) break;
                if (sig.type == EXEC_CONTINUE) continue;
                if (sig.type == EXEC_RETURN) return sig;
            }
            return signal_ok();
        }

        case STMT_FOR_IN: {
            AiraValue iter = eval_expression(eval, env, stmt->as.for_in_stmt.iterable);
            if (iter.type == VAL_LIST) {
                for (int i = 0; i < iter.as.list->count; i++) {
                    env_define(env, stmt->as.for_in_stmt.var_name, iter.as.list->elements[i]);
                    EvalSignal sig = eval_statement(eval, env, stmt->as.for_in_stmt.body);
                    if (sig.type == EXEC_BREAK) break;
                    if (sig.type == EXEC_CONTINUE) continue;
                    if (sig.type == EXEC_RETURN) return sig;
                }
            } else {
                aira_panic("'for..in' loop requires an iterable list at line %d.", stmt->line);
            }
            return signal_ok();
        }

        case STMT_FN_DEF: {
            AiraValue fn = val_function(stmt->as.fn_def.name, stmt, env);
            env_define(env, stmt->as.fn_def.name, fn);
            return signal_ok();
        }

        case STMT_RETURN: {
            AiraValue ret = stmt->as.return_stmt.expr ? eval_expression(eval, env, stmt->as.return_stmt.expr) : val_null();
            return signal_return(ret);
        }

        case STMT_BREAK:
            return signal_break();

        case STMT_CONTINUE:
            return signal_continue();

        case STMT_BLOCK: {
            Environment* block_env = env_new(env);
            for (int i = 0; i < stmt->as.block.count; i++) {
                EvalSignal sig = eval_statement(eval, block_env, stmt->as.block.statements[i]);
                if (sig.type != EXEC_OK) {
                    env_free(block_env);
                    return sig;
                }
            }
            env_free(block_env);
            return signal_ok();
        }

        case STMT_EXPR:
            eval_expression(eval, env, stmt->as.expr_stmt.expr);
            return signal_ok();

        case STMT_IMPORT: {
            AiraValue mod = load_builtin_module(stmt->as.import_stmt.module_name);
            if (mod.type != VAL_NULL) {
                const char* alias = stmt->as.import_stmt.alias ? stmt->as.import_stmt.alias : stmt->as.import_stmt.module_name;
                env_define(env, alias, mod);
            } else {
                aira_panic("Module '%s' could not be loaded at line %d.", stmt->as.import_stmt.module_name, stmt->line);
            }
            return signal_ok();
        }

        default:
            return signal_ok();
    }
}

void eval_program(Evaluator* eval, StmtNode* program) {
    if (!program || program->type != STMT_PROGRAM) return;
    for (int i = 0; i < program->as.program.count; i++) {
        EvalSignal sig = eval_statement(eval, eval->global_env, program->as.program.statements[i]);
        if (sig.type == EXEC_RETURN) break;
    }
}
