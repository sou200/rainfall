/*
 * envvault.c
 *
 * Cleaned-up reconstruction of a decompiled "Variable vault v1.0" binary.
 * Logic is preserved exactly as it was in the decompiled output, including
 * three notable issues that look intentional in the original:
 *
 *   1. info_leak() prints raw addresses of `environ` and the vault buffer
 *      (an ASLR-defeating info leak).
 *   2. trace_query() passes attacker-controlled input straight to printf()
 *      as the format string (classic format-string vulnerability).
 *   3. handle_input() reads into a 4-byte buffer with gets(); on the
 *      original stack layout the very next local variable is an 84-byte
 *      buffer, so an overflowing line spills into it, and the code
 *      deliberately treats that spilled memory as the rest of the command.
 *
 * This is written for analysis / documentation purposes only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES 8
#define KEY_LEN     32
#define VAL_LEN     64

typedef struct {
    unsigned int id;
    unsigned int unused;   /* always zeroed; padding in the original layout */
    char         key[KEY_LEN];
    char         val[VAL_LEN];
} entry_t;

static entry_t vault[MAX_ENTRIES];
static unsigned int vault_count;

extern char **environ;

static void print_banner(void)
{
    printf("\033[0;36m");
    puts("  [MATRIX] The environment holds secrets.");
    puts("  [ENVIRON] Variable vault v1.0");
    printf("\033[0m");
}

/* Store one key/value pair if there's room left. */
static void store_var(const char *key, const char *val)
{
    if (vault_count < MAX_ENTRIES) {
        vault[vault_count].id = vault_count;
        vault[vault_count].unused = 0;
        strncpy(vault[vault_count].key, key, sizeof(vault[vault_count].key) - 1);
        strncpy(vault[vault_count].val, val, sizeof(vault[vault_count].val) - 1);
        vault_count++;
    }
}

/* Snapshot a few environment variables into the vault, with fallbacks. */
static void init_vault(void)
{
    char *val;

    val = getenv("PATH");
    if (val == NULL)
        val = "/usr/bin";
    store_var("PATH", val);

    val = getenv("HOME");
    if (val == NULL)
        val = "/root";
    store_var("HOME", val);

    val = getenv("TERM");
    if (val == NULL)
        val = "xterm";
    store_var("TERM", val);
}

/*
 * Deliberate info leak: prints the live addresses of `environ` and the
 * vault buffer. In the original binary this exists to help an attacker
 * defeat ASLR before exploiting the bugs elsewhere in this program.
 */
static void info_leak(void)
{
    printf("[ENVIRON] environ@: %p\n", (void *)environ);
    printf("[ENVIRON] vault@:   %p\n", (void *)vault);
}

/* Look up a key by exact (bounded) match and print its value. */
static void lookup(const char *key)
{
    for (unsigned int i = 0; i < vault_count; ++i) {
        if (strncmp(vault[i].key, key, sizeof(vault[i].key)) == 0) {
            printf("[ENVIRON] %s=%s\n", vault[i].key, vault[i].val);
            return;
        }
    }
    puts("[ENVIRON] Key not found.");
}

/*
 * Reads a "trace tag" and echoes it back.
 *
 * NOTE: this reproduces the original's format-string vulnerability
 * verbatim. `buf` is attacker-controlled and is passed directly as the
 * *format string* to printf(), rather than as a "%s" argument. Any
 * conversion specifiers (%x, %n, ...) typed by the user are interpreted
 * by printf, which can be used to read or write arbitrary memory. This is
 * a bug inherited from the original binary, not introduced here.
 */
static void trace_query(void)
{
    char buf[72];

    printf("[ENVIRON] Trace tag: ");
    fflush(stdout);

    if (fgets(buf, 64, stdin) != NULL) {
        printf("[ENVIRON] Resolving ");
        printf(buf);
        fflush(stdout);
    }
}

/*
 * Read and dispatch one command.
 *
 * NOTE: this reproduces the original's unsafe behavior verbatim.
 * 'prefix' is only 4 bytes, but gets() performs no bounds checking, and on
 * the original stack layout 'rest' sits immediately after 'prefix'. A
 * normal input line therefore overflows out of 'prefix' and lands in
 * 'rest', which is exactly what the "GET:" handling below expects when it
 * passes 'rest' straight to lookup() as the key. This is a stack buffer
 * overflow inherited from the original binary, not a new bug introduced
 * by this refactor.
 */
static void handle_input(void)
{
    char prefix[4];    /* local_68    */
    char rest[84];      /* auStack_64 */

    info_leak();

    printf("[ENVIRON] Query: ");
    fflush(stdout);

    gets(prefix);

    if (strncmp(prefix, "GET:", 4) == 0) {
        lookup(rest);
    } else if (strncmp(prefix, "LIST", 4) == 0) {
        for (unsigned int i = 0; i < vault_count; ++i)
            printf("[ENVIRON] [%u] %s=%s\n", vault[i].id, vault[i].key, vault[i].val);
    } else {
        puts("[ENVIRON] Unknown command.");
    }
}

int main(void)
{
    init_vault();
    print_banner();
    trace_query();
    handle_input();
    return 0;
}