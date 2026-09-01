/*
 * vault.c - "3JANE" access vault demo
 *
 * Reconstructed from a decompilation. The CRT/startup boilerplate that
 * Hex-Rays showed (_init, _start, register_tm_clones, frame_dummy, etc.)
 * is compiler/linker-generated glue, not code a person would have written,
 * so it's left out here. Everything below preserves the original program's
 * behavior exactly, including its bugs.
 */

#include <stdio.h>
#include <string.h>

#define NUM_ENTRIES 4

typedef struct {
    int  id;
    int  _reserved;        /* unused 4-byte gap between id and label */
    char label[16];
    char entry_id[32];
} VaultEntry;

/* Clearance level assigned to each vault entry (1, 2, 4, 8 - bitflags). */
static const int clearance_levels[NUM_ENTRIES] = { 1, 2, 4, 8 };

/*
 * Only the first label was named by the decompiler ("labels_0" = "ALPHA");
 * the loop below reads it as the base of a 4-entry string table, i.e. it
 * walks 3 more char* globals that sit right after this one in memory.
 * Kept as the original pointer-indexing trick rather than guessing at the
 * unnamed strings' actual values.
 */
static const char *label_table = "ALPHA";

static VaultEntry vault[NUM_ENTRIES];

/*
 * Second, apparently unrelated array that mirrors the vault's stride
 * (14 ints = 56 bytes) but only ever uses index 0 of each 14-int "slot"
 * to store a clearance level. Reproduced as-is rather than folded into
 * VaultEntry, since that's how the binary actually lays it out.
 */
static int clearance_data[55];

static int access_log_idx;

static void init_vault(void)
{
    for (unsigned i = 0; i < NUM_ENTRIES; ++i) {
        vault[i].id = i;
        clearance_data[14 * i] = clearance_levels[i];
        strncpy(vault[i].label, (&label_table)[i], 15);
        snprintf(vault[i].entry_id, sizeof(vault[i].entry_id),
                 "entry_%04x", 4919 * i);
    }
}

static void print_banner(void)
{
    printf("\x1B[0;37m");
    puts("  [VILLA STRAYLIGHT] Access restricted.");
    puts("  [3JANE] Convince me.");
    printf("\x1B[0m");
}

static void log_request(void)
{
    char request_id[64];

    printf("[3JANE] Request ID: ");
    fflush(stdout);
    fgets(request_id, sizeof(request_id), stdin);
    request_id[strcspn(request_id, "\n")] = 0;

    printf("[3JANE] Logging: ");
    printf(request_id);   /* NOTE: format-string bug - user input used as the format */
    putchar('\n');

    ++access_log_idx;
}

static void authenticate(void)
{
    char access_code[136];

    printf("[3JANE] Access code: ");
    fflush(stdout);
    gets(access_code);    /* NOTE: classic unbounded gets() - stack buffer overflow */

    if (strncmp(access_code, "STRAYLIGHT_", 11) == 0)
        printf("[3JANE] Access granted. Clearance level: %u\n", clearance_data[0]);
    else
        puts("[3JANE] Access denied.");
}

static void display_vault(void)
{
    puts("[3JANE] Vault status:");
    for (unsigned i = 0; i < NUM_ENTRIES; ++i) {
        printf("  [%u] %s clearance=%u\n",
               vault[i].id, vault[i].label, clearance_data[14 * i]);
    }
}

int main(void)
{
    init_vault();
    print_banner();
    log_request();
    authenticate();
    display_vault();
    return 0;
}