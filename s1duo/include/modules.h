#ifndef AIRA_MODULES_H
#define AIRA_MODULES_H

#include "value.h"
#include "env.h"

void register_builtin_globals(Environment* env);
AiraValue load_builtin_module(const char* name);

/* Pure C SHA256 */
void aira_sha256(const uint8_t* data, size_t len, char output[65]);

#endif /* AIRA_MODULES_H */
