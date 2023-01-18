/* PT/SC remotes. Usually 443.92 Mhz OOK.
 *
 * This line code is used in many remotes such as Princeton chips
 * named PT2262, Silian Microelectronics SC5262 and others.
 * Basically every 4 pulsee represent a bit, where 1000 means 0, and
 * 1110 means 1. Usually we can read 24 bits of data.
 * In this specific implementation we check for a prelude that is
 * 1 bit high, 31 bits low, but the check is relaxed. */

#include "../app.h"

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    if(numbits < 30) return false;
    const char* sync_patterns[3] = {
        "10000000000000000000000000000001", /* 30 zero bits. */
        "100000000000000000000000000000001", /* 31 zero bits. */
        "1000000000000000000000000000000001", /* 32 zero bits. */
    };

    uint32_t off;
    int j;
    for(j = 0; j < 3; j++) {
        off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_patterns[j]);
        if(off != BITMAP_SEEK_NOT_FOUND) break;
    }
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    if(DEBUG_MSG) FURI_LOG_E(TAG, "B4B1 preamble at: %lu", off);
    info->start_off = off;

    // Seek data setction. Why -1? Last bit is data.
    off += strlen(sync_patterns[j]) - 1;

    uint8_t d[3]; /* 24 bits of data. */
    uint32_t decoded = convert_from_line_code(d, sizeof(d), bits, numbytes, off, "1000", "1110");

    if(DEBUG_MSG) FURI_LOG_E(TAG, "B4B1 decoded: %lu", decoded);
    if(decoded < 24) return false;

    off += 24 * 4; // seek to end symbol offset to calculate the length.
    off++; // In this protocol there is a final pulse as terminator.
    info->pulses_count = off - info->start_off;
    snprintf(info->name, PROTOVIEW_MSG_STR_LEN, "PT/SC remote");
    snprintf(info->raw, PROTOVIEW_MSG_STR_LEN, "%02X%02X%02X", d[0], d[1], d[2]);
    return true;
}

ProtoViewDecoder B4B1Decoder = {"B4B1", decode};
