/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * Ford tires TPMS. Usually 443.92 Mhz FSK (in Europe).
 *
 * 52 us short pules
 * Preamble: 0101010101010101010101010101
 * Sync: 0110 (that is 52 us gap + 104 us pulse + 52 us gap)
 * Data: 8 bytes Manchester encoded
 * 01 = zero
 * 10 = one
 */

#include "../../app.h"

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    const char* sync_pattern = "010101010101"
                               "0110";
    uint8_t sync_len = 12 + 4; /* We just use 12 preamble symbols + sync. */
    if(numbits - sync_len < 8 * 8) return false;

    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Fort TPMS preamble+sync found");

    info->start_off = off;
    off += sync_len; /* Skip preamble and sync. */

    uint8_t raw[8];
    uint32_t decoded = convert_from_line_code(
        raw, sizeof(raw), bits, numbytes, off, "01", "10"); /* Manchester. */
    FURI_LOG_E(TAG, "Ford TPMS decoded bits: %lu", decoded);

    if(decoded < 8 * 8) return false; /* Require the full 8 bytes. */

    /* CRC is just the sum of the first 7 bytes MOD 256. */
    uint8_t crc = 0;
    for(int j = 0; j < 7; j++) crc += raw[j];
    if(crc != raw[7]) return false; /* Require sane CRC. */

    info->pulses_count = (off + 8 * 8 * 2) - info->start_off;

    float psi = 0.25 * (((raw[6] & 0x20) << 3) | raw[4]);

    /* Temperature apperas to be valid only if the most significant
     * bit of the value is not set. Otherwise its meaning is unknown.
     * Likely useful to alternatively send temperature or other info. */
    int temp = raw[5] & 0x80 ? 0 : raw[5] - 56;
    int flags = raw[5] & 0x7f;
    int car_moving = (raw[6] & 0x44) == 0x44;

    fieldset_add_bytes(info->fieldset, "Tire ID", raw, 4 * 2);
    fieldset_add_float(info->fieldset, "Pressure psi", psi, 2);
    fieldset_add_int(info->fieldset, "Temperature C", temp, 8);
    fieldset_add_hex(info->fieldset, "Flags", flags, 7);
    fieldset_add_uint(info->fieldset, "Moving", car_moving, 1);
    return true;
}

ProtoViewDecoder FordTPMSDecoder =
    {.name = "Ford TPMS", .decode = decode, .get_fields = NULL, .build_message = NULL};
