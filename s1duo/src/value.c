#include "../include/value.h"
#include <inttypes.h>

AiraValue val_null(void) {
    AiraValue v;
    v.type = VAL_NULL;
    v.as.integer = 0;
    return v;
}

AiraValue val_bool(bool b) {
    AiraValue v;
    v.type = VAL_BOOL;
    v.as.boolean = b;
    return v;
}

AiraValue val_int(int64_t i) {
    AiraValue v;
    v.type = VAL_INT;
    v.as.integer = i;
    return v;
}

AiraValue val_float(double f) {
    AiraValue v;
    v.type = VAL_FLOAT;
    v.as.floating = f;
    return v;
}

AiraValue val_string(const char* s) {
    AiraValue v;
    v.type = VAL_STRING;
    v.as.string = aira_strdup(s ? s : "");
    return v;
}

AiraValue val_string_take(char* s) {
    AiraValue v;
    v.type = VAL_STRING;
    v.as.string = s ? s : aira_strdup("");
    return v;
}

AiraValue val_list(void) {
    AiraValue v;
    v.type = VAL_LIST;
    AiraList* list = (AiraList*)aira_malloc(sizeof(AiraList));
    list->count = 0;
    list->capacity = 8;
    list->elements = (AiraValue*)aira_malloc(sizeof(AiraValue) * list->capacity);
    v.as.list = list;
    return v;
}

AiraValue val_dict(void) {
    AiraValue v;
    v.type = VAL_DICT;
    AiraDict* dict = (AiraDict*)aira_malloc(sizeof(AiraDict));
    dict->count = 0;
    dict->capacity = 8;
    dict->keys = (char**)aira_malloc(sizeof(char*) * dict->capacity);
    dict->values = (AiraValue*)aira_malloc(sizeof(AiraValue) * dict->capacity);
    v.as.dict = dict;
    return v;
}

AiraValue val_builtin(AiraBuiltinFn fn) {
    AiraValue v;
    v.type = VAL_BUILTIN_FN;
    v.as.builtin = fn;
    return v;
}

AiraValue val_function(const char* name, struct StmtNode* node, struct Environment* closure) {
    AiraValue v;
    v.type = VAL_FN;
    AiraFunction* fn = (AiraFunction*)aira_malloc(sizeof(AiraFunction));
    fn->name = name ? aira_strdup(name) : NULL;
    fn->node = node;
    fn->closure = closure;
    v.as.fn = fn;
    return v;
}

AiraValue val_module(const char* name) {
    AiraValue v;
    v.type = VAL_MODULE;
    AiraModule* mod = (AiraModule*)aira_malloc(sizeof(AiraModule));
    mod->name = aira_strdup(name ? name : "module");
    mod->members = (AiraDict*)aira_malloc(sizeof(AiraDict));
    mod->members->count = 0;
    mod->members->capacity = 16;
    mod->members->keys = (char**)aira_malloc(sizeof(char*) * mod->members->capacity);
    mod->members->values = (AiraValue*)aira_malloc(sizeof(AiraValue) * mod->members->capacity);
    v.as.module = mod;
    return v;
}

bool val_is_truthy(AiraValue v) {
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.as.boolean;
        case VAL_INT: return v.as.integer != 0;
        case VAL_FLOAT: return v.as.floating != 0.0;
        case VAL_STRING: return v.as.string && v.as.string[0] != '\0';
        case VAL_LIST: return v.as.list && v.as.list->count > 0;
        case VAL_DICT: return v.as.dict && v.as.dict->count > 0;
        default: return true;
    }
}

bool val_equals(AiraValue a, AiraValue b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_NULL: return true;
        case VAL_BOOL: return a.as.boolean == b.as.boolean;
        case VAL_INT: return a.as.integer == b.as.integer;
        case VAL_FLOAT: return a.as.floating == b.as.floating;
        case VAL_STRING: return strcmp(a.as.string, b.as.string) == 0;
        case VAL_LIST: return a.as.list == b.as.list;
        case VAL_DICT: return a.as.dict == b.as.dict;
        case VAL_BUILTIN_FN: return a.as.builtin == b.as.builtin;
        case VAL_FN: return a.as.fn == b.as.fn;
        case VAL_MODULE: return a.as.module == b.as.module;
        default: return false;
    }
}

int val_compare(AiraValue a, AiraValue b) {
    if (a.type == VAL_INT && b.type == VAL_INT) {
        if (a.as.integer < b.as.integer) return -1;
        if (a.as.integer > b.as.integer) return 1;
        return 0;
    }
    if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        if (a.as.floating < b.as.floating) return -1;
        if (a.as.floating > b.as.floating) return 1;
        return 0;
    }
    if (a.type == VAL_INT && b.type == VAL_FLOAT) {
        double diff = (double)a.as.integer - b.as.floating;
        return (diff < 0) ? -1 : (diff > 0 ? 1 : 0);
    }
    if (a.type == VAL_FLOAT && b.type == VAL_INT) {
        double diff = a.as.floating - (double)b.as.integer;
        return (diff < 0) ? -1 : (diff > 0 ? 1 : 0);
    }
    if (a.type == VAL_STRING && b.type == VAL_STRING) {
        return strcmp(a.as.string, b.as.string);
    }
    return 0;
}

AiraValue val_add(AiraValue a, AiraValue b) {
    if (a.type == VAL_INT && b.type == VAL_INT) {
        return val_int(a.as.integer + b.as.integer);
    }
    if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        return val_float(a.as.floating + b.as.floating);
    }
    if (a.type == VAL_INT && b.type == VAL_FLOAT) {
        return val_float((double)a.as.integer + b.as.floating);
    }
    if (a.type == VAL_FLOAT && b.type == VAL_INT) {
        return val_float(a.as.floating + (double)b.as.integer);
    }
    if (a.type == VAL_STRING || b.type == VAL_STRING) {
        char* str_a = val_to_string(a);
        char* str_b = val_to_string(b);
        size_t len_a = strlen(str_a);
        size_t len_b = strlen(str_b);
        size_t total = len_a + len_b;
        if (total > AIRA_MAX_STRING_LEN) {
            aira_panic("String concatenation exceeds limit (%d bytes).", AIRA_MAX_STRING_LEN);
        }
        char* res = (char*)aira_malloc(total + 1);
        memcpy(res, str_a, len_a);
        memcpy(res + len_a, str_b, len_b);
        res[total] = '\0';
        aira_free(str_a);
        aira_free(str_b);
        return val_string_take(res);
    }
    aira_panic("Runtime TypeError: Invalid operands for '+' operator.");
    return val_null();
}

AiraValue val_sub(AiraValue a, AiraValue b) {
    if (a.type == VAL_INT && b.type == VAL_INT) return val_int(a.as.integer - b.as.integer);
    if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) return val_float(a.as.floating - b.as.floating);
    if (a.type == VAL_INT && b.type == VAL_FLOAT) return val_float((double)a.as.integer - b.as.floating);
    if (a.type == VAL_FLOAT && b.type == VAL_INT) return val_float(a.as.floating - (double)b.as.integer);
    aira_panic("Runtime TypeError: Invalid operands for '-' operator.");
    return val_null();
}

AiraValue val_mul(AiraValue a, AiraValue b) {
    if (a.type == VAL_INT && b.type == VAL_INT) return val_int(a.as.integer * b.as.integer);
    if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) return val_float(a.as.floating * b.as.floating);
    if (a.type == VAL_INT && b.type == VAL_FLOAT) return val_float((double)a.as.integer * b.as.floating);
    if (a.type == VAL_FLOAT && b.type == VAL_INT) return val_float(a.as.floating * (double)b.as.integer);
    aira_panic("Runtime TypeError: Invalid operands for '*' operator.");
    return val_null();
}

AiraValue val_div(AiraValue a, AiraValue b) {
    double denom = (b.type == VAL_INT) ? (double)b.as.integer : ((b.type == VAL_FLOAT) ? b.as.floating : 0.0);
    if (denom == 0.0) {
        aira_panic("Arithmetic Error: Division by zero.");
    }
    double num = (a.type == VAL_INT) ? (double)a.as.integer : ((a.type == VAL_FLOAT) ? a.as.floating : 0.0);
    return val_float(num / denom);
}

AiraValue val_mod(AiraValue a, AiraValue b) {
    if (a.type == VAL_INT && b.type == VAL_INT) {
        if (b.as.integer == 0) {
            aira_panic("Arithmetic Error: Modulo by zero.");
        }
        return val_int(a.as.integer % b.as.integer);
    }
    aira_panic("Runtime TypeError: Modulo operator requires integer operands.");
    return val_null();
}

char* val_to_string(AiraValue v) {
    char buf[128];
    switch (v.type) {
        case VAL_NULL: return aira_strdup("null");
        case VAL_BOOL: return aira_strdup(v.as.boolean ? "true" : "false");
        case VAL_INT:
            snprintf(buf, sizeof(buf), "%" PRId64, v.as.integer);
            return aira_strdup(buf);
        case VAL_FLOAT:
            snprintf(buf, sizeof(buf), "%g", v.as.floating);
            return aira_strdup(buf);
        case VAL_STRING: return aira_strdup(v.as.string);
        case VAL_LIST: {
            AiraStringBuilder sb;
            sb_init(&sb);
            sb_append_char(&sb, '[');
            for (int i = 0; i < v.as.list->count; i++) {
                if (i > 0) sb_append(&sb, ", ");
                char* elem_str = val_to_string(v.as.list->elements[i]);
                sb_append(&sb, elem_str);
                aira_free(elem_str);
            }
            sb_append_char(&sb, ']');
            return sb_finish(&sb);
        }
        case VAL_DICT: {
            AiraStringBuilder sb;
            sb_init(&sb);
            sb_append_char(&sb, '{');
            for (int i = 0; i < v.as.dict->count; i++) {
                if (i > 0) sb_append(&sb, ", ");
                sb_append(&sb, v.as.dict->keys[i]);
                sb_append(&sb, ": ");
                char* val_str = val_to_string(v.as.dict->values[i]);
                sb_append(&sb, val_str);
                aira_free(val_str);
            }
            sb_append_char(&sb, '}');
            return sb_finish(&sb);
        }
        case VAL_BUILTIN_FN: return aira_strdup("<builtin function>");
        case VAL_FN: {
            snprintf(buf, sizeof(buf), "<function %s>", v.as.fn->name ? v.as.fn->name : "anonymous");
            return aira_strdup(buf);
        }
        case VAL_MODULE: {
            snprintf(buf, sizeof(buf), "<module %s>", v.as.module->name);
            return aira_strdup(buf);
        }
        default: return aira_strdup("<unknown>");
    }
}

void val_print(AiraValue v) {
    char* s = val_to_string(v);
    printf("%s", s);
    aira_free(s);
}

AiraValue val_copy(AiraValue v) {
    if (v.type == VAL_STRING) {
        return val_string(v.as.string);
    }
    return v;
}

void val_free(AiraValue* v) {
    if (!v) return;
    if (v->type == VAL_STRING && v->as.string) {
        aira_free(v->as.string);
        v->as.string = NULL;
    }
    v->type = VAL_NULL;
}

void list_append(AiraList* list, AiraValue v) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->elements = (AiraValue*)aira_realloc(list->elements, sizeof(AiraValue) * list->capacity);
    }
    list->elements[list->count++] = v;
}

AiraValue list_get(AiraList* list, int index) {
    if (index < 0) {
        index = list->count + index;
    }
    if (index < 0 || index >= list->count) {
        aira_panic("IndexError: List index %d out of bounds (length %d).", index, list->count);
    }
    return list->elements[index];
}

void list_set(AiraList* list, int index, AiraValue v) {
    if (index < 0) {
        index = list->count + index;
    }
    if (index < 0 || index >= list->count) {
        aira_panic("IndexError: List assignment index %d out of range (length %d).", index, list->count);
    }
    list->elements[index] = v;
}

AiraValue list_pop(AiraList* list) {
    if (list->count == 0) {
        aira_panic("Runtime Error: Cannot pop from empty list.");
    }
    return list->elements[--list->count];
}

void dict_set(AiraDict* dict, const char* key, AiraValue v) {
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) {
            dict->values[i] = v;
            return;
        }
    }
    if (dict->count >= dict->capacity) {
        dict->capacity *= 2;
        dict->keys = (char**)aira_realloc(dict->keys, sizeof(char*) * dict->capacity);
        dict->values = (AiraValue*)aira_realloc(dict->values, sizeof(AiraValue) * dict->capacity);
    }
    dict->keys[dict->count] = aira_strdup(key);
    dict->values[dict->count] = v;
    dict->count++;
}

AiraValue dict_get(AiraDict* dict, const char* key) {
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) {
            return dict->values[i];
        }
    }
    return val_null();
}

bool dict_has(AiraDict* dict, const char* key) {
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) return true;
    }
    return false;
}
