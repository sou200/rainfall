/*
 * SENDAI connection broker
 *
 * Reconstructed from Hex-Rays decompiler output.
 * Logic, buffer sizes, and control flow are preserved exactly as
 * decompiled - this is a faithful rewrite, not a fixed/patched version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_CONNECTIONS 8
#define HOST_SIZE       32   /* 0x20 */
#define CMD_SIZE        64   /* s + v5 + v6 + v7 in the decompiled output */
#define SEND_BUF_SIZE   136  /* 0x88 - the `format` buffer */

typedef struct {
    unsigned int id;
    unsigned int port;
    unsigned int connected;
    char         host[HOST_SIZE];
    void (*disconnect)(unsigned int id);
} connection_t;

static connection_t conns[NUM_CONNECTIONS];

static void disconnect_default(unsigned int id)
{
    printf("[SENDAI] Connection %u closed.\n", id);
}

static void init_connections(void)
{
    for (unsigned int i = 0; i < NUM_CONNECTIONS; ++i) {
        conns[i].id         = i;
        conns[i].connected  = 0;
        conns[i].disconnect = disconnect_default;
        snprintf(conns[i].host, HOST_SIZE, "node-%04x.sprawl.net", 4919 * i);
        conns[i].port = 13 * i + 1024;
    }
}

static void print_banner(void)
{
    printf("\x1B[0;34m");
    puts("  [SENDAI] Connection broker online.");
    puts("  [SENDAI] Ono-Sendai routing layer v5.0");
    printf("\x1B[0m");
}

/* Prints the address of a function, exactly as in the decompiled version. */
static void info_leak(void)
{
    printf("[SENDAI] disconnect@binary: %p\n", disconnect_default);
}

static void handle_command(void)
{
    char cmd[CMD_SIZE];
    char format[SEND_BUF_SIZE];
    unsigned int conn_id;

    info_leak();

    while (1) {
        printf("[SENDAI] Command: ");
        fflush(stdout);

        if (fgets(cmd, sizeof(cmd), stdin) == NULL)
            break;

        cmd[strcspn(cmd, "\n")] = 0;

        if (strncmp(cmd, "CONNECT:", 8) == 0) {
            conn_id = strtoul(cmd + 8, NULL, 10);
            if (conn_id <= 7) {
                conns[conn_id].connected = 1;
                printf("[SENDAI] Connected to %s:%u\n",
                       conns[conn_id].host, conns[conn_id].port);
            }
        }
        else if (strncmp(cmd, "SEND:", 5) == 0) {
            conn_id = strtoul(cmd + 5, NULL, 10);
            if (conn_id <= 7 && conns[conn_id].connected != 0) {
                printf("[SENDAI] Data for conn %u: ", conn_id);
                fflush(stdout);
                gets(format);
                printf("[SENDAI] Sent: ");
                printf(format);
                putchar('\n');
            }
        }
        else if (strncmp(cmd, "DISCONNECT:", 11) == 0) {
            conn_id = strtoul(cmd + 11, NULL, 10);
            if (conn_id <= 7)
                conns[conn_id].disconnect(conn_id);
        }
        else if (strncmp(cmd, "QUIT", 4) == 0) {
            return;
        }
        else {
            puts("[SENDAI] Unknown command.");
        }
    }
}

int main(int argc, const char **argv, const char **envp)
{
    init_connections();
    print_banner();
    handle_command();
    return 0;
}