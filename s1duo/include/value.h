#ifndef AIRA_VALUE_H
#define AIRA_VALUE_H

#include "common.h"

typedef enum {
    VAL_NULL = 0,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_LIST,
    VAL_DICT,
    VAL_BUILTIN_FN,
    VAL_FN,
    VAL_MODULE
} ValueType;

struct AiraValue;
struct Environment;
struct StmtNode;

typedef struct AiraValue (*AiraBuiltinFn)(int arg_count, struct AiraValue* args);

typedef struct {
    struct AiraValue* elements;
    int count;
    int capacity;
} AiraList;

typedef struct {
    char** keys;
    struct AiraValue* values;
    int count;
    int capacity;
} AiraDict;

typedef struct {
    const char* name;
    struct StmtNode* node;
    struct Environment* closure;
} AiraFunction;

typedef struct {
    const char* name;
    AiraDict* members;
} AiraModule;

typedef struct AiraValue {
    ValueType type;
    union {
        bool boolean;
        int64_t integer;
        double floating;
        char* string;
        AiraList* list;
        AiraDict* dict;
        AiraBuiltinFn builtin;
        AiraFunction* fn;
        AiraModule* module;
    } as;
} AiraValue;

/* Value Constructors */
AiraValue val_null(void);
AiraValue val_bool(bool b);
AiraValue val_int(int64_t i);
AiraValue val_float(double f);
AiraValue val_string(const char* s);
AiraValue val_string_take(char* s);
AiraValue val_list(void);
AiraValue val_dict(void);
AiraValue val_builtin(AiraBuiltinFn fn);
AiraValue val_function(const char* name, struct StmtNode* node, struct Environment* closure);
AiraValue val_module(const char* name);

/* Value Operations */
bool val_is_truthy(AiraValue v);
bool val_equals(AiraValue a, AiraValue b);
int  val_compare(AiraValue a, AiraValue b);
AiraValue val_add(AiraValue a, AiraValue b);
AiraValue val_sub(AiraValue a, AiraValue b);
AiraValue val_mul(AiraValue a, AiraValue b);
AiraValue val_div(AiraValue a, AiraValue b);
AiraValue val_mod(AiraValue a, AiraValue b);

/* String representation & Print */
char* val_to_string(AiraValue v);
void  val_print(AiraValue v);
void  val_free(AiraValue* v);
AiraValue val_copy(AiraValue v);

/* List and Dict Helpers */
void      list_append(AiraList* list, AiraValue v);
AiraValue list_get(AiraList* list, int index);
void      list_set(AiraList* list, int index, AiraValue v);
AiraValue list_pop(AiraList* list);

void      dict_set(AiraDict* dict, const char* key, AiraValue v);
AiraValue dict_get(AiraDict* dict, const char* key);
bool      dict_has(AiraDict* dict, const char* key);

/* Fast no-op verify macro */
#define val_verify_security(v) ((void)0)

#endif /* AIRA_VALUE_H */
