#include "../include/common.h"

void* aira_malloc(size_t size) {
    if (size == 0) return NULL;
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, COLOR_RED "[!] Critical Error: Out of memory allocating %zu bytes." COLOR_RESET "\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* aira_calloc(size_t num, size_t size) {
    if (num == 0 || size == 0) return NULL;
    void* ptr = calloc(num, size);
    if (!ptr) {
        fprintf(stderr, COLOR_RED "[!] Critical Error: Out of memory in calloc (%zu * %zu)." COLOR_RESET "\n", num, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* aira_realloc(void* ptr, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr) {
        fprintf(stderr, COLOR_RED "[!] Critical Error: Out of memory reallocating to %zu bytes." COLOR_RESET "\n", new_size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

void aira_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

char* aira_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (len > AIRA_MAX_STRING_LEN) {
        fprintf(stderr, COLOR_RED "[!] Error: String length %zu exceeds maximum permitted size (%d bytes)." COLOR_RESET "\n", len, AIRA_MAX_STRING_LEN);
        exit(EXIT_FAILURE);
    }
    char* copy = (char*)aira_malloc(len + 1);
    memcpy(copy, s, len + 1);
    return copy;
}

char* aira_strndup(const char* s, size_t n) {
    if (!s) return NULL;
    if (n > AIRA_MAX_STRING_LEN) {
        n = AIRA_MAX_STRING_LEN;
    }
    char* copy = (char*)aira_malloc(n + 1);
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}

void aira_panic(const char* fmt, ...) {
    fprintf(stderr, COLOR_RED "[!] Runtime Error: " COLOR_RESET);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void aira_error_at(const char* filename, int line, int col, const char* fmt, ...) {
    fprintf(stderr, COLOR_RED "[!] Error" COLOR_RESET " at %s:%d:%d: ", filename ? filename : "<input>", line, col);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void sb_init(AiraStringBuilder* sb) {
    sb->capacity = 64;
    sb->length = 0;
    sb->data = (char*)aira_malloc(sb->capacity);
    sb->data[0] = '\0';
}

void sb_append_len(AiraStringBuilder* sb, const char* str, size_t len) {
    if (!str || len == 0) return;
    if (sb->length + len + 1 > AIRA_MAX_STRING_LEN) {
        aira_panic("StringBuilder exceeded safe string threshold of %d bytes.", AIRA_MAX_STRING_LEN);
    }
    if (sb->length + len + 1 > sb->capacity) {
        while (sb->capacity < sb->length + len + 1) {
            sb->capacity *= 2;
        }
        sb->data = (char*)aira_realloc(sb->data, sb->capacity);
    }
    memcpy(sb->data + sb->length, str, len);
    sb->length += len;
    sb->data[sb->length] = '\0';
}

void sb_append(AiraStringBuilder* sb, const char* str) {
    if (str) {
        sb_append_len(sb, str, strlen(str));
    }
}

void sb_append_char(AiraStringBuilder* sb, char c) {
    char buf[1] = { c };
    sb_append_len(sb, buf, 1);
}

char* sb_finish(AiraStringBuilder* sb) {
    char* res = sb->data;
    sb->data = NULL;
    sb->length = 0;
    sb->capacity = 0;
    return res;
}

void sb_free(AiraStringBuilder* sb) {
    if (sb->data) {
        aira_free(sb->data);
        sb->data = NULL;
    }
    sb->length = 0;
    sb->capacity = 0;
}
