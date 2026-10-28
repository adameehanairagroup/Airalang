#ifndef AIRA_ENV_H
#define AIRA_ENV_H

#include "value.h"

typedef struct Environment {
    struct Environment* parent;
    char** names;
    AiraValue* values;
    int count;
    int capacity;
} Environment;

Environment* env_new(Environment* parent);
void env_free(Environment* env);
void env_define(Environment* env, const char* name, AiraValue val);
bool env_get(Environment* env, const char* name, AiraValue* out_val);
bool env_assign(Environment* env, const char* name, AiraValue val);

#endif /* AIRA_ENV_H */
