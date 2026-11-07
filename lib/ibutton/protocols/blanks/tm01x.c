#include <core/kernel.h>
#include <core/check.h>
#include <core/core_defines.h>
#include <one_wire/one_wire_host.h>
#include <bit_lib/bit_lib.h>
#include <string.h>
#include "tm01x.h"

// Commands for TM01x
#define TM01X_CMD_WRITE_FLAG 0xC1
#define TM01X_CMD_WRITE_ROM  0xC5
#define TM01X_CMD_READ_ROM   0x33

#define TM01X_CMD_FINALIZE_CYFRAL  0xCA
#define TM01X_CMD_FINALIZE_METAKOM 0xCB

// Timings measured with a logic analyzer on a TM01A blank, see the write-protocol
// breakdown at
// https://domofon-master2009.narod.ru/publ/logicheskij_analizator_i_primer_ego_primenenija_dlja_issledovanija_protokola_zapisi_zagotovki_tm_01/1-1-0-16

// 5 ms pause after each bit to let the blank complete the write
#define TM01X_BIT_DELAY_US        5000
// 5 ms pause after the write flag is set
#define TM01X_WRITE_FLAG_DELAY_US 5000
// Pause after the blank is switched into the target mode
#define TM01X_FINALIZE_DELAY_US   10000

/** Data read back from a blank during write verification. */
typedef struct {
    /** true if the data was actually read from the blank. */
    bool valid;
    /** Raw 1-Wire read result, as returned by the blank. */
    uint8_t data[TM01X_ROM_SIZE];
} Tm01xReadback;

static const uint8_t tm01x_finalize_command[Tm01xKeyModeMax] = {
    [Tm01xKeyModeCyfral] = TM01X_CMD_FINALIZE_CYFRAL,
    [Tm01xKeyModeMetakom] = TM01X_CMD_FINALIZE_METAKOM,
};

// Keyed by Tm01xKeyMode, so a new mode cannot silently finalize as Metakom.
_Static_assert(
    sizeof(tm01x_finalize_command) == Tm01xKeyModeMax,
    "Every Tm01xKeyMode needs a finalize command");

/** Get a bit of the write stream: misc-protocol data is programmed inverted, MSB first. */
static bool tm01x_protocol_misc_get_bit(const uint8_t rom[TM01X_ROM_SIZE], size_t bit_index) {
    return !bit_lib_get_bit(rom, bit_index);
}

uint32_t tm01x_bit_reverse32(uint32_t value) {
    const uint32_t low = bit_lib_reverse_16_fast((uint16_t)value);
    const uint32_t high = bit_lib_reverse_16_fast((uint16_t)(value >> 16));

    return (low << 16) | high;
}

void tm01x_protocol_misc_make_image(
    uint8_t rom[TM01X_ROM_SIZE],
    uint8_t prefix,
    uint32_t wire_bits) {
    // 36-bit frame: 4-bit prefix, then 32 data bits, most significant first
    rom[0] = (uint8_t)(((prefix & 0x0F) << 4) | (wire_bits >> 28));
    rom[1] = (uint8_t)(wire_bits >> 20);
    rom[2] = (uint8_t)(wire_bits >> 12);
    rom[3] = (uint8_t)(wire_bits >> 4);
    rom[4] = (uint8_t)((wire_bits & 0x0F) << 4);
    rom[5] = 0;
    rom[6] = 0;
    rom[7] = 0;
}

/**
 * Convert ROM data read back from a blank into the representation
 * tm01x_protocol_misc_make_image() produces.
 *
 * Only the inversion is a chip convention: the frame is kept in the order it was
 * shifted in. The per-byte reversal comes from the two transfer directions - the
 * write path shifts the image out MSB-first, while onewire_host_read() assembles
 * each byte LSB-first - so a plain read yields inverted, bit-reversed bytes. This
 * function restores the original image in place.
 */
static void tm01x_protocol_misc_normalize_read(uint8_t rom[TM01X_ROM_SIZE]) {
    for(size_t i = 0; i < TM01X_PROTOCOL_MISC_DATA_BYTES; ++i) {
        // Inverted by the chip, and reversed per byte because the write path shifts
        // the image out MSB-first while onewire_host_read() assembles bytes LSB-first
        rom[i] = (uint8_t)~bit_lib_reverse_8_fast(rom[i]);
    }

    // Only the upper nibble of the last byte is programmed
    rom[4] &= 0xF0;
    rom[5] = 0;
    rom[6] = 0;
    rom[7] = 0;
}

static void tm01x_write_byte(OneWireHost* host, uint8_t value) {
    // Dallas data is programmed least significant bit first, while
    // bit_lib_get_bit() counts bits from the most significant one
    for(int bit_index = 7; bit_index >= 0; --bit_index) {
        onewire_host_write_bit(host, bit_lib_get_bit(&value, bit_index));
        furi_delay_us(TM01X_BIT_DELAY_US);
    }
}

// Helper function to read and verify written data
static bool
    tm01x_read_and_verify_dallas(OneWireHost* host, const uint8_t* data, size_t data_size) {
    if(!onewire_host_reset(host)) {
        return false;
    }

    onewire_host_write(host, TM01X_CMD_READ_ROM);

    for(size_t i = 0; i < data_size; ++i) {
        if(data[i] != onewire_host_read(host)) {
            return false;
        }
    }

    return true;
}

static bool tm01x_read_and_verify_protocol_misc(
    OneWireHost* host,
    const uint8_t expected[TM01X_ROM_SIZE],
    Tm01xReadback* readback) {
    readback->valid = false;

    if(!onewire_host_reset(host)) {
        return false;
    }

    onewire_host_write(host, TM01X_CMD_READ_ROM);
    onewire_host_read_bytes(host, readback->data, TM01X_ROM_SIZE);
    readback->valid = true;

    uint8_t normalized[TM01X_ROM_SIZE];
    memcpy(normalized, readback->data, sizeof(normalized));
    tm01x_protocol_misc_normalize_read(normalized);

    return memcmp(normalized, expected, sizeof(normalized)) == 0;
}

/** Set the TM01x timings and arm the blank for a ROM write. */
static bool tm01x_begin_write(OneWireHost* host) {
    onewire_host_set_timings_tm01x(host);

    if(!onewire_host_reset(host)) {
        return false;
    }
    onewire_host_write(host, TM01X_CMD_WRITE_FLAG);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_WRITE_FLAG_DELAY_US);

    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_ROM);

    return true;
}

bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size) {
    if(!tm01x_begin_write(host)) {
        return false;
    }

    for(size_t i = 0; i < data_size; ++i) {
        tm01x_write_byte(host, data[i]);
    }

    return tm01x_read_and_verify_dallas(host, data, data_size);
}

bool tm01x_write_protocol_misc(
    OneWireHost* host,
    Tm01xKeyMode mode,
    const uint8_t rom[TM01X_ROM_SIZE]) {
    furi_check(mode == Tm01xKeyModeCyfral || mode == Tm01xKeyModeMetakom);

    if(!tm01x_begin_write(host)) {
        return false;
    }

    // Write the 36-bit image
    for(size_t bit_index = 0; bit_index < TM01X_PROTOCOL_MISC_BIT_COUNT; ++bit_index) {
        onewire_host_write_bit(host, tm01x_protocol_misc_get_bit(rom, bit_index));
        furi_delay_us(TM01X_BIT_DELAY_US);
    }

    // A finalized blank no longer answers 0x33, so the data has to be verified
    // before the mode is switched
    Tm01xReadback readback = {0};
    if(!tm01x_read_and_verify_protocol_misc(host, rom, &readback)) {
        return false;
    }

    // Switch the blank into the target mode
    onewire_host_reset(host);
    onewire_host_write(host, tm01x_finalize_command[mode]);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_FINALIZE_DELAY_US);

    return true;
}
