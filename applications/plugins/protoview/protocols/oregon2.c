/* Oregon remote termometers. Usually 443.92 Mhz OOK.
 *
 * The protocol is described here:
 * https://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
 * This implementation is not very complete. */

#include "../app.h"

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    if(numbits < 32) return false;
    const char* sync_pattern = "01100110"
                               "01100110"
                               "10010110"
                               "10010110";
    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Oregon2 preamble+sync found");

    info->start_off = off;
    off += 32; /* Skip preamble. */

    uint8_t buffer[8], raw[8] = {0};
    uint32_t decoded =
        convert_from_line_code(buffer, sizeof(buffer), bits, numbytes, off, "1001", "0110");
    FURI_LOG_E(TAG, "Oregon2 decoded bits: %lu", decoded);

    if(decoded < 11 * 4) return false; /* Minimum len to extract some data. */
    info->pulses_count = (off + 11 * 4 * 4) - info->start_off;

    char temp[3] = {0}, deviceid[2] = {0}, hum[2] = {0};
    for(int j = 0; j < 64; j += 4) {
        uint8_t nib[1];
        nib[0] =
            (bitmap_get(buffer, 8, j + 0) | bitmap_get(buffer, 8, j + 1) << 1 |
             bitmap_get(buffer, 8, j + 2) << 2 | bitmap_get(buffer, 8, j + 3) << 3);
        if(DEBUG_MSG) FURI_LOG_E(TAG, "Not inverted nibble[%d]: %x", j / 4, (unsigned int)nib[0]);
        raw[j / 8] |= nib[0] << (4 - (j % 4));
        switch(j / 4) {
        case 1:
            deviceid[0] |= nib[0];
            break;
        case 0:
            deviceid[0] |= nib[0] << 4;
            break;
        case 3:
            deviceid[1] |= nib[0];
            break;
        case 2:
            deviceid[1] |= nib[0] << 4;
            break;
        case 10:
            temp[0] = nib[0];
            break;
        /* Fixme: take the temperature sign from nibble 11. */
        case 9:
            temp[1] = nib[0];
            break;
        case 8:
            temp[2] = nib[0];
            break;
        case 13:
            hum[0] = nib[0];
            break;
        case 12:
            hum[1] = nib[0];
            break;
        }
    }

    snprintf(info->name, sizeof(info->name), "%s", "Oregon v2.1");
    /* The following line crashes the Flipper because of broken
     * snprintf() implementation. */
    snprintf(
        info->raw,
        sizeof(info->raw),
        "%02X%02X%02X%02X%02X%02X%02X%02X",
        raw[0],
        raw[1],
        raw[2],
        raw[3],
        raw[4],
        raw[5],
        raw[6],
        raw[7]);
    snprintf(info->info1, sizeof(info->info1), "Sensor ID %02X%02X", deviceid[0], deviceid[1]);
    snprintf(info->info2, sizeof(info->info2), "Temperature %d%d.%d", temp[0], temp[1], temp[2]);
    snprintf(info->info3, sizeof(info->info3), "Humidity %d%d", hum[0], hum[1]);
    return true;
}

ProtoViewDecoder Oregon2Decoder = {"Oregon2", decode};
