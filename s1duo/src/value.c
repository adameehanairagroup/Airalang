#include "../include/value.h"
#include <inttypes.h>

void val_verify_security(AiraValue v) {
    if (v.canary != AIRA_CANARY_VALUE) {
        aira_panic("Memory Security Canary Violation in AiraValue! Value corrupted or tampered (0x%08X != 0x%08X).", v.canary, AIRA_CANARY_VALUE);
    }
    if (v.type == VAL_LIST && v.as.list) {
        if (v.as.list->canary != AIRA_CANARY_VALUE) {
            aira_panic("AiraList Canary Check Failed! Heap buffer corruption detected.");
        }
    } else if (v.type == VAL_DICT && v.as.dict) {
        if (v.as.dict->canary != AIRA_CANARY_VALUE) {
            aira_panic("AiraDict Canary Check Failed! Heap buffer corruption detected.");
        }
    }
}

AiraValue val_null(void) {
    AiraValue v;
    v.type = VAL_NULL;
    v.canary = AIRA_CANARY_VALUE;
    v.as.integer = 0;
    return v;
}

AiraValue val_bool(bool b) {
    AiraValue v;
    v.type = VAL_BOOL;
    v.canary = AIRA_CANARY_VALUE;
    v.as.boolean = b;
    return v;
}

AiraValue val_int(int64_t i) {
    AiraValue v;
    v.type = VAL_INT;
    v.canary = AIRA_CANARY_VALUE;
    v.as.integer = i;
    return v;
}

AiraValue val_float(double f) {
    AiraValue v;
    v.type = VAL_FLOAT;
    v.canary = AIRA_CANARY_VALUE;
    v.as.floating = f;
    return v;
}

AiraValue val_string(const char* s) {
    AiraValue v;
    v.type = VAL_STRING;
    v.canary = AIRA_CANARY_VALUE;
    v.as.string = aira_strdup(s ? s : "");
    return v;
}

AiraValue val_string_take(char* s) {
    AiraValue v;
    v.type = VAL_STRING;
    v.canary = AIRA_CANARY_VALUE;
    v.as.string = s ? s : aira_strdup("");
    return v;
}

AiraValue val_list(void) {
    AiraValue v;
    v.type = VAL_LIST;
    v.canary = AIRA_CANARY_VALUE;
    AiraList* list = (AiraList*)aira_malloc(sizeof(AiraList));
    list->canary = AIRA_CANARY_VALUE;
    list->count = 0;
    list->capacity = 8;
    list->elements = (AiraValue*)aira_malloc(sizeof(AiraValue) * list->capacity);
    v.as.list = list;
    return v;
}

AiraValue val_dict(void) {
    AiraValue v;
    v.type = VAL_DICT;
    v.canary = AIRA_CANARY_VALUE;
    AiraDict* dict = (AiraDict*)aira_malloc(sizeof(AiraDict));
    dict->canary = AIRA_CANARY_VALUE;
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
    v.canary = AIRA_CANARY_VALUE;
    v.as.builtin = fn;
    return v;
}

AiraValue val_function(const char* name, struct StmtNode* node, struct Environment* closure) {
    AiraValue v;
    v.type = VAL_FN;
    v.canary = AIRA_CANARY_VALUE;
    AiraFunction* fn = (AiraFunction*)aira_malloc(sizeof(AiraFunction));
    fn->canary = AIRA_CANARY_VALUE;
    fn->name = name ? aira_strdup(name) : NULL;
    fn->node = node;
    fn->closure = closure;
    v.as.fn = fn;
    return v;
}

AiraValue val_module(const char* name) {
    AiraValue v;
    v.type = VAL_MODULE;
    v.canary = AIRA_CANARY_VALUE;
    AiraModule* mod = (AiraModule*)aira_malloc(sizeof(AiraModule));
    mod->canary = AIRA_CANARY_VALUE;
    mod->name = aira_strdup(name ? name : "module");
    mod->members = (AiraDict*)aira_malloc(sizeof(AiraDict));
    mod->members->canary = AIRA_CANARY_VALUE;
    mod->members->count = 0;
    mod->members->capacity = 16;
    mod->members->keys = (char**)aira_malloc(sizeof(char*) * mod->members->capacity);
    mod->members->values = (AiraValue*)aira_malloc(sizeof(AiraValue) * mod->members->capacity);
    v.as.module = mod;
    return v;
}

bool val_is_truthy(AiraValue v) {
    val_verify_security(v);
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.as.boolean;
        case VAL_INT: return v.as.integer != 0;
        case VAL_FLOAT: return v.as.floating != 0.0;
        case VAL_STRING: return v.as.string && strlen(v.as.string) > 0;
        case VAL_LIST: return v.as.list && v.as.list->count > 0;
        case VAL_DICT: return v.as.dict && v.as.dict->count > 0;
        default: return true;
    }
}

bool val_equals(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    if (a.type == VAL_INT && b.type == VAL_FLOAT) return (double)a.as.integer == b.as.floating;
    if (a.type == VAL_FLOAT && b.type == VAL_INT) return a.as.floating == (double)b.as.integer;
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NULL: return true;
        case VAL_BOOL: return a.as.boolean == b.as.boolean;
        case VAL_INT: return a.as.integer == b.as.integer;
        case VAL_FLOAT: return a.as.floating == b.as.floating;
        case VAL_STRING: return strcmp(a.as.string, b.as.string) == 0;
        default: return false;
    }
}

int val_compare(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    double num_a = 0, num_b = 0;
    if (a.type == VAL_INT) num_a = (double)a.as.integer;
    else if (a.type == VAL_FLOAT) num_a = a.as.floating;

    if (b.type == VAL_INT) num_b = (double)b.as.integer;
    else if (b.type == VAL_FLOAT) num_b = b.as.floating;

    if ((a.type == VAL_INT || a.type == VAL_FLOAT) && (b.type == VAL_INT || b.type == VAL_FLOAT)) {
        if (num_a < num_b) return -1;
        if (num_a > num_b) return 1;
        return 0;
    }

    if (a.type == VAL_STRING && b.type == VAL_STRING) {
        return strcmp(a.as.string, b.as.string);
    }

    return 0;
}

AiraValue val_add(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);

    /* String Concatenation */
    if (a.type == VAL_STRING || b.type == VAL_STRING) {
        char* sa = val_to_string(a);
        char* sb = val_to_string(b);
        size_t len_a = strlen(sa);
        size_t len_b = strlen(sb);
        if (len_a + len_b > AIRA_MAX_STRING_LEN) {
            aira_panic("String concatenation exceeds security limit (%d bytes).", AIRA_MAX_STRING_LEN);
        }
        char* res = (char*)aira_malloc(len_a + len_b + 1);
        memcpy(res, sa, len_a);
        memcpy(res + len_a, sb, len_b + 1);
        aira_free(sa);
        aira_free(sb);
        return val_string_take(res);
    }

    if (a.type == VAL_INT && b.type == VAL_INT) {
        return val_int(a.as.integer + b.as.integer);
    }

    double da = (a.type == VAL_INT) ? (double)a.as.integer : ((a.type == VAL_FLOAT) ? a.as.floating : 0.0);
    double db = (b.type == VAL_INT) ? (double)b.as.integer : ((b.type == VAL_FLOAT) ? b.as.floating : 0.0);
    return val_float(da + db);
}

AiraValue val_sub(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    if (a.type == VAL_INT && b.type == VAL_INT) {
        return val_int(a.as.integer - b.as.integer);
    }
    double da = (a.type == VAL_INT) ? (double)a.as.integer : ((a.type == VAL_FLOAT) ? a.as.floating : 0.0);
    double db = (b.type == VAL_INT) ? (double)b.as.integer : ((b.type == VAL_FLOAT) ? b.as.floating : 0.0);
    return val_float(da - db);
}

AiraValue val_mul(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    if (a.type == VAL_INT && b.type == VAL_INT) {
        return val_int(a.as.integer * b.as.integer);
    }
    double da = (a.type == VAL_INT) ? (double)a.as.integer : ((a.type == VAL_FLOAT) ? a.as.floating : 0.0);
    double db = (b.type == VAL_INT) ? (double)b.as.integer : ((b.type == VAL_FLOAT) ? b.as.floating : 0.0);
    return val_float(da * db);
}

AiraValue val_div(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    double db = (b.type == VAL_INT) ? (double)b.as.integer : ((b.type == VAL_FLOAT) ? b.as.floating : 0.0);
    if (db == 0.0) {
        aira_panic("Security & Arithmetic Fault: Division by zero attempted.");
    }
    if (a.type == VAL_INT && b.type == VAL_INT && (a.as.integer % b.as.integer == 0)) {
        return val_int(a.as.integer / b.as.integer);
    }
    double da = (a.type == VAL_INT) ? (double)a.as.integer : ((a.type == VAL_FLOAT) ? a.as.floating : 0.0);
    return val_float(da / db);
}

AiraValue val_mod(AiraValue a, AiraValue b) {
    val_verify_security(a);
    val_verify_security(b);
    if (b.type == VAL_INT && b.as.integer == 0) {
        aira_panic("Security & Arithmetic Fault: Modulo by zero attempted.");
    }
    if (a.type == VAL_INT && b.type == VAL_INT) {
        return val_int(a.as.integer % b.as.integer);
    }
    aira_panic("Modulo operator requires integer operands.");
    return val_null();
}

char* val_to_string(AiraValue v) {
    val_verify_security(v);
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
        case VAL_STRING: return aira_strdup(v.as.string ? v.as.string : "");
        case VAL_LIST: {
            AiraStringBuilder sb;
            sb_init(&sb);
            sb_append(&sb, "[");
            for (int i = 0; i < v.as.list->count; i++) {
                if (i > 0) sb_append(&sb, ", ");
                char* elem_s = val_to_string(v.as.list->elements[i]);
                sb_append(&sb, elem_s);
                aira_free(elem_s);
            }
            sb_append(&sb, "]");
            return sb_finish(&sb);
        }
        case VAL_DICT: {
            AiraStringBuilder sb;
            sb_init(&sb);
            sb_append(&sb, "{");
            for (int i = 0; i < v.as.dict->count; i++) {
                if (i > 0) sb_append(&sb, ", ");
                sb_append(&sb, "\"");
                sb_append(&sb, v.as.dict->keys[i]);
                sb_append(&sb, "\": ");
                char* val_s = val_to_string(v.as.dict->values[i]);
                sb_append(&sb, val_s);
                aira_free(val_s);
            }
            sb_append(&sb, "}");
            return sb_finish(&sb);
        }
        case VAL_BUILTIN_FN: return aira_strdup("<native fn>");
        case VAL_FN: {
            snprintf(buf, sizeof(buf), "<fn %s>", v.as.fn && v.as.fn->name ? v.as.fn->name : "anonymous");
            return aira_strdup(buf);
        }
        case VAL_MODULE: {
            snprintf(buf, sizeof(buf), "<module '%s'>", v.as.module && v.as.module->name ? v.as.module->name : "builtin");
            return aira_strdup(buf);
        }
        default: return aira_strdup("undefined");
    }
}

void val_print(AiraValue v) {
    char* s = val_to_string(v);
    printf("%s", s);
    aira_free(s);
}

AiraValue val_copy(AiraValue v) {
    val_verify_security(v);
    switch (v.type) {
        case VAL_STRING: return val_string(v.as.string);
        case VAL_LIST: {
            AiraValue new_list = val_list();
            for (int i = 0; i < v.as.list->count; i++) {
                list_append(new_list.as.list, val_copy(v.as.list->elements[i]));
            }
            return new_list;
        }
        case VAL_DICT: {
            AiraValue new_dict = val_dict();
            for (int i = 0; i < v.as.dict->count; i++) {
                dict_set(new_dict.as.dict, v.as.dict->keys[i], val_copy(v.as.dict->values[i]));
            }
            return new_dict;
        }
        default: return v;
    }
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
    if (list->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraList memory corrupted on append.");
    }
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->elements = (AiraValue*)aira_realloc(list->elements, sizeof(AiraValue) * list->capacity);
    }
    list->elements[list->count++] = v;
}

AiraValue list_get(AiraList* list, int index) {
    if (list->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraList memory corrupted on get.");
    }
    if (index < 0) {
        index = list->count + index;
    }
    /* STRICT BOUNDS CHECKING */
    if (index < 0 || index >= list->count) {
        aira_panic("Runtime Bounds Violation: List index %d out of bounds (length %d).", index, list->count);
    }
    return list->elements[index];
}

void list_set(AiraList* list, int index, AiraValue v) {
    if (list->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraList memory corrupted on set.");
    }
    if (index < 0) {
        index = list->count + index;
    }
    if (index < 0 || index >= list->count) {
        aira_panic("Runtime Bounds Violation: List assignment index %d out of range (length %d).", index, list->count);
    }
    list->elements[index] = v;
}

AiraValue list_pop(AiraList* list) {
    if (list->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraList memory corrupted on pop.");
    }
    if (list->count == 0) {
        aira_panic("Runtime Error: Cannot pop from empty list.");
    }
    return list->elements[--list->count];
}

void dict_set(AiraDict* dict, const char* key, AiraValue v) {
    if (dict->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraDict memory corrupted on set.");
    }
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
    if (dict->canary != AIRA_CANARY_VALUE) {
        aira_panic("Security Violation: AiraDict memory corrupted on get.");
    }
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) {
            return dict->values[i];
        }
    }
    return val_null();
}

bool dict_has(AiraDict* dict, const char* key) {
    if (dict->canary != AIRA_CANARY_VALUE) return false;
    for (int i = 0; i < dict->count; i++) {
        if (strcmp(dict->keys[i], key) == 0) return true;
    }
    return false;
}
