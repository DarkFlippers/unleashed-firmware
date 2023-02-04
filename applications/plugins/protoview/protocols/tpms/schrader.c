/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * Schrader TPMS. Usually 443.92 Mhz OOK, 120us pulse len.
 *
 * 500us high pulse + Preamble + Manchester coded bits where:
 * 1 = 10
 * 0 = 01
 *
 * 60 bits of data total (first 4 nibbles is the preamble, 0xF).
 *
 * Used in FIAT-Chrysler, Mercedes, ... */

#include "../../app.h"

#define USE_TEST_VECTOR 0
static const char* test_vector =
    "000000111101010101011010010110010110101001010110100110011001100101010101011010100110100110011010101010101010101010101010101010101010101010101010";

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    if(USE_TEST_VECTOR) { /* Test vector to check that decoding works. */
        bitmap_set_pattern(bits, numbytes, 0, test_vector);
        numbits = strlen(test_vector);
    }

    if(numbits < 64) return false; /* Preamble + data. */

    const char* sync_pattern = "1111010101"
                               "01011010";
    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Schrader TPMS gap+preamble found");

    info->start_off = off;
    off += 10; /* Skip just the long pulse and the first 3 bits of sync, so
                  that we have the first byte of data with the sync nibble
                  0011 = 0x3. */

    uint8_t raw[8];
    uint8_t id[4];
    uint32_t decoded = convert_from_line_code(
        raw, sizeof(raw), bits, numbytes, off, "01", "10"); /* Manchester code. */
    FURI_LOG_E(TAG, "Schrader TPMS decoded bits: %lu", decoded);

    if(decoded < 64) return false; /* Require the full 8 bytes. */

    raw[0] |= 0xf0; // Fix the preamble nibble for checksum computation.
    uint8_t cksum = crc8(raw, sizeof(raw) - 1, 0xf0, 0x7);
    if(cksum != raw[7]) {
        FURI_LOG_E(TAG, "Schrader TPMS checksum mismatch");
        return false;
    }

    info->pulses_count = (off + 8 * 8 * 2) - info->start_off;

    float kpa = (float)raw[5] * 2.5;
    int temp = raw[6] - 50;
    id[0] = raw[1] & 7;
    id[1] = raw[2];
    id[2] = raw[3];
    id[3] = raw[4];

    fieldset_add_bytes(info->fieldset, "Tire ID", id, 4 * 2);
    fieldset_add_float(info->fieldset, "Pressure kpa", kpa, 2);
    fieldset_add_int(info->fieldset, "Temperature C", temp, 8);
    return true;
}

ProtoViewDecoder SchraderTPMSDecoder =
    {.name = "Schrader TPMS", .decode = decode, .get_fields = NULL, .build_message = NULL};
