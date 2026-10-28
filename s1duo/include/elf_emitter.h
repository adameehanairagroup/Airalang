#ifndef AIRA_ELF_EMITTER_H
#define AIRA_ELF_EMITTER_H

#include "ast.h"

#define ELF_MAGIC "\x7f\x45\x4c\x46"
#define EM_AARCH64 0xB7
#define EM_X86_64  0x3E

typedef enum {
    ARCH_ARM64,
    ARCH_X86_64
} TargetArch;

typedef struct {
    TargetArch arch;
    uint8_t* code_buf;
    size_t code_len;
    size_t code_cap;

    uint8_t* data_buf;
    size_t data_len;
    size_t data_cap;

    uint64_t base_vaddr;
} ElfEmitter;

void elf_emitter_init(ElfEmitter* emitter, TargetArch arch);
void elf_emitter_free(ElfEmitter* emitter);

/* Binary File Generation */
bool elf_emitter_compile_ast(ElfEmitter* emitter, StmtNode* ast, const char* output_binary);

#endif /* AIRA_ELF_EMITTER_H */
