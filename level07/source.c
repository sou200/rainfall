/*
 * [THE SPRAWL] Packet relay node v4.2
 *
 * Human-readable reconstruction of a Hex-Rays decompilation.
 * Logic, control flow, buffer sizes, and data layout are preserved
 * exactly as they appeared in the decompiled output -- including any
 * quirks/bugs present in the original binary (e.g. the mismatch
 * between the size used for the bounds check and the size actually
 * passed to fread()).
 */

#include <stdio.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Data layout                                                        */
/* ------------------------------------------------------------------ */

/*
 * Wire header format, as parsed by read_header():
 *   offset 0..3  magic       (must be 0xDEADBEEF, i.e. -559038737 signed)
 *   offset 4..5  count       (scanned as %hx into byte offset 4)
 *   offset 6..7  block_size  (scanned as %hx into byte offset 6)
 *
 * The original code treated this as a raw 8-byte blob (__int64 v4) and
 * pulled the two 16-bit fields back out with WORD2()/HIWORD(). Modeling
 * it as a packed struct is equivalent and much easier to read.
 */
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint16_t count;
    uint16_t block_size;
} packet_header_t;
#pragma pack(pop)

#define PACKET_MAGIC   0xDEADBEEFu
#define MAX_PACKETS    32
#define RELAY_BUF_SIZE 72   /* stack buffer used to receive packet data */

/*
 * Log entry for each accepted packet. Matches the original 24-byte
 * stride (3 qwords) used to index into `packets` / qword_404090:
 *   - the 8-byte header (magic/count/block_size)
 *   - a 4-byte checksum (with 4 bytes of padding to keep the 8-byte stride)
 *   - an 8-byte slot that was always zeroed (qword_404090 slot)
 */
typedef struct {
    packet_header_t header;   /* 8 bytes  */
    uint32_t checksum;        /* 4 bytes  */
    uint32_t _pad;            /* 4 bytes  -> 16 bytes so far */
    uint64_t reserved;        /* 8 bytes, always set to 0    -> 24 bytes total */
} packet_log_entry_t;

static packet_log_entry_t packets[MAX_PACKETS];   /* was: packets / unk_404084 / unk_404086 / unk_404088 / qword_404090 */
static int packet_count = 0;                      /* was: packet_count */

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void print_banner(void)
{
    printf("\x1B[0;36m");
    puts("  [THE SPRAWL] BostonAtlantaNewYork online.");
    puts("  [SPRAWL] Packet relay node v4.2");
    printf("\x1B[0m");
}

static unsigned int calc_checksum(const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    unsigned int sum = 0;

    for (size_t i = 0; i < len; ++i)
        sum += p[i];

    return sum;
}

/*
 * NOTE: preserved as-is, including the format-string bug: the user's
 * input buffer `tag` is passed directly to printf() as the format
 * string rather than as an argument (printf(s) in the original).
 */
static void route_tag(void)
{
    char tag[128];

    printf("[SPRAWL] Route tag: ");
    fflush(stdout);

    if (fgets(tag, sizeof(tag), stdin) != NULL) {
        printf("[SPRAWL] Routing via ");
        printf(tag);            /* <-- format string bug, preserved from original */
        fflush(stdout);
    }
}

/*
 * Reads a packet header from stdin in the form: "<magic hex> <count hex> <block_size hex>"
 * Returns 0 on success, -1 on parse failure or bad magic.
 */
static int read_header(packet_header_t *hdr)
{
    printf("[SPRAWL] Header (hex): ");
    fflush(stdout);

    if (scanf("%x %hx %hx", &hdr->magic, &hdr->count, &hdr->block_size) != 3)
        return -1;

    if (hdr->magic != PACKET_MAGIC)
        return -1;

    return 0;
}

/*
 * Reads one packet, checksums it, and logs it.
 *
 * NOTE (preserved from the original): the value that gets bounds-checked
 * against RELAY_BUF_SIZE/2 (0x40) is (uint16_t)(header.count * header.block_size),
 * i.e. a 16-bit truncated product used only for the printed size and the
 * size check. The value actually passed to fread() as the byte count is
 * header.count itself (untruncated by the multiplication), not the
 * checked product. This mismatch is present in the original decompiled
 * logic and has not been "fixed" here.
 */
static void process_packet(void)
{
    packet_header_t hdr;

    if (read_header(&hdr) < 0) {
        puts("[SPRAWL] Invalid header.");
        return;
    }

    uint16_t checked_size = (uint16_t)(hdr.count * hdr.block_size);

    if (checked_size > 0x40) {
        puts("[SPRAWL] Frame too large for relay buffer.");
        return;
    }

    unsigned char buf[RELAY_BUF_SIZE];
    size_t read_len = hdr.count;   /* size actually used for fread(), per original logic */

    printf("[SPRAWL] Transmit (%u bytes): ", checked_size);
    fflush(stdout);

    getchar();                     /* consume leftover newline, as in original */
    fread(buf, 1, read_len, stdin);

    unsigned int checksum = calc_checksum(buf, checked_size);

    if (packet_count <= 31) {
        packets[packet_count].header    = hdr;
        packets[packet_count].checksum  = checksum;
        packets[packet_count].reserved  = 0;
        packet_count++;
    }

    printf("[SPRAWL] Packet %u received. Checksum: %08x\n", packet_count - 1, checksum);
}

static void dump_packets(void)
{
    for (unsigned int i = 0; i < (unsigned int)packet_count; ++i) {
        printf("[SPRAWL] Packet %u: count=%u block_size=%u checksum=%08x\n",
               i,
               packets[i].header.count,
               packets[i].header.block_size,
               packets[i].checksum);
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */

int main(void)
{
    print_banner();
    route_tag();
    process_packet();
    dump_packets();
    return 0;
}