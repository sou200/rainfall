/*
 * Refactored / human-readable version of the decompiled "NEUROMANCER" binary.
 *
 * NOTE: This preserves the original logic EXACTLY, including its bugs:
 *   - console_handshake() has a format-string vulnerability
 *     (printf(buf) instead of printf("%s", buf))
 *   - process_input() reads into a fixed-size stack buffer with gets(),
 *     which has no bounds checking (classic stack buffer overflow)
 *   - info_leak() prints the runtime address of puts(), presumably to
 *     help defeat ASLR for an exploit chain
 *
 * These are left in place intentionally per the "don't change the logic"
 * requirement. This looks like a CTF pwn challenge.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Types & globals                                                     */
/* ------------------------------------------------------------------ */

#define MAX_NODES 12

/* Original struct was 0x28 (40) bytes:
 *   +0x00  id
 *   +0x04  type
 *   +0x08  flags
 *   +0x0c  name[16]
 *   +0x20  handler (function pointer)
 */
typedef struct {
    unsigned int id;
    unsigned int type;
    unsigned int flags;
    char         name[16];
    void       (*handler)(void);
} node_t;

static node_t      nodes[MAX_NODES];
static unsigned int node_count;

/* ------------------------------------------------------------------ */
/* Handlers                                                            */
/* ------------------------------------------------------------------ */

/* Default per-node handler; every node starts out pointing here. */
static void default_handler(void)
{
    puts("[NEUROMANCER] Default handler invoked.");
}

/* ------------------------------------------------------------------ */
/* Setup / display                                                     */
/* ------------------------------------------------------------------ */

static void init_nodes(void)
{
    for (unsigned int i = 0; i < MAX_NODES; i++) {
        nodes[i].id    = i;
        nodes[i].type  = i % 3;
        nodes[i].flags = 0;
        snprintf(nodes[i].name, sizeof(nodes[i].name), "NODE_%04x", i * 0x1a2b);
        nodes[i].handler = default_handler;
    }
    node_count = MAX_NODES;
}

static void print_banner(void)
{
    printf("\x1b[0;35m");
    puts("  [NEUROMANCER] I am the land of the dead.");
    puts("  [NEUROMANCER] You should not be here.");
    printf("\x1b[0m");
}

static void show_node(unsigned int idx)
{
    if (idx < node_count) {
        printf("[NEUROMANCER] Node %u: %s type=%u flags=%08x\n",
               nodes[idx].id,
               nodes[idx].name,
               nodes[idx].type,
               nodes[idx].flags);
    }
}

/* Leaks the runtime address of puts() in libc (no ASLR mitigation). */
static void info_leak(void)
{
    printf("[NEUROMANCER] puts@libc: %p\n", (void *)puts);
}

/* ------------------------------------------------------------------ */
/* Input handling                                                      */
/* ------------------------------------------------------------------ */

/*
 * BUG (kept intentionally): user-controlled input is passed directly as
 * the format string, so anything with %x/%n/etc. in it will be
 * interpreted by printf -- a format-string vulnerability.
 */
static void console_handshake(void)
{
    char handshake_buf[72];

    printf("[NEUROMANCER] Console handle: ");
    fflush(stdout);

    if (fgets(handshake_buf, sizeof(handshake_buf) - 8, stdin) != NULL) {
        printf("[NEUROMANCER] Trace echo: ");
        printf(handshake_buf);          /* <-- format string bug, left as-is */
    }
}

/*
 * BUG (kept intentionally): the original stack layout placed two
 * decompiler-split variables ("local_98" and "acStack_93")
 * contiguously in memory. In reality this is one 136-byte command
 * buffer, with the portion after the first 5 bytes reused as the
 * argument to "NODE:". gets() is used with no bounds checking, so this
 * is a classic stack buffer overflow.
 */
static void process_input(void)
{
    char cmd[136];              /* combined local_98 (5) + acStack_93 (131) */
    char *arg = cmd + 5;        /* where a numeric argument starts, e.g. "NODE:<n>" */

    printf("[NEUROMANCER] Interface: ");
    fflush(stdout);

    gets(cmd);                  /* <-- unbounded read, left as-is */

    if (strncmp(cmd, "NODE:", 5) == 0) {
        unsigned long node_idx = strtoul(arg, NULL, 10);
        show_node((unsigned int)node_idx);
    } else if (strncmp(cmd, "LIST", 4) == 0) {
        for (unsigned int i = 0; i < node_count; i++) {
            show_node(i);
        }
    } else {
        puts("[NEUROMANCER] Unknown command.");
    }
}

static void handle_command(void)
{
    info_leak();
    console_handshake();
    process_input();
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    init_nodes();
    print_banner();
    handle_command();
    return 0;
}