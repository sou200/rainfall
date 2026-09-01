/*
 * ZION - toy heap allocator / block manager
 *
 * Reconstructed from a Hex-Rays decompilation. Struct layout, control
 * flow, and (accidentally) the original bugs are preserved exactly;
 * only naming, formatting, and comments have been cleaned up.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BLOCKS   8
#define LABEL_LEN    15   /* strncpy copies at most this many bytes */

/*
 * Each block is a 32-byte heap allocation:
 *   id       (4 bytes)
 *   size     (4 bytes)
 *   label    (16 bytes, strncpy'd from caller-supplied string)
 *   on_free  (8 bytes, function pointer invoked when the block is freed)
 *
 * sizeof(block_t) == 0x20, matching the malloc(0x20) call in alloc_block().
 */
typedef struct block {
    unsigned int id;
    unsigned int size;
    char         label[16];
    void       (*on_free)(struct block *self);
} block_t;

static block_t *blocks[MAX_BLOCKS];
static int      block_count;

/* Default per-block destructor. Does nothing but must exist so that
 * free_block() always has a valid function pointer to call. */
static void default_on_free(block_t *self)
{
    (void)self;
}

static void print_banner(void)
{
    printf("\x1b[0;32m");
    puts("  [ZION] The station endures.");
    puts("  [ZION] Memory allocator v1.1");
    printf("\x1b[0m");
}

/*
 * Allocate a new tracked block and read caller-controlled data into it.
 *
 * NOTE: this reproduces the original bug verbatim - it reads
 * (size + 64) bytes into a buffer that is only `size` bytes long,
 * i.e. a heap buffer overflow of up to 64 bytes.
 */
static void alloc_block(unsigned int size, const char *label)
{
    if (block_count > MAX_BLOCKS - 1)
        return;

    block_t *block = malloc(sizeof(block_t));
    void    *buf   = malloc(size);

    if (block == NULL || buf == NULL) {
        free(block);
        free(buf);
        return;
    }

    block->id      = block_count;
    block->size    = size;
    block->on_free = default_on_free;
    strncpy(block->label, label, LABEL_LEN);

    blocks[block_count] = block;
    block_count++;

    printf("[ZION] Block %u allocated (%u bytes) label=%s\n",
           block->id, size, block->label);
    printf("[ZION] Data: ");
    fflush(stdout);

    read(STDIN_FILENO, buf, size + 64); /* bug: should be just `size` */

    free(buf);
}

/*
 * Free a tracked block by index. Invokes the block's stored on_free
 * callback before releasing the block itself, then clears the slot.
 */
static void free_block(unsigned int index)
{
    if (index >= (unsigned int)block_count || blocks[index] == NULL)
        return;

    printf("[ZION] Freeing block %u\n", index);

    blocks[index]->on_free(blocks[index]);
    free(blocks[index]);
    blocks[index] = NULL;
}

static void list_blocks(void)
{
    for (unsigned int i = 0; i < (unsigned int)block_count; i++) {
        block_t *b = blocks[i];
        if (b != NULL) {
            printf("[ZION] Block %u: label=%s size=%u\n",
                   b->id, b->label, b->size);
        }
    }
}

int main(void)
{
    /* Drop-and-restore privilege dance from the original binary
     * (effectively a no-op: sets euid to whatever it already was). */
    uid_t uid = geteuid();
    setreuid(uid, uid);

    print_banner();

    alloc_block(0x40, "ALPHA");
    alloc_block(0x40, "BETA");

    list_blocks();

    free_block(0);
    free_block(1);

    return 0;
}