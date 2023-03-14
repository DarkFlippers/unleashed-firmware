/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * PT/SC remotes. Usually 443.92 Mhz OOK.
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

    /* Test different pulse + gap + first byte possibilities. */
    const char* sync_patterns[6] = {
        "100000000000000000000000000000011101", /* 30 times gap + one. */
        "100000000000000000000000000000010001", /* 30 times gap + zero. */
        "1000000000000000000000000000000011101", /* 31 times gap + one. */
        "1000000000000000000000000000000010001", /* 31 times gap + zero. */
        "10000000000000000000000000000000011101", /* 32 times gap + one. */
        "10000000000000000000000000000000010001", /* 32 times gap + zero. */
    };

    uint32_t off;
    int j;
    for(j = 0; j < 3; j++) {
        off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_patterns[j]);
        if(off != BITMAP_SEEK_NOT_FOUND) break;
    }
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    if(DEBUG_MSG) FURI_LOG_E(TAG, "B4B1 preamble id:%d at: %lu", j, off);
    info->start_off = off;

    // Seek data setction. Why -5? Last 5 half-bit-times are data.
    off += strlen(sync_patterns[j]) - 5;

    uint8_t d[3]; /* 24 bits of data. */
    uint32_t decoded = convert_from_line_code(d, sizeof(d), bits, numbytes, off, "1000", "1110");

    if(DEBUG_MSG) FURI_LOG_E(TAG, "B4B1 decoded: %lu", decoded);
    if(decoded < 24) return false;

    off += 24 * 4; // seek to end symbol offset to calculate the length.
    off++; // In this protocol there is a final pulse as terminator.
    info->pulses_count = off - info->start_off;

    fieldset_add_bytes(info->fieldset, "id", d, 5);
    fieldset_add_uint(info->fieldset, "button", d[2] & 0xf, 4);
    return true;
}

/* Give fields and defaults for the signal creator. */
static void get_fields(ProtoViewFieldSet* fieldset) {
    uint8_t default_id[3] = {0xAB, 0xCD, 0xE0};
    fieldset_add_bytes(fieldset, "id", default_id, 5);
    fieldset_add_uint(fieldset, "button", 1, 4);
}

/* Create a signal. */
static void build_message(RawSamplesBuffer* samples, ProtoViewFieldSet* fs) {
    uint32_t te = 334; // Short pulse duration in microseconds.

    // Sync: 1 te pulse, 31 te gap.
    raw_samples_add(samples, true, te);
    raw_samples_add(samples, false, te * 31);

    // ID + button state
    uint8_t data[3];
    memcpy(data, fs->fields[0]->bytes, 3);
    data[2] = (data[2] & 0xF0) | (fs->fields[1]->uvalue & 0xF);
    for(uint32_t j = 0; j < 24; j++) {
        if(bitmap_get(data, sizeof(data), j)) {
            raw_samples_add(samples, true, te * 3);
            raw_samples_add(samples, false, te);
        } else {
            raw_samples_add(samples, true, te);
            raw_samples_add(samples, false, te * 3);
        }
    }

    // Signal terminator. Just a single short pulse.
    raw_samples_add(samples, true, te);
}

ProtoViewDecoder B4B1Decoder = {
    .name = "PT/SC remote",
    .decode = decode,
    .get_fields = get_fields,
    .build_message = build_message};
