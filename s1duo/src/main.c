#include "../include/common.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/evaluator.h"
#include "../include/elf_emitter.h"

static void print_banner(void) {
    printf(COLOR_CYAN
"     ___   _           __                    _____ __   ____             \n"
"    /   | (_)________ _/ /   ____ _____  ____ _ / ___// /  / __ \\__  ______  \n"
"   / /| |/ / ___/ __ `/ /   / __ `/ __ \\/ __ `// __// /  / / / / / / / __ \\ \n"
"  / ___ / / /  / /_/ / /___/ /_/ / / / / /_/ /___/ / /__/ /_/ / /_/ / /_/ / \n"
" /_/  |/_/_/   \\__,_/_____/\\__,_/_/ /_/\\__, //____/____/_____/\\__,_/\\____/  \n"
"                                      /____/                                \n"
COLOR_RESET);
    printf(COLOR_DIM "----------------------------------------------------------------------\n" COLOR_RESET);
    printf(" " COLOR_GREEN "[+]" COLOR_RESET " " COLOR_BOLD "Engine:" COLOR_RESET " %s v%s\n", AIRA_CODENAME, AIRA_VERSION);
    printf(" " COLOR_BLUE "[*]" COLOR_RESET " " COLOR_BOLD "Architect:" COLOR_RESET " %s\n", AIRA_ARCHITECT);
    printf(" " COLOR_YELLOW "[🛡️]" COLOR_RESET " " COLOR_BOLD "Security:" COLOR_RESET " Hardened Stack, Anti-Tamper Canaries (0x%08X)\n", AIRA_CANARY_VALUE);
    printf(" " COLOR_MAGENTA "[⚡]" COLOR_RESET " " COLOR_BOLD "Runtime:" COLOR_RESET " Pure C Native Binary Execution (Zero Python Overhead)\n");
    printf(COLOR_DIM "----------------------------------------------------------------------\n" COLOR_RESET);
}

static char* read_file_source(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, COLOR_RED "[!] Error: Could not open source file '%s'.\n" COLOR_RESET, filepath);
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length < 0 || length > AIRA_MAX_STRING_LEN) {
        fclose(f);
        fprintf(stderr, COLOR_RED "[!] Security Violation: Source file exceeds maximum allowed threshold of %d bytes.\n" COLOR_RESET, AIRA_MAX_STRING_LEN);
        exit(EXIT_FAILURE);
    }

    char* buffer = (char*)aira_malloc(length + 1);
    size_t read_bytes = fread(buffer, 1, length, f);
    buffer[read_bytes] = '\0';
    fclose(f);
    return buffer;
}

static void run_source(const char* source, const char* filename, Evaluator* eval) {
    Parser parser;
    parser_init(&parser, source, filename);

    StmtNode* program = parser_parse(&parser);
    if (!parser.had_error && program) {
        eval_program(eval, program);
    }

    stmt_free(program);
}

static void run_file(const char* filepath) {
    char* source = read_file_source(filepath);
    Evaluator eval;
    evaluator_init(&eval);

    run_source(source, filepath, &eval);

    evaluator_cleanup(&eval);
    aira_free(source);
}

static void run_repl(void) {
    print_banner();
    printf(COLOR_CYAN "Type your AiraLang statements or 'exit'/'quit' to exit.\n\n" COLOR_RESET);

    Evaluator eval;
    evaluator_init(&eval);

    char line_buf[4096];
    for (;;) {
        printf(COLOR_GREEN "aira3> " COLOR_RESET);
        fflush(stdout);

        if (!fgets(line_buf, sizeof(line_buf), stdin)) {
            printf("\n");
            break;
        }

        size_t len = strlen(line_buf);
        if (len > 0 && line_buf[len - 1] == '\n') line_buf[--len] = '\0';
        if (len > 0 && line_buf[len - 1] == '\r') line_buf[--len] = '\0';

        if (strcmp(line_buf, "exit") == 0 || strcmp(line_buf, "quit") == 0) {
            printf(COLOR_YELLOW "Goodbye Boss! Sovereign AiraLang S1 Duo stopping.\n" COLOR_RESET);
            break;
        }

        if (len == 0) continue;

        run_source(line_buf, "<repl>", &eval);
    }

    evaluator_cleanup(&eval);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        run_repl();
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        print_banner();
        return 0;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_banner();
        printf("Usage:\n");
        printf("  airalang3 <script.aira>              Execute AiraLang script with S1 Duo native engine\n");
        printf("  airalang3 -b <script.aira> -o <bin>  Compile directly to raw standalone ELF machine binary (0%% C, 0%% Python)\n");
        printf("  airalang3 -c \"<code>\"                 Execute inline AiraLang code string\n");
        printf("  airalang3                            Start S1 Duo interactive REPL\n");
        printf("  airalang3 -v, --version              Show engine version & security metadata\n");
        return 0;
    }

    if (strcmp(argv[1], "--emit-elf") == 0 || strcmp(argv[1], "-b") == 0) {
        if (argc < 3) {
            fprintf(stderr, COLOR_RED "[!] Error: -b requires an input .aira file.\n" COLOR_RESET);
            return 1;
        }
        const char* input_file = argv[2];
        const char* output_file = "a.out";
        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                output_file = argv[i + 1];
                break;
            }
        }
        char* source = read_file_source(input_file);
        Parser parser;
        parser_init(&parser, source, input_file);
        StmtNode* ast = parser_parse(&parser);

        if (!parser.had_error && ast) {
            ElfEmitter emitter;
            elf_emitter_init(&emitter, ARCH_ARM64);
            if (elf_emitter_compile_ast(&emitter, ast, output_file)) {
                printf(COLOR_GREEN "[✓] Standalone Raw Machine-Code ELF Binary created:" COLOR_RESET " %s\n", output_file);
                printf(COLOR_CYAN "[*] Zero Dependencies (0%% C, 0%% Python, Static Bare-Metal ELF)\n" COLOR_RESET);
            } else {
                fprintf(stderr, COLOR_RED "[!] Failed to emit ELF binary.\n" COLOR_RESET);
            }
            elf_emitter_free(&emitter);
        }
        stmt_free(ast);
        aira_free(source);
        return 0;
    }

    if (strcmp(argv[1], "-c") == 0) {
        if (argc < 3) {
            fprintf(stderr, COLOR_RED "[!] Error: -c requires a code string argument.\n" COLOR_RESET);
            return 1;
        }
        Evaluator eval;
        evaluator_init(&eval);
        run_source(argv[2], "<cli-eval>", &eval);
        evaluator_cleanup(&eval);
        return 0;
    }

    /* Direct execution: airalang3 script.aira */
    run_file(argv[1]);
    return 0;
}
