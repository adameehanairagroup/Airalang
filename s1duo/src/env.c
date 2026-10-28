#include "../include/env.h"

Environment* env_new(Environment* parent) {
    Environment* env = (Environment*)aira_malloc(sizeof(Environment));
    env->parent = parent;
    env->canary = AIRA_CANARY_VALUE;
    env->count = 0;
    env->capacity = 8;
    env->names = (char**)aira_malloc(sizeof(char*) * env->capacity);
    env->values = (AiraValue*)aira_malloc(sizeof(AiraValue) * env->capacity);
    return env;
}

void env_free(Environment* env) {
    if (!env) return;
    if (env->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: Environment canary corrupted during cleanup.");
    }
    for (int i = 0; i < env->count; i++) {
        aira_free(env->names[i]);
    }
    aira_free(env->names);
    aira_free(env->values);
    env->canary = 0;
    aira_free(env);
}

void env_define(Environment* env, const char* name, AiraValue val) {
    if (env->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: Environment memory corrupted on define.");
    }
    /* Check if already in current scope */
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            env->values[i] = val;
            return;
        }
    }
    if (env->count >= env->capacity) {
        env->capacity *= 2;
        env->names = (char**)aira_realloc(env->names, sizeof(char*) * env->capacity);
        env->values = (AiraValue*)aira_realloc(env->values, sizeof(AiraValue) * env->capacity);
    }
    env->names[env->count] = aira_strdup(name);
    env->values[env->count] = val;
    env->count++;
}

bool env_get(Environment* env, const char* name, AiraValue* out_val) {
    Environment* curr = env;
    while (curr) {
        if (curr->canary != AIRA_CANARY_VALUE) {
            aira_panic("Security Violation: Environment pointer corrupted during variable lookup.");
        }
        for (int i = 0; i < curr->count; i++) {
            if (strcmp(curr->names[i], name) == 0) {
                if (out_val) *out_val = curr->values[i];
                return true;
            }
        }
        curr = curr->parent;
    }
    return false;
}

bool env_assign(Environment* env, const char* name, AiraValue val) {
    Environment* curr = env;
    while (curr) {
        if (curr->canary != AIRA_CANARY_VALUE) {
            aira_panic("Security Violation: Environment pointer corrupted during variable assignment.");
        }
        for (int i = 0; i < curr->count; i++) {
            if (strcmp(curr->names[i], name) == 0) {
                curr->values[i] = val;
                return true;
            }
        }
        curr = curr->parent;
    }
    return false;
}
