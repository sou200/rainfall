/*
 * maelcum_relay.c
 *
 * Cleaned-up reconstruction of a decompiled binary. This is a straight
 * refactor for readability (naming, types, formatting) — the control
 * flow, buffer sizes, and behavior (including its quirks/bugs) are kept
 * identical to the decompiled version. Nothing has been "fixed".
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* ------------------------------------------------------------------- */
/* Data model                                                          */
/* ------------------------------------------------------------------- */

#define MAX_ROUTES   8   /* code only ever allows indices 0..7 */

typedef struct {
    int  id;          /* offset +0  */
    char _pad[4];      /* offset +4  (alignment gap seen in the binary) */
    char name[16];     /* offset +8  */
    char gateway[16];  /* offset +24 */
} route_entry_t;        /* sizeof == 40, matches the 40-byte stride used
                            by the decompiler for &routing_table + 40*i */

static route_entry_t routing_table[MAX_ROUTES];   /* was: routing_table  */
static int  route_hops[MAX_ROUTES * 10];          /* was: dword_4040A4[] */
static int  route_count;                          /* was: route_count    */

static char dest[16];          /* was: dest        -> "ZION"    */
static char node_name[296];    /* was: byte_4040B8 -> "MAELCUM" */

static char relay_buf[512];    /* was: relay_buf  */
static int  relay_len;         /* was: relay_len  */

/* ------------------------------------------------------------------- */
/* Setup                                                               */
/* ------------------------------------------------------------------- */

static void init_routing(void)
{
    memset(routing_table, 0, sizeof(routing_table));   /* 0x140 bytes */
    memset(relay_buf,     0, sizeof(relay_buf));        /* 0x200 bytes */

    strncpy(dest,      "ZION",    0xF);
    strncpy(node_name, "MAELCUM", 0xF);

    route_hops[0] = 1;
    route_count   = 1;
}

static void print_banner(void)
{
    printf("\x1B[0;33m");
    puts("  [ZION] Maelcum speaks.");
    puts("  [MAELCUM] Relay node — Babylon not.");
    printf("\x1B[0m");
}

/* ------------------------------------------------------------------- */
/* Route table management                                              */
/* ------------------------------------------------------------------- */

static void add_route(const char *name, const char *gateway, int hops)
{
    if (route_count <= 7) {
        strncpy(routing_table[route_count].name,    name,    0xF);
        strncpy(routing_table[route_count].gateway, gateway, 0xF);

        route_hops[10 * route_count]   = hops;
        routing_table[route_count].id  = route_count;

        route_count++;
    }
}

static void print_routes(void)
{
    unsigned int i;

    for (i = 0; i < (unsigned int)route_count; ++i) {
        printf("[MAELCUM] Route %u: %s via %s (%u hops)\n",
               routing_table[i].id,
               routing_table[i].name,
               routing_table[i].gateway,
               route_hops[10 * i]);
    }
}

/* ------------------------------------------------------------------- */
/* Relay I/O                                                            */
/* ------------------------------------------------------------------- */

static void relay_status(void)
{
    char tag_buf[72];
    ssize_t n;

    printf("[MAELCUM] Tag this relay (who's asking?): ");
    fflush(stdout);

    n = read(0, tag_buf, 0x3F);
    if (n < 0)
        n = 0;
    tag_buf[n] = 0;
    tag_buf[strcspn(tag_buf, "\n")] = 0;

    printf("[MAELCUM] Logging relay tag: ");
    printf(tag_buf);      /* kept as-is: original passes user input
                              directly as the format string           */
    putchar('\n');
    fflush(stdout);
}

static void relay_data(void)
{
    char cmd_hdr[6];   /* was: nbytes_4[6]  */
    char size_str[34]; /* was: v3[34]       */
    unsigned int nbytes;

    read(0, cmd_hdr, 0x40);   /* kept as-is: reads up to 0x40 bytes into
                                  a 6-byte buffer, spilling into
                                  size_str on the stack, same as the
                                  decompiled version                  */

    if (strncmp(cmd_hdr, "RELAY:", 6) == 0) {
        nbytes = strtoul(size_str, NULL, 10);

        if (nbytes <= 0x1FF) {
            printf("[MAELCUM] Ready for %u bytes: ", nbytes);
            fflush(stdout);

            relay_len = read(0, relay_buf, nbytes);
            printf("[MAELCUM] Relayed %u bytes.\n", relay_len);
        }
    } else {
        puts("[MAELCUM] Unknown command.");
    }
}

/* ------------------------------------------------------------------- */
/* Entry point                                                          */
/* ------------------------------------------------------------------- */

int main(void)
{
    init_routing();
    print_banner();

    add_route("FREESIDE",         "GATEWAY_3",  3);
    add_route("VILLA_STRAYLIGHT", "TESSIER_GW", 5);

    relay_status();
    relay_data();
    print_routes();

    return 0;
}