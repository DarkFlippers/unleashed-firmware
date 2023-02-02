/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * Citroen TPMS. Usually 443.92 Mhz FSK.
 *
 * Preamble of ~14 high/low 52 us pulses
 * Sync of high 100us pulse then 50us low
 * Then Manchester bits, 10 bytes total.
 * Simple XOR checksum. */

#include "../../app.h"

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    /* We consider a preamble of 17 symbols. They are more, but the decoding
     * is more likely to happen if we don't pretend to receive from the
     * very start of the message. */
    uint32_t sync_len = 17;
    const char* sync_pattern = "10101010101010110";
    if(numbits - sync_len < 8 * 10) return false; /* Expect 10 bytes. */

    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Renault TPMS preamble+sync found");

    info->start_off = off;
    off += sync_len; /* Skip preamble + sync. */

    uint8_t raw[10];
    uint32_t decoded = convert_from_line_code(
        raw, sizeof(raw), bits, numbytes, off, "01", "10"); /* Manchester. */
    FURI_LOG_E(TAG, "Citroen TPMS decoded bits: %lu", decoded);

    if(decoded < 8 * 10) return false; /* Require the full 10 bytes. */

    /* Check the CRC. It's a simple XOR of bytes 1-9, the first byte
     * is not included. The meaning of the first byte is unknown and
     * we don't display it. */
    uint8_t crc = 0;
    for(int j = 1; j < 10; j++) crc ^= raw[j];
    if(crc != 0) return false; /* Require sane checksum. */

    info->pulses_count = (off + 8 * 10 * 2) - info->start_off;

    int repeat = raw[5] & 0xf;
    float kpa = (float)raw[6] * 1.364;
    int temp = raw[7] - 50;
    int battery = raw[8]; /* This may be the battery. It's not clear. */

    fieldset_add_bytes(info->fieldset, "Tire ID", raw + 1, 4 * 2);
    fieldset_add_float(info->fieldset, "Pressure kpa", kpa, 2);
    fieldset_add_int(info->fieldset, "Temperature C", temp, 8);
    fieldset_add_uint(info->fieldset, "Repeat", repeat, 4);
    fieldset_add_uint(info->fieldset, "Battery", battery, 8);
    return true;
}

ProtoViewDecoder CitroenTPMSDecoder =
    {.name = "Citroen TPMS", .decode = decode, .get_fields = NULL, .build_message = NULL};
