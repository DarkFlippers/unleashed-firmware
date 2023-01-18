/* Schrader variant EG53MA4 TPMS.
 * Usually 443.92 Mhz OOK, 100us pulse len.
 *
 * Preamble: alternating pulse/gap, 100us.
 * Sync (as pulses and gaps): "01100101", already part of the data stream
 * (first nibble) corresponding to 0x4
 *
 * A total of 10 bytes payload, Manchester encoded.
 *
 * 0 = 01
 * 1 = 10
 *
 * Used in certain Open cars and others.
 */

#include "../../app.h"

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    const char* sync_pattern = "010101010101"
                               "01100101";
    uint8_t sync_len = 12 + 8; /* We just use 12 preamble symbols + sync. */
    if(numbits - sync_len + 8 < 8 * 10) return false;

    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Schrader EG53MA4 TPMS preamble+sync found");

    info->start_off = off;
    off += sync_len - 8; /* Skip preamble, not sync that is part of the data. */

    uint8_t raw[10];
    uint32_t decoded = convert_from_line_code(
        raw, sizeof(raw), bits, numbytes, off, "01", "10"); /* Manchester code. */
    FURI_LOG_E(TAG, "Schrader EG53MA4 TPMS decoded bits: %lu", decoded);

    if(decoded < 10 * 8) return false; /* Require the full 10 bytes. */

    /* CRC is just all bytes added mod 256. */
    uint8_t crc = 0;
    for(int j = 0; j < 9; j++) crc += raw[j];
    if(crc != raw[9]) return false; /* Require sane CRC. */

    info->pulses_count = (off + 10 * 8 * 2) - info->start_off;

    /* To convert the raw pressure to kPa, RTL433 uses 2.5, but is likely
     * wrong. Searching on Google for users experimenting with the value
     * reported, the value appears to be 2.75. */
    float kpa = (float)raw[7] * 2.75;
    int temp_f = raw[8];
    int temp_c = (temp_f - 32) * 5 / 9; /* Convert Fahrenheit to Celsius. */

    snprintf(info->name, sizeof(info->name), "%s", "Schrader EG53MA4 TPMS");
    snprintf(
        info->raw,
        sizeof(info->raw),
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        raw[0],
        raw[1],
        raw[2],
        raw[3],
        raw[4],
        raw[5],
        raw[6],
        raw[7],
        raw[8],
        raw[9]);
    snprintf(
        info->info1,
        sizeof(info->info1),
        "Tire ID %02X%02X%02X",
        raw[4],
        raw[5],
        raw[6]); /* Only 28 bits of ID, not 32. */
    snprintf(info->info2, sizeof(info->info2), "Pressure %.2f kpa", (double)kpa);
    snprintf(info->info3, sizeof(info->info3), "Temperature %d C", temp_c);
    return true;
}

ProtoViewDecoder SchraderEG53MA4TPMSDecoder = {"Schrader EG53MA4 TPMS", decode};
