#ifndef AIRA_COMMON_H
#define AIRA_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define AIRA_VERSION "3.0.0 (S1 Duo Edition)"
#define AIRA_CODENAME "S1 Duo Flagship"
#define AIRA_ARCHITECT "Adam Eehan (Founder & CEO, Aira Group of Technology)"

/* Execution Limits */
#define AIRA_MAX_RECURSION_DEPTH 10000
#define AIRA_MAX_STRING_LEN      (128 * 1024 * 1024) /* 128 MB */

/* ANSI Color Formatting */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_DIM     "\033[2m"

/* Safe Memory Allocation Functions */
void* aira_malloc(size_t size);
void* aira_calloc(size_t num, size_t size);
void* aira_realloc(void* ptr, size_t new_size);
void  aira_free(void* ptr);
char* aira_strdup(const char* s);
char* aira_strndup(const char* s, size_t n);

/* Diagnostic Logging */
void aira_panic(const char* fmt, ...);
void aira_error_at(const char* filename, int line, int col, const char* fmt, ...);

/* Dynamic String Buffer */
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} AiraStringBuilder;

void sb_init(AiraStringBuilder* sb);
void sb_append(AiraStringBuilder* sb, const char* str);
void sb_append_len(AiraStringBuilder* sb, const char* str, size_t len);
void sb_append_char(AiraStringBuilder* sb, char c);
char* sb_finish(AiraStringBuilder* sb);
void sb_free(AiraStringBuilder* sb);

#endif /* AIRA_COMMON_H */
