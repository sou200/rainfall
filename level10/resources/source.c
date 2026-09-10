/*
 * ta_vault.c
 *
 * Human-readable reconstruction of the decompiled binary.
 * Behavior, control flow, buffer sizes and bugs are preserved exactly
 * as they were in the original decompilation (including the unsafe
 * gets() call and the format-string bug in format_log()) — this is a
 * cleanup/refactor pass, not a security fix.
 */

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Data structures                                                     */
/* ------------------------------------------------------------------ */

#define MAX_SESSIONS 4
#define SESSION_TAG_LEN 32

typedef struct {
    int  id;                       /* session index                    */
    int  privilege;                /* 0 = none, 0xFF = root             */
    int  active;                   /* 1 = active, 0 = terminated        */
    char tag[SESSION_TAG_LEN];     /* attacker-supplied session tag     */
} session_t;

typedef struct {
    int  magic;                    /* 0xCAFEBABE                        */
    int  size;                     /* 512                                */
    int  level;                    /* 3                                  */
    char name[16];                 /* "T-A_VAULT"                        */
    char message[260];             /* banner shown after root auth       */
} vault_t;

/* ------------------------------------------------------------------ */
/* Globals                                                             */
/* ------------------------------------------------------------------ */

static vault_t   vault;
static session_t sessions[MAX_SESSIONS];
static int       session_count;

/* ------------------------------------------------------------------ */
/* Initialization                                                      */
/* ------------------------------------------------------------------ */

static void init_vault(void)
{
    vault.magic = (int)0xCAFEBABE;
    vault.size  = 512;
    vault.level = 3;

    strncpy(vault.name, "T-A_VAULT", sizeof(vault.name) - 1);

    memset(vault.message, 0, sizeof(vault.message));
    strncpy(vault.message,
            "TESSIER-ASHPOOL SA \xE2\x80\x94 Final authentication required.",
            sizeof(vault.message) - 1);
}

static void init_sessions(void)
{
    memset(sessions, 0, sizeof(sessions));
}

/* ------------------------------------------------------------------ */
/* Banner / logging                                                    */
/* ------------------------------------------------------------------ */

static void print_banner(void)
{
    printf("\x1B[0;31m");
    puts("  [TESSIER-ASHPOOL SA] This system is beyond you.");
    puts("  [T-A] Final authentication required.");
    printf("\x1B[0m");
}

/* NOTE: original bug preserved — the user-controlled tag is passed
 * directly as the format string to printf(), i.e. a classic format
 * string vulnerability. Not fixed here, per "keep the same logic". */
static void format_log(void)
{
    char tag[40];

    printf("[T-A] Session tag: ");
    fflush(stdout);
    fgets(tag, 32, stdin);
    tag[strcspn(tag, "\n")] = 0;

    printf("[T-A] Tag accepted: ");
    printf(tag);          /* <-- format string bug, kept intentionally */
    putchar('\n');
}

/* ------------------------------------------------------------------ */
/* Session handling                                                    */
/* ------------------------------------------------------------------ */

static int create_session(const char *token)
{
    if (session_count > 3)
        return -1;

    session_t *s = &sessions[session_count];

    s->id        = session_count;
    s->privilege = 0;
    s->active    = 1;
    strncpy(s->tag, token, sizeof(s->tag) - 1);

    return session_count++;
}

/* NOTE: original bug preserved — authentication reads the token with
 * gets(), an unbounded read into a fixed 72-byte stack buffer
 * (classic stack buffer overflow). Not fixed here, per "keep the
 * same logic". */
static void authenticate(void)
{
    char token[72];
    int  session;

    printf("[T-A] Authentication token: ");
    fflush(stdout);
    gets(token);                       /* <-- unbounded read, kept intentionally */

    session = create_session(token);

    if (session >= 0) {
        if (strncmp(token, "TA_ROOT_", 8) == 0) {
            sessions[session].privilege = 0xFF;
            printf("[T-A] Root access granted. Session %d.\n", session);
            printf("[T-A] Vault: %s\n", vault.message);
        } else {
            printf("[T-A] Access denied. Session %d terminated.\n", session);
            sessions[session].active = 0;
        }
    } else {
        puts("[T-A] Session limit reached.");
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    init_vault();
    init_sessions();
    print_banner();
    format_log();
    authenticate();
    return 0;
}