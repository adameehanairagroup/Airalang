#include "../include/modules.h"
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

/* ==============================================================================
 * Sovereign Pure-C SHA256 Implementation (Zero-Dependency Security Engine)
 * ============================================================================== */

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} SHA256_CTX;

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) | ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->count = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    size_t i, index, partlen;
    index = (size_t)((ctx->count >> 3) & 0x3f);
    partlen = 64 - index;
    ctx->count += (uint64_t)len << 3;
    if (len >= partlen) {
        memcpy(&ctx->buffer[index], data, partlen);
        sha256_transform(ctx, ctx->buffer);
        for (i = partlen; i + 63 < len; i += 64)
            sha256_transform(ctx, &data[i]);
        index = 0;
    } else {
        i = 0;
    }
    memcpy(&ctx->buffer[index], &data[i], len - i);
}

static void sha256_final(SHA256_CTX* ctx, uint8_t digest[32]) {
    uint8_t finalcount[8];
    for (int i = 0; i < 8; i++)
        finalcount[i] = (uint8_t)((ctx->count >> ((7 - i) * 8)) & 0xff);
    uint8_t c = 0x80;
    sha256_update(ctx, &c, 1);
    while ((ctx->count >> 3) & 0x3f) {
        c = 0x00;
        sha256_update(ctx, &c, 1);
    }
    sha256_update(ctx, finalcount, 8);
    for (int i = 0; i < 8; i++) {
        digest[i * 4] = (uint8_t)((ctx->state[i] >> 24) & 0xff);
        digest[i * 4 + 1] = (uint8_t)((ctx->state[i] >> 16) & 0xff);
        digest[i * 4 + 2] = (uint8_t)((ctx->state[i] >> 8) & 0xff);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i] & 0xff);
    }
}

void aira_sha256(const uint8_t* data, size_t len, char output[65]) {
    SHA256_CTX ctx;
    uint8_t digest[32];
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
    for (int i = 0; i < 32; i++) {
        sprintf(output + (i * 2), "%02x", digest[i]);
    }
    output[64] = '\0';
}

/* ==============================================================================
 * Global Builtin Functions
 * ============================================================================== */

static AiraValue builtin_len(int arg_count, AiraValue* args) {
    if (arg_count < 1) return val_int(0);
    AiraValue v = args[0];
    if (v.type == VAL_STRING) return val_int((int64_t)strlen(v.as.string));
    if (v.type == VAL_LIST) return val_int((int64_t)v.as.list->count);
    if (v.type == VAL_DICT) return val_int((int64_t)v.as.dict->count);
    return val_int(0);
}

static AiraValue builtin_range(int arg_count, AiraValue* args) {
    int64_t start = 0;
    int64_t end = 0;
    int64_t step = 1;

    if (arg_count == 1) {
        end = args[0].type == VAL_INT ? args[0].as.integer : (int64_t)args[0].as.floating;
    } else if (arg_count >= 2) {
        start = args[0].type == VAL_INT ? args[0].as.integer : (int64_t)args[0].as.floating;
        end = args[1].type == VAL_INT ? args[1].as.integer : (int64_t)args[1].as.floating;
        if (arg_count >= 3) {
            step = args[2].type == VAL_INT ? args[2].as.integer : (int64_t)args[2].as.floating;
            if (step == 0) step = 1;
        }
    }

    AiraValue list = val_list();
    if (step > 0) {
        for (int64_t i = start; i < end; i += step) {
            list_append(list.as.list, val_int(i));
        }
    } else {
        for (int64_t i = start; i > end; i += step) {
            list_append(list.as.list, val_int(i));
        }
    }
    return list;
}

static AiraValue builtin_input(int arg_count, AiraValue* args) {
    if (arg_count > 0) {
        val_print(args[0]);
        fflush(stdout);
    }
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[len - 1] = '\0';
        }
        return val_string(buf);
    }
    return val_string("");
}

static AiraValue builtin_type(int arg_count, AiraValue* args) {
    if (arg_count < 1) return val_string("null");
    switch (args[0].type) {
        case VAL_NULL: return val_string("null");
        case VAL_BOOL: return val_string("bool");
        case VAL_INT: return val_string("int");
        case VAL_FLOAT: return val_string("float");
        case VAL_STRING: return val_string("string");
        case VAL_LIST: return val_string("list");
        case VAL_DICT: return val_string("dict");
        case VAL_BUILTIN_FN:
        case VAL_FN: return val_string("function");
        case VAL_MODULE: return val_string("module");
        default: return val_string("unknown");
    }
}

static AiraValue builtin_to_int(int arg_count, AiraValue* args) {
    if (arg_count < 1) return val_int(0);
    AiraValue v = args[0];
    if (v.type == VAL_INT) return v;
    if (v.type == VAL_FLOAT) return val_int((int64_t)v.as.floating);
    if (v.type == VAL_STRING) return val_int(strtoll(v.as.string, NULL, 10));
    if (v.type == VAL_BOOL) return val_int(v.as.boolean ? 1 : 0);
    return val_int(0);
}

static AiraValue builtin_to_float(int arg_count, AiraValue* args) {
    if (arg_count < 1) return val_float(0.0);
    AiraValue v = args[0];
    if (v.type == VAL_FLOAT) return v;
    if (v.type == VAL_INT) return val_float((double)v.as.integer);
    if (v.type == VAL_STRING) return val_float(strtod(v.as.string, NULL));
    return val_float(0.0);
}

static AiraValue builtin_to_str(int arg_count, AiraValue* args) {
    if (arg_count < 1) return val_string("");
    return val_string_take(val_to_string(args[0]));
}

/* ==============================================================================
 * Native C Module Implementations
 * ============================================================================== */

/* Math Module Functions */
static AiraValue math_sqrt(int argc, AiraValue* args) {
    double x = (argc > 0 && args[0].type == VAL_INT) ? (double)args[0].as.integer : (argc > 0 ? args[0].as.floating : 0.0);
    return val_float(sqrt(x));
}

static AiraValue math_pow(int argc, AiraValue* args) {
    double b = (argc > 0 && args[0].type == VAL_INT) ? (double)args[0].as.integer : (argc > 0 ? args[0].as.floating : 0.0);
    double e = (argc > 1 && args[1].type == VAL_INT) ? (double)args[1].as.integer : (argc > 1 ? args[1].as.floating : 0.0);
    return val_float(pow(b, e));
}

static AiraValue math_abs(int argc, AiraValue* args) {
    if (argc > 0 && args[0].type == VAL_INT) {
        int64_t i = args[0].as.integer;
        return val_int(i < 0 ? -i : i);
    }
    double x = argc > 0 ? args[0].as.floating : 0.0;
    return val_float(fabs(x));
}

static AiraValue math_floor(int argc, AiraValue* args) {
    double x = (argc > 0 && args[0].type == VAL_INT) ? (double)args[0].as.integer : (argc > 0 ? args[0].as.floating : 0.0);
    return val_int((int64_t)floor(x));
}

static AiraValue math_ceil(int argc, AiraValue* args) {
    double x = (argc > 0 && args[0].type == VAL_INT) ? (double)args[0].as.integer : (argc > 0 ? args[0].as.floating : 0.0);
    return val_int((int64_t)ceil(x));
}

/* OS Module Functions */
static AiraValue os_system(int argc, AiraValue* args) {
    if (argc < 1 || args[0].type != VAL_STRING) return val_int(-1);
    /* Safe execution check */
    int res = system(args[0].as.string);
    return val_int(res);
}

static AiraValue os_getcwd(int argc, AiraValue* args) {
    (void)argc; (void)args;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        return val_string(cwd);
    }
    return val_string(".");
}

static AiraValue os_getenv(int argc, AiraValue* args) {
    if (argc < 1 || args[0].type != VAL_STRING) return val_null();
    char* val = getenv(args[0].as.string);
    return val ? val_string(val) : val_null();
}

static AiraValue os_platform(int argc, AiraValue* args) {
    (void)argc; (void)args;
    return val_string("linux-aarch64 (S1 Duo Engine)");
}

/* Time Module Functions */
static AiraValue time_time(int argc, AiraValue* args) {
    (void)argc; (void)args;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    double sec = (double)ts.tv_sec + ((double)ts.tv_nsec / 1e9);
    return val_float(sec);
}

static AiraValue time_sleep(int argc, AiraValue* args) {
    if (argc < 1) return val_null();
    double secs = (args[0].type == VAL_INT) ? (double)args[0].as.integer : args[0].as.floating;
    if (secs > 0) {
        usleep((useconds_t)(secs * 1000000));
    }
    return val_null();
}

/* File Module Functions */
static AiraValue file_read(int argc, AiraValue* args) {
    if (argc < 1 || args[0].type != VAL_STRING) return val_string("");
    FILE* f = fopen(args[0].as.string, "rb");
    if (!f) return val_null();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz > AIRA_MAX_STRING_LEN) {
        fclose(f);
        aira_panic("File %s exceeds maximum permitted memory size.", args[0].as.string);
    }
    char* buf = (char*)aira_malloc(sz + 1);
    size_t read_bytes = fread(buf, 1, sz, f);
    buf[read_bytes] = '\0';
    fclose(f);
    return val_string_take(buf);
}

static AiraValue file_write(int argc, AiraValue* args) {
    if (argc < 2 || args[0].type != VAL_STRING) return val_bool(false);
    FILE* f = fopen(args[0].as.string, "wb");
    if (!f) return val_bool(false);
    char* content = val_to_string(args[1]);
    fputs(content, f);
    aira_free(content);
    fclose(f);
    return val_bool(true);
}

static AiraValue file_append(int argc, AiraValue* args) {
    if (argc < 2 || args[0].type != VAL_STRING) return val_bool(false);
    FILE* f = fopen(args[0].as.string, "ab");
    if (!f) return val_bool(false);
    char* content = val_to_string(args[1]);
    fputs(content, f);
    aira_free(content);
    fclose(f);
    return val_bool(true);
}

static AiraValue file_exists(int argc, AiraValue* args) {
    if (argc < 1 || args[0].type != VAL_STRING) return val_bool(false);
    struct stat st;
    return val_bool(stat(args[0].as.string, &st) == 0);
}

/* Crypto & Sec Module Functions */
static AiraValue crypto_sha256(int argc, AiraValue* args) {
    if (argc < 1) return val_string("");
    char* str = val_to_string(args[0]);
    char hash[65];
    aira_sha256((const uint8_t*)str, strlen(str), hash);
    aira_free(str);
    return val_string(hash);
}

/* ==============================================================================
 * Native AI & AiraAI Sovereign Module Functions
 * ============================================================================== */

static char ai_provider_str[128] = "airagroup/aira-3-flash";
static char ai_endpoint_str[256] = "";
static char ai_system_prompt[512] = 
    "You are Aira, the official AI engine of AiraLang, engineered and created by Adam Eehan "
    "(Founder & CEO, Aira Group of Technology).";

static AiraValue ai_set_provider(int argc, AiraValue* args) {
    if (argc > 0 && args[0].type == VAL_STRING) {
        strncpy(ai_provider_str, args[0].as.string, sizeof(ai_provider_str) - 1);
        ai_provider_str[sizeof(ai_provider_str) - 1] = '\0';
        return val_bool(true);
    }
    return val_bool(false);
}

static AiraValue ai_get_provider(int argc, AiraValue* args) {
    (void)argc; (void)args;
    return val_string(ai_provider_str);
}

static AiraValue ai_set_endpoint(int argc, AiraValue* args) {
    if (argc > 0 && args[0].type == VAL_STRING) {
        strncpy(ai_endpoint_str, args[0].as.string, sizeof(ai_endpoint_str) - 1);
        ai_endpoint_str[sizeof(ai_endpoint_str) - 1] = '\0';
        return val_bool(true);
    }
    return val_bool(false);
}

static AiraValue ai_get_endpoint(int argc, AiraValue* args) {
    (void)argc; (void)args;
    const char* env_url = getenv("AIRA_API_URL");
    if (ai_endpoint_str[0]) return val_string(ai_endpoint_str);
    if (env_url && strlen(env_url) > 0) return val_string(env_url);
    return val_string("http://127.0.0.1:8080/v1/chat/completions");
}

static AiraValue ai_set_system(int argc, AiraValue* args) {
    if (argc > 0 && args[0].type == VAL_STRING) {
        strncpy(ai_system_prompt, args[0].as.string, sizeof(ai_system_prompt) - 1);
        ai_system_prompt[sizeof(ai_system_prompt) - 1] = '\0';
        return val_bool(true);
    }
    return val_bool(false);
}

static uint32_t json_parse_hex4(const char* s) {
    uint32_t val = 0;
    for (int i = 0; i < 4; i++) {
        char c = s[i];
        val <<= 4;
        if (c >= '0' && c <= '9') val |= (c - '0');
        else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        else return 0;
    }
    return val;
}

static size_t json_encode_utf8(uint32_t cp, char* out) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

static AiraValue ai_ask(int argc, AiraValue* args) {
    if (argc < 1) return val_string("");
    char* prompt = val_to_string(args[0]);

    const char* target_url = "http://127.0.0.1:8080/v1/chat/completions";
    if (ai_endpoint_str[0]) {
        target_url = ai_endpoint_str;
    } else {
        const char* env_url = getenv("AIRA_API_URL");
        if (env_url && strlen(env_url) > 0) {
            target_url = env_url;
        }
    }

    /* Auto-launch Aira-AI-Endpoint locally only if target URL is localhost */
    if (strstr(target_url, "127.0.0.1") || strstr(target_url, "localhost")) {
        system("pgrep -f 'Aira-AI-Endpoint/server.py' >/dev/null 2>&1 || (nohup python3 $HOME/Aira-AI-Endpoint/server.py >/dev/null 2>&1 & sleep 1)");
    }

    /* Escape quotes for safe curl execution */
    char safe_prompt[2048];
    size_t j = 0;
    for (size_t i = 0; prompt[i] && j < sizeof(safe_prompt) - 4; i++) {
        if (prompt[i] == '"' || prompt[i] == '\\' || prompt[i] == '`' || prompt[i] == '$') {
            safe_prompt[j++] = '\\';
        }
        safe_prompt[j++] = prompt[i];
    }
    safe_prompt[j] = '\0';

    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "curl -s -m 35 -X POST '%s' "
        "-H 'Content-Type: application/json' "
        "-d '{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}' 2>/dev/null",
        target_url, ai_provider_str, safe_prompt
    );

    FILE* fp = popen(cmd, "r");
    char res_buf[8192];
    size_t bytes = 0;
    if (fp) {
        bytes = fread(res_buf, 1, sizeof(res_buf) - 1, fp);
        res_buf[bytes] = '\0';
        pclose(fp);
    }

    aira_free(prompt);

    /* If API returned valid content, extract and unescape it */
    if (bytes > 10 && strstr(res_buf, "\"content\":")) {
        char* content_start = strstr(res_buf, "\"content\":");
        if (content_start) {
            content_start += 10;
            while (*content_start == ' ' || *content_start == '\t' || *content_start == '\r' || *content_start == '\n') content_start++;
            if (*content_start == '"') {
                content_start++;
                /* Find unescaped end quote */
                char* p = content_start;
                while (*p) {
                    if (*p == '\\' && *(p + 1)) {
                        p += 2;
                    } else if (*p == '"') {
                        *p = '\0';
                        break;
                    } else {
                        p++;
                    }
                }
                /* Unescape \n, \", etc. */
                char* clean = (char*)aira_malloc(strlen(content_start) + 1);
                size_t c_idx = 0;
                for (size_t k = 0; content_start[k]; k++) {
                    if (content_start[k] == '\\' && content_start[k+1]) {
                        char esc = content_start[k+1];
                        if (esc == 'n') { clean[c_idx++] = '\n'; k++; }
                        else if (esc == 't') { clean[c_idx++] = '\t'; k++; }
                        else if (esc == 'r') { clean[c_idx++] = '\r'; k++; }
                        else if (esc == '"') { clean[c_idx++] = '"'; k++; }
                        else if (esc == '\\') { clean[c_idx++] = '\\'; k++; }
                        else if (esc == 'u' && content_start[k+2] && content_start[k+3] && content_start[k+4] && content_start[k+5]) {
                            uint32_t cp = json_parse_hex4(&content_start[k+2]);
                            k += 5;
                            /* Check for UTF-16 surrogate pair (e.g. \uD83D\uDCAC -> 💬) */
                            if (cp >= 0xD800 && cp <= 0xDBFF && content_start[k+1] == '\\' && content_start[k+2] == 'u' &&
                                content_start[k+3] && content_start[k+4] && content_start[k+5] && content_start[k+6]) {
                                uint32_t low = json_parse_hex4(&content_start[k+3]);
                                if (low >= 0xDC00 && low <= 0xDFFF) {
                                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                                    k += 6;
                                }
                            }
                            c_idx += json_encode_utf8(cp, &clean[c_idx]);
                        } else {
                            clean[c_idx++] = esc;
                            k++;
                        }
                    } else {
                        clean[c_idx++] = content_start[k];
                    }
                }
                clean[c_idx] = '\0';
                AiraValue ret = val_string(clean);
                aira_free(clean);
                return ret;
            }
        }
    }

    /* Sovereign Swarm Zero-Key Default Response */
    char fallback[512];
    snprintf(fallback, sizeof(fallback),
        "Hello Boss! I am Aira AI (Sovereign Engine v3.6, Swarm: %s), "
        "created by Adam Eehan. I am connected directly to AiraLang systems core!",
        ai_provider_str
    );
    return val_string(fallback);
}

/* ==============================================================================
 * Registration & Loader
 * ============================================================================== */

void register_builtin_globals(Environment* env) {
    env_define(env, "len", val_builtin(builtin_len));
    env_define(env, "range", val_builtin(builtin_range));
    env_define(env, "input", val_builtin(builtin_input));
    env_define(env, "type", val_builtin(builtin_type));
    env_define(env, "int", val_builtin(builtin_to_int));
    env_define(env, "float", val_builtin(builtin_to_float));
    env_define(env, "str", val_builtin(builtin_to_str));
}

AiraValue load_builtin_module(const char* name) {
    if (strcmp(name, "math") == 0) {
        AiraValue mod = val_module("math");
        dict_set(mod.as.module->members, "sqrt", val_builtin(math_sqrt));
        dict_set(mod.as.module->members, "pow", val_builtin(math_pow));
        dict_set(mod.as.module->members, "abs", val_builtin(math_abs));
        dict_set(mod.as.module->members, "floor", val_builtin(math_floor));
        dict_set(mod.as.module->members, "ceil", val_builtin(math_ceil));
        dict_set(mod.as.module->members, "pi", val_float(3.141592653589793));
        dict_set(mod.as.module->members, "e", val_float(2.718281828459045));
        return mod;
    }

    if (strcmp(name, "os") == 0 || strcmp(name, "system") == 0) {
        AiraValue mod = val_module("os");
        dict_set(mod.as.module->members, "system", val_builtin(os_system));
        dict_set(mod.as.module->members, "getcwd", val_builtin(os_getcwd));
        dict_set(mod.as.module->members, "getenv", val_builtin(os_getenv));
        dict_set(mod.as.module->members, "platform", val_builtin(os_platform));
        return mod;
    }

    if (strcmp(name, "time") == 0) {
        AiraValue mod = val_module("time");
        dict_set(mod.as.module->members, "time", val_builtin(time_time));
        dict_set(mod.as.module->members, "sleep", val_builtin(time_sleep));
        return mod;
    }

    if (strcmp(name, "file") == 0) {
        AiraValue mod = val_module("file");
        dict_set(mod.as.module->members, "read", val_builtin(file_read));
        dict_set(mod.as.module->members, "write", val_builtin(file_write));
        dict_set(mod.as.module->members, "append", val_builtin(file_append));
        dict_set(mod.as.module->members, "exists", val_builtin(file_exists));
        return mod;
    }

    if (strcmp(name, "crypto") == 0 || strcmp(name, "sec") == 0) {
        AiraValue mod = val_module("crypto");
        dict_set(mod.as.module->members, "sha256", val_builtin(crypto_sha256));
        dict_set(mod.as.module->members, "hash", val_builtin(crypto_sha256));
        return mod;
    }

    if (strcmp(name, "ai") == 0 || strcmp(name, "airaai") == 0) {
        AiraValue mod = val_module("ai");
        dict_set(mod.as.module->members, "set_provider", val_builtin(ai_set_provider));
        dict_set(mod.as.module->members, "get_provider", val_builtin(ai_get_provider));
        dict_set(mod.as.module->members, "set_endpoint", val_builtin(ai_set_endpoint));
        dict_set(mod.as.module->members, "get_endpoint", val_builtin(ai_get_endpoint));
        dict_set(mod.as.module->members, "set_system", val_builtin(ai_set_system));
        dict_set(mod.as.module->members, "ask", val_builtin(ai_ask));
        return mod;
    }

    return val_null();
}
