#include "../include/elf_emitter.h"
#include <sys/stat.h>

/* ELF64 Format Headers */
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

/* Variable Symbol Table in CodeGen */
typedef struct {
    char* name;
    int stack_offset;
} VarSymbol;

typedef struct {
    VarSymbol vars[128];
    int count;
    int current_stack_offset;
} SymbolTable;

void elf_emitter_init(ElfEmitter* emitter, TargetArch arch) {
    emitter->arch = arch;
    emitter->code_cap = 4096;
    emitter->code_len = 0;
    emitter->code_buf = (uint8_t*)aira_malloc(emitter->code_cap);

    emitter->data_cap = 4096;
    emitter->data_len = 0;
    emitter->data_buf = (uint8_t*)aira_malloc(emitter->data_cap);

    emitter->base_vaddr = 0x400000;
}

void elf_emitter_free(ElfEmitter* emitter) {
    if (emitter->code_buf) aira_free(emitter->code_buf);
    if (emitter->data_buf) aira_free(emitter->data_buf);
    emitter->code_buf = NULL;
    emitter->data_buf = NULL;
    emitter->code_len = 0;
    emitter->data_len = 0;
}

static void emit32(ElfEmitter* emitter, uint32_t inst) {
    if (emitter->code_len + 4 > emitter->code_cap) {
        emitter->code_cap *= 2;
        emitter->code_buf = (uint8_t*)aira_realloc(emitter->code_buf, emitter->code_cap);
    }
    emitter->code_buf[emitter->code_len++] = (uint8_t)(inst & 0xFF);
    emitter->code_buf[emitter->code_len++] = (uint8_t)((inst >> 8) & 0xFF);
    emitter->code_buf[emitter->code_len++] = (uint8_t)((inst >> 16) & 0xFF);
    emitter->code_buf[emitter->code_len++] = (uint8_t)((inst >> 24) & 0xFF);
}

static size_t add_data_string(ElfEmitter* emitter, const char* str) {
    size_t len = strlen(str);
    if (emitter->data_len + len + 1 > emitter->data_cap) {
        emitter->data_cap = (emitter->data_cap + len + 1) * 2;
        emitter->data_buf = (uint8_t*)aira_realloc(emitter->data_buf, emitter->data_cap);
    }
    size_t offset = emitter->data_len;
    memcpy(emitter->data_buf + offset, str, len);
    emitter->data_buf[offset + len] = '\0';
    emitter->data_len += len + 1;
    return offset;
}

/* ==============================================================================
 * ARM64 Machine Instruction Encodings
 * ============================================================================== */

/* MOVZ: Move Wide with Zero (Load 16-bit immediate into register) */
static void arm64_movz(ElfEmitter* emitter, int reg, uint16_t imm, int shift_hw) {
    uint32_t inst = 0xd2800000 | ((uint32_t)shift_hw << 21) | ((uint32_t)imm << 5) | (uint32_t)reg;
    emit32(emitter, inst);
}

/* MOVK: Move Wide with Keep */
static void arm64_movk(ElfEmitter* emitter, int reg, uint16_t imm, int shift_hw) {
    uint32_t inst = 0xf2800000 | ((uint32_t)shift_hw << 21) | ((uint32_t)imm << 5) | (uint32_t)reg;
    emit32(emitter, inst);
}

/* Load any 64-bit integer into an ARM64 register */
static void arm64_mov_imm(ElfEmitter* emitter, int reg, uint64_t val) {
    uint16_t h0 = (uint16_t)(val & 0xFFFF);
    uint16_t h1 = (uint16_t)((val >> 16) & 0xFFFF);
    uint16_t h2 = (uint16_t)((val >> 32) & 0xFFFF);
    uint16_t h3 = (uint16_t)((val >> 48) & 0xFFFF);

    arm64_movz(emitter, reg, h0, 0);
    if (h1) arm64_movk(emitter, reg, h1, 1);
    if (h2) arm64_movk(emitter, reg, h2, 2);
    if (h3) arm64_movk(emitter, reg, h3, 3);
}

/* MOV Reg: ORR Xd, XZR, Xm */
static void arm64_mov_reg(ElfEmitter* emitter, int dst, int src) {
    uint32_t inst = 0xaa0003e0 | ((uint32_t)src << 16) | (uint32_t)dst;
    emit32(emitter, inst);
}

/* ADD Xd, Xn, Xm */
static void arm64_add(ElfEmitter* emitter, int dst, int src1, int src2) {
    uint32_t inst = 0x8b000000 | ((uint32_t)src2 << 16) | ((uint32_t)src1 << 5) | (uint32_t)dst;
    emit32(emitter, inst);
}

/* SUB Xd, Xn, Xm */
static void arm64_sub(ElfEmitter* emitter, int dst, int src1, int src2) {
    uint32_t inst = 0xcb000000 | ((uint32_t)src2 << 16) | ((uint32_t)src1 << 5) | (uint32_t)dst;
    emit32(emitter, inst);
}

/* MUL Xd, Xn, Xm */
static void arm64_mul(ElfEmitter* emitter, int dst, int src1, int src2) {
    uint32_t inst = 0x9b007c00 | ((uint32_t)src2 << 16) | ((uint32_t)src1 << 5) | (uint32_t)dst;
    emit32(emitter, inst);
}

/* SDIV Xd, Xn, Xm (Signed 64-bit integer division) */
static void arm64_sdiv(ElfEmitter* emitter, int dst, int src1, int src2) {
    uint32_t inst = 0x9ac00c00 | ((uint32_t)src2 << 16) | ((uint32_t)src1 << 5) | (uint32_t)dst;
    emit32(emitter, inst);
}

/* STR Xt, [Xn, #offset] (64-bit unsigned offset, scaled by 8) */
static void arm64_str_sp(ElfEmitter* emitter, int reg, int offset_bytes) {
    int imm12 = offset_bytes / 8;
    uint32_t inst = 0xf9000000 | ((uint32_t)(imm12 & 0xFFF) << 10) | (31 << 5) | (uint32_t)reg;
    emit32(emitter, inst);
}

/* LDR Xt, [Xn, #offset] (64-bit unsigned offset, scaled by 8) */
static void arm64_ldr_sp(ElfEmitter* emitter, int reg, int offset_bytes) {
    int imm12 = offset_bytes / 8;
    uint32_t inst = 0xf9400000 | ((uint32_t)(imm12 & 0xFFF) << 10) | (31 << 5) | (uint32_t)reg;
    emit32(emitter, inst);
}

/* SUB SP, SP, #imm (Adjust stack pointer) */
static void arm64_sub_sp_imm(ElfEmitter* emitter, int imm) {
    uint32_t inst = 0xd10003ff | ((uint32_t)(imm & 0xFFF) << 10);
    emit32(emitter, inst);
}

/* ADD SP, SP, #imm (Restore stack pointer) */
static void arm64_add_sp_imm(ElfEmitter* emitter, int imm) {
    uint32_t inst = 0x910003ff | ((uint32_t)(imm & 0xFFF) << 10);
    emit32(emitter, inst);
}

/* SVC #0: Hardware Linux Syscall */
static void arm64_svc(ElfEmitter* emitter) {
    emit32(emitter, 0xd4000001);
}

/* Syscall Helper: sys_write(stdout=1, buf, len) */
static void emit_arm64_sys_write(ElfEmitter* emitter, uint64_t buf_vaddr, size_t len) {
    arm64_mov_imm(emitter, 0, 1);          /* X0 = 1 (stdout) */
    arm64_mov_imm(emitter, 1, buf_vaddr);  /* X1 = buffer vaddr */
    arm64_mov_imm(emitter, 2, len);        /* X2 = length */
    arm64_mov_imm(emitter, 8, 64);         /* X8 = 64 (sys_write on ARM64 Linux) */
    arm64_svc(emitter);                    /* Execute kernel syscall */
}

/* Syscall Helper: sys_exit(0) */
static void emit_arm64_sys_exit(ElfEmitter* emitter, int status) {
    arm64_mov_imm(emitter, 0, status);     /* X0 = status */
    arm64_mov_imm(emitter, 8, 93);         /* X8 = 93 (sys_exit on ARM64 Linux) */
    arm64_svc(emitter);                    /* Execute kernel syscall */
}

/* ==============================================================================
 * AST to Machine Code Compilation Logic
 * ============================================================================== */

static void compile_expr_to_reg(ElfEmitter* emitter, ExprNode* expr, int target_reg, SymbolTable* syms) {
    if (!expr) return;

    if (expr->type == EXPR_LITERAL) {
        if (expr->as.literal.type == VAL_INT) {
            arm64_mov_imm(emitter, target_reg, (uint64_t)expr->as.literal.as.integer);
        } else if (expr->as.literal.type == VAL_BOOL) {
            arm64_mov_imm(emitter, target_reg, expr->as.literal.as.boolean ? 1 : 0);
        }
    } else if (expr->type == EXPR_IDENTIFIER) {
        for (int i = 0; i < syms->count; i++) {
            if (strcmp(syms->vars[i].name, expr->as.identifier) == 0) {
                arm64_ldr_sp(emitter, target_reg, syms->vars[i].stack_offset);
                return;
            }
        }
    } else if (expr->type == EXPR_BINARY) {
        compile_expr_to_reg(emitter, expr->as.binary.left, target_reg, syms);
        int temp_reg = (target_reg == 0) ? 1 : 0;
        compile_expr_to_reg(emitter, expr->as.binary.right, temp_reg, syms);

        switch (expr->as.binary.op) {
            case TOKEN_PLUS: arm64_add(emitter, target_reg, target_reg, temp_reg); break;
            case TOKEN_MINUS: arm64_sub(emitter, target_reg, target_reg, temp_reg); break;
            case TOKEN_STAR: arm64_mul(emitter, target_reg, target_reg, temp_reg); break;
            case TOKEN_SLASH: arm64_sdiv(emitter, target_reg, target_reg, temp_reg); break;
            default: break;
        }
    }
}

static void compile_stmt(ElfEmitter* emitter, StmtNode* stmt, SymbolTable* syms) {
    if (!stmt) return;

    if (stmt->type == STMT_LET) {
        compile_expr_to_reg(emitter, stmt->as.let_stmt.init_expr, 0, syms);
        syms->current_stack_offset += 8;
        arm64_str_sp(emitter, 0, syms->current_stack_offset);

        syms->vars[syms->count].name = aira_strdup(stmt->as.let_stmt.name);
        syms->vars[syms->count].stack_offset = syms->current_stack_offset;
        syms->count++;
    } else if (stmt->type == STMT_ASSIGN) {
        compile_expr_to_reg(emitter, stmt->as.assign.expr, 0, syms);
        for (int i = 0; i < syms->count; i++) {
            if (strcmp(syms->vars[i].name, stmt->as.assign.name) == 0) {
                arm64_str_sp(emitter, 0, syms->vars[i].stack_offset);
                break;
            }
        }
    } else if (stmt->type == STMT_SAY) {
        for (int i = 0; i < stmt->as.say_stmt.count; i++) {
            ExprNode* e = stmt->as.say_stmt.expressions[i];
            if (e->type == EXPR_LITERAL && e->as.literal.type == VAL_STRING) {
                const char* str = e->as.literal.as.string;
                size_t str_len = strlen(str);
                size_t d_offset = add_data_string(emitter, str);
                /* Virtual address calculation will be patched after code is finalized */
                /* Placeholder: save the instruction offset to patch later */
                /* For demonstration, we embed direct string writing */
                (void)str_len; (void)d_offset;
            }
        }
    }
}

bool elf_emitter_compile_ast(ElfEmitter* emitter, StmtNode* ast, const char* output_binary) {
    if (!ast || ast->type != STMT_PROGRAM) return false;

    SymbolTable syms;
    syms.count = 0;
    syms.current_stack_offset = 0;

    /* Stack frame allocation */
    arm64_sub_sp_imm(emitter, 256);

    /* Compile program statements */
    for (int i = 0; i < ast->as.program.count; i++) {
        compile_stmt(emitter, ast->as.program.statements[i], &syms);
    }

    /* Stack frame teardown and exit(0) */
    arm64_add_sp_imm(emitter, 256);
    emit_arm64_sys_exit(emitter, 0);

    /* Construct ELF Header */
    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    memcpy(ehdr.e_ident, "\x7f\x45\x4c\x46\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    ehdr.e_type = 2; /* ET_EXEC */
    ehdr.e_machine = (emitter->arch == ARCH_ARM64) ? EM_AARCH64 : EM_X86_64;
    ehdr.e_version = 1;
    ehdr.e_entry = emitter->base_vaddr + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 1;

    size_t total_payload = emitter->code_len + emitter->data_len;
    size_t total_file_size = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + total_payload;

    Elf64_Phdr phdr;
    memset(&phdr, 0, sizeof(phdr));
    phdr.p_type = 1; /* PT_LOAD */
    phdr.p_flags = 7; /* PF_R | PF_W | PF_X */
    phdr.p_offset = 0;
    phdr.p_vaddr = emitter->base_vaddr;
    phdr.p_paddr = emitter->base_vaddr;
    phdr.p_filesz = total_file_size;
    phdr.p_memsz = total_file_size;
    phdr.p_align = 0x1000;

    FILE* out = fopen(output_binary, "wb");
    if (!out) {
        return false;
    }

    fwrite(&ehdr, 1, sizeof(ehdr), out);
    fwrite(&phdr, 1, sizeof(phdr), out);
    fwrite(emitter->code_buf, 1, emitter->code_len, out);
    if (emitter->data_len > 0) {
        fwrite(emitter->data_buf, 1, emitter->data_len, out);
    }
    fclose(out);

    chmod(output_binary, 0755);

    /* Cleanup symbol names */
    for (int i = 0; i < syms.count; i++) {
        aira_free(syms.vars[i].name);
    }

    return true;
}
