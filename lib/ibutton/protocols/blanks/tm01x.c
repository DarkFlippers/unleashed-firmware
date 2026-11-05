#include <core/kernel.h>
#include <one_wire/one_wire_host.h>
#include <string.h>
#include "tm01x.h"

// Commands for TM01x
#define TM01X_CMD_WRITE_FLAG 0xC1
#define TM01X_CMD_WRITE_ROM  0xC5
#define TM01X_CMD_READ_ROM   0x33

#define TM01X_CMD_FINALIZE_CYFRAL  0xCA
#define TM01X_CMD_FINALIZE_METAKOM 0xCB

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

bool tm01x_dallas_get_bit(uint8_t value, size_t bit_index) {
    return (value >> bit_index) & 0x01;
}

bool tm01x_protocol_misc_get_bit(const uint8_t rom[TM01X_ROM_SIZE], size_t bit_index) {
    const size_t byte_index = bit_index / 8;
    const size_t bit_in_byte = 7 - (bit_index % 8);

    // Misc-protocol data is programmed inverted
    return ((uint8_t)~rom[byte_index] >> bit_in_byte) & 0x01;
}

uint32_t tm01x_bit_reverse32(uint32_t value) {
    uint32_t result = 0;

    for(size_t bit = 0; bit < 32; ++bit) {
        result |= ((value >> bit) & 0x01) << (31 - bit);
    }

    return result;
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

void tm01x_protocol_misc_normalize_read(uint8_t rom[TM01X_ROM_SIZE]) {
    for(size_t i = 0; i < TM01X_PROTOCOL_MISC_DATA_BYTES; ++i) {
        const uint8_t value = (uint8_t)~rom[i];
        uint8_t reversed = 0;

        for(size_t bit = 0; bit < 8; ++bit) {
            reversed |= ((value >> bit) & 0x01) << (7 - bit);
        }

        rom[i] = reversed;
    }

    // Only the upper nibble of the last byte is programmed
    rom[4] &= 0xF0;
    rom[5] = 0;
    rom[6] = 0;
    rom[7] = 0;
}

static void tm01x_write_byte(OneWireHost* host, uint8_t value) {
    for(size_t bit_index = 0; bit_index < 8; ++bit_index) {
        onewire_host_write_bit(host, tm01x_dallas_get_bit(value, bit_index));
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

bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size) {
    // Set TM01x specific timings
    onewire_host_set_timings_tm01x(host);

    // Write sequence
    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_FLAG);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_WRITE_FLAG_DELAY_US);

    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_ROM);

    for(size_t i = 0; i < data_size; ++i) {
        tm01x_write_byte(host, data[i]);
    }

    return tm01x_read_and_verify_dallas(host, data, data_size);
}

bool tm01x_write_protocol_misc(
    OneWireHost* host,
    Tm01xKeyMode mode,
    const uint8_t rom[TM01X_ROM_SIZE]) {
    // Set TM01x specific timings
    onewire_host_set_timings_tm01x(host);

    // Enable writing
    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_FLAG);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_WRITE_FLAG_DELAY_US);

    // Write the 36-bit image
    onewire_host_reset(host);
    onewire_host_write(host, TM01X_CMD_WRITE_ROM);

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
    onewire_host_write(
        host,
        (mode == Tm01xKeyModeCyfral) ? TM01X_CMD_FINALIZE_CYFRAL : TM01X_CMD_FINALIZE_METAKOM);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_FINALIZE_DELAY_US);

    return true;
}
