/*
 * kvstore.c
 *
 * Cleaned-up reconstruction of a decompiled "Key-value store v3.1" binary.
 * Logic is preserved exactly as it was in the decompiled output, including
 * the (intentional-looking) stack buffer overflow in handle_input() caused
 * by gets(). This is written for analysis / documentation purposes only.
 *
 * Original decompiler artifacts (dword_403564[] vs. table) were actually
 * the SAME 4-byte field of each 72-byte record viewed through two
 * different casts -- init_table()/insert() write an index into it and then
 * immediately overwrite it with the djb2 hash of the key. That's modeled
 * below as a single `hash` field per entry that gets written twice.
 */

#include <stdio.h>
#include <string.h>

#define MAX_ENTRIES     16   /* table holds 0x480 / 72 = 16 slots        */
#define KEY_LEN         32
#define VAL_LEN         32
#define DEFAULT_ENTRIES 4    /* number of built-in KEY_xxxx/VAL_xxxx rows */

typedef struct {
    unsigned int hash;       /* djb2(key); briefly holds the slot index first */
    char         key[KEY_LEN];
    char         val[VAL_LEN];
} entry_t;

static entry_t table[MAX_ENTRIES];
static int entry_count;

/* Classic djb2 string hash (xor variant), same as the original binary. */
static unsigned int djb2(const unsigned char *str)
{
    unsigned int hash = 5381;

    while (*str)
        hash = (33 * hash) ^ *str++;

    return hash;
}

/* Seed the table with a handful of deterministic default entries. */
static void init_table(void)
{
    memset(table, 0, sizeof(table));

    for (unsigned int i = 0; i < DEFAULT_ENTRIES; ++i) {
        table[i].hash = i;   /* placeholder, overwritten below */

        snprintf(table[i].key, sizeof(table[i].key), "KEY_%04x", 0xDEAD * i);
        snprintf(table[i].val, sizeof(table[i].val), "VAL_%04x", 0xBEEF * i);

        table[i].hash = djb2((unsigned char *)table[i].key);
        entry_count++;
    }
}

static void print_banner(void)
{
    printf("\033[0;36m");
    puts("  [MATRIX] The cyberspace awaits.");
    puts("  [MATRIX] Key-value store v3.1");
    printf("\033[0m");
}

/* Look up a key by hash + strncmp and print the result. */
static void lookup(const unsigned char *key)
{
    unsigned int target_hash = djb2(key);

    for (unsigned int i = 0; i < (unsigned int)entry_count; ++i) {
        if (target_hash == table[i].hash &&
            strncmp(table[i].key, (const char *)key, sizeof(table[i].key)) == 0) {
            printf("[MATRIX] Found: %s => %s\n", table[i].key, table[i].val);
            return;
        }
    }

    puts("[MATRIX] Key not found.");
}

/* Append a new key/value pair if there's room. */
static void insert(const char *key, const char *val)
{
    if (entry_count > MAX_ENTRIES - 1)   /* original check: entry_count <= 0xF */
        return;

    table[entry_count].hash = entry_count; /* placeholder, overwritten below */
    strncpy(table[entry_count].key, key, sizeof(table[entry_count].key) - 1);
    strncpy(table[entry_count].val, val, sizeof(table[entry_count].val) - 1);
    table[entry_count].hash = djb2((unsigned char *)key);

    entry_count++;
    printf("[MATRIX] Inserted: %s\n", key);
}

/*
 * Read and dispatch one command.
 *
 * NOTE: this reproduces the original's unsafe behavior verbatim.
 * 'prefix' is only 4 bytes, but gets() performs no bounds checking, and on
 * the original stack layout 'command_body' sits immediately after
 * 'prefix'. A normal input line therefore overflows out of 'prefix' and
 * lands in 'command_body', which is exactly what the parsing code below
 * expects when it reads the text following the 4-byte "GET:"/"SET:" tag.
 * This is a stack buffer overflow inherited from the original binary, not
 * a new bug introduced by this refactor.
 */
static void handle_input(void)
{
    char set_value[VAL_LEN];   /* v2   */
    char key_buf[KEY_LEN];     /* dest */
    char prefix[4];            /* s1   */
    char command_body[100];    /* v5   */
    char *sep;                 /* v6   */

    printf("[MATRIX] Command (GET/SET): ");
    fflush(stdout);

    gets(prefix);

    if (strncmp(prefix, "GET:", 4) == 0) {
        strncpy(key_buf, command_body, sizeof(key_buf) - 1);
        lookup((unsigned char *)key_buf);
    } else if (strncmp(prefix, "SET:", 4) == 0) {
        sep = strchr(command_body, ':');
        if (sep != NULL) {
            *sep = '\0';
            strncpy(key_buf, command_body, sizeof(key_buf) - 1);
            strncpy(set_value, sep + 1, sizeof(set_value) - 1);
            insert(key_buf, set_value);
        }
    } else {
        puts("[MATRIX] Unknown command.");
    }
}

int main(void)
{
    init_table();
    print_banner();
    handle_input();
    return 0;
}