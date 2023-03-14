/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license.
 *
 * Renault tires TPMS. Usually 443.92 Mhz FSK.
 *
 * Preamble + sync + Manchester bits. ~48us short pulse.
 * 9 Bytes in total not counting the preamble. */

#include "../../app.h"

#define USE_TEST_VECTOR 0
static const char* test_vector =
    "...01010101010101010110" // Preamble + sync

    /* The following is Marshal encoded, so each two characters are
     * actaully one bit. 01 = 0, 10 = 1. */
    "010110010110" // Flags.
    "10011001101010011001" // Pressure, multiply by 0.75 to obtain kpa.
    // 244 kpa here.
    "1010010110011010" // Temperature, subtract 30 to obtain celsius. 22C here.
    "1001010101101001"
    "0101100110010101"
    "1001010101100110" // Tire ID. 0x7AD779 here.
    "0101010101010101"
    "0101010101010101" // Two FF bytes (usually). Unknown.
    "0110010101010101"; // CRC8 with (poly 7, initialization 0).

static bool decode(uint8_t* bits, uint32_t numbytes, uint32_t numbits, ProtoViewMsgInfo* info) {
    if(USE_TEST_VECTOR) { /* Test vector to check that decoding works. */
        bitmap_set_pattern(bits, numbytes, 0, test_vector);
        numbits = strlen(test_vector);
    }

    if(numbits - 12 < 9 * 8) return false;

    const char* sync_pattern = "01010101010101010110";
    uint64_t off = bitmap_seek_bits(bits, numbytes, 0, numbits, sync_pattern);
    if(off == BITMAP_SEEK_NOT_FOUND) return false;
    FURI_LOG_E(TAG, "Renault TPMS preamble+sync found");

    info->start_off = off;
    off += 20; /* Skip preamble. */

    uint8_t raw[9];
    uint32_t decoded = convert_from_line_code(
        raw, sizeof(raw), bits, numbytes, off, "01", "10"); /* Manchester. */
    FURI_LOG_E(TAG, "Renault TPMS decoded bits: %lu", decoded);

    if(decoded < 8 * 9) return false; /* Require the full 9 bytes. */
    if(crc8(raw, 8, 0, 7) != raw[8]) return false; /* Require sane CRC. */

    info->pulses_count = (off + 8 * 9 * 2) - info->start_off;

    uint8_t flags = raw[0] >> 2;
    float kpa = 0.75 * ((uint32_t)((raw[0] & 3) << 8) | raw[1]);
    int temp = raw[2] - 30;

    fieldset_add_bytes(info->fieldset, "Tire ID", raw + 3, 3 * 2);
    fieldset_add_float(info->fieldset, "Pressure kpa", kpa, 2);
    fieldset_add_int(info->fieldset, "Temperature C", temp, 8);
    fieldset_add_hex(info->fieldset, "Flags", flags, 6);
    fieldset_add_bytes(info->fieldset, "Unknown1", raw + 6, 2);
    fieldset_add_bytes(info->fieldset, "Unknown2", raw + 7, 2);
    return true;
}

/* Give fields and defaults for the signal creator. */
static void get_fields(ProtoViewFieldSet* fieldset) {
    uint8_t default_id[3] = {0xAB, 0xCD, 0xEF};
    fieldset_add_bytes(fieldset, "Tire ID", default_id, 3 * 2);
    fieldset_add_float(fieldset, "Pressure kpa", 123, 2);
    fieldset_add_int(fieldset, "Temperature C", 20, 8);
    // We don't know what flags are, but 1B is a common value.
    fieldset_add_hex(fieldset, "Flags", 0x1b, 6);
    fieldset_add_bytes(fieldset, "Unknown1", (uint8_t*)"\xff", 2);
    fieldset_add_bytes(fieldset, "Unknown2", (uint8_t*)"\xff", 2);
}

/* Create a Renault TPMS signal, according to the fields provided. */
static void build_message(RawSamplesBuffer* samples, ProtoViewFieldSet* fieldset) {
    uint32_t te = 50; // Short pulse duration in microseconds.

    // Preamble + sync
    const char* psync = "01010101010101010101010101010110";
    const char* p = psync;
    while(*p) {
        raw_samples_add_or_update(samples, *p == '1', te);
        p++;
    }

    // Data, 9 bytes
    uint8_t data[9] = {0};
    unsigned int raw_pressure = fieldset->fields[1]->fvalue * 4 / 3;
    data[0] = fieldset->fields[3]->uvalue << 2; // Flags
    data[0] |= (raw_pressure >> 8) & 3; // Pressure kpa high 2 bits
    data[1] = raw_pressure & 0xff; // Pressure kpa low 8 bits
    data[2] = fieldset->fields[2]->value + 30; // Temperature C
    memcpy(data + 3, fieldset->fields[0]->bytes, 6); // ID, 24 bits.
    data[6] = fieldset->fields[4]->bytes[0]; // Unknown 1
    data[7] = fieldset->fields[5]->bytes[0]; // Unknown 2
    data[8] = crc8(data, 8, 0, 7);

    // Generate Manchester code for each bit
    for(uint32_t j = 0; j < 9 * 8; j++) {
        if(bitmap_get(data, sizeof(data), j)) {
            raw_samples_add_or_update(samples, true, te);
            raw_samples_add_or_update(samples, false, te);
        } else {
            raw_samples_add_or_update(samples, false, te);
            raw_samples_add_or_update(samples, true, te);
        }
    }
}

ProtoViewDecoder RenaultTPMSDecoder = {
    .name = "Renault TPMS",
    .decode = decode,
    .get_fields = get_fields,
    .build_message = build_message};
