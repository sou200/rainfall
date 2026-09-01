/*
 * FLATLINE construct manager
 *
 * Reconstructed from Hex-Rays decompiler output.
 * Logic, sizes, and offsets are preserved exactly as decompiled -
 * this is a faithful rewrite, not a fixed/patched version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CONSTRUCTS   4
#define NAME_SIZE        32   /* 0x20 */
#define DATA_SIZE        64   /* 0x40 */
#define UPDATE_READ_SIZE 160  /* 0xA0 - intentionally larger than DATA_SIZE */

typedef struct construct {
    unsigned int id;
    unsigned int active;
    char name[NAME_SIZE];
    char data[DATA_SIZE];
    void (*execute)(void *self);
} construct_t;

static construct_t *constructs[MAX_CONSTRUCTS];
static int construct_count;

/* Default handler installed on every new construct. */
static void default_execute(void *self)
{
    /* Interprets whatever is at `self` as a construct-style
       {id, name} pair, exactly as in the decompiled version. */
    printf("[FLATLINE] Executing construct %u: %s\n",
           *(unsigned int *)self, (char *)self + 8);
}

static void print_banner(void)
{
    printf("\x1B[0;35m");
    puts("  [FLATLINE] ROM construct system.");
    puts("  [FLATLINE] Construct manager v2.0");
    printf("\x1B[0m");
}

static construct_t *new_construct(const char *name)
{
    if ((unsigned int)construct_count > 3)
        return NULL;

    construct_t *c = malloc(sizeof(*c));
    if (c == NULL)
        return NULL;

    c->id     = construct_count;
    c->active = 1;
    c->execute = default_execute;

    strncpy(c->name, name, NAME_SIZE - 1);
    memset(c->data, 0, DATA_SIZE);

    constructs[construct_count] = c;
    construct_count++;

    printf("[FLATLINE] Construct %u created: %s\n", c->id, c->name);
    return c;
}

static void delete_construct(unsigned int index)
{
    if (index < (unsigned int)construct_count && constructs[index] != NULL) {
        printf("[FLATLINE] Deleting construct %u\n", index);
        free(constructs[index]);
        /* note: the slot is not cleared after the free,
           matching the original decompiled behavior */
    }
}

static void update_construct(unsigned int index)
{
    if (index < (unsigned int)construct_count && constructs[index] != NULL) {
        printf("[FLATLINE] Update data for construct %u: ", index);
        fflush(stdout);
        /* reads more bytes than `data` actually holds,
           matching the original decompiled behavior */
        read(0, constructs[index]->data, UPDATE_READ_SIZE);
        printf("[FLATLINE] Construct %u updated.\n", index);
    }
}

static void run_construct(unsigned int index)
{
    if (index < (unsigned int)construct_count && constructs[index] != NULL) {
        constructs[index]->execute(constructs[index]->data);
    }
}

int main(int argc, const char **argv, const char **envp)
{
    uid_t euid = geteuid();
    setreuid(euid, euid);

    print_banner();

    new_construct("DIXIE");
    new_construct("WINTERMUTE");

    delete_construct(0);
    update_construct(0);
    run_construct(0);

    return 0;
}