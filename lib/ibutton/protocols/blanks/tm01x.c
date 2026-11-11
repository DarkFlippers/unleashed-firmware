#include <core/kernel.h>
#include <core/check.h>
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

#define TM01X_ROM_SIZE 8U

// Number of bits programmed into a Cyfral or Metakom blank
#define TM01X_FRAME_BIT_COUNT 36U

// Number of ROM bytes that carry the data of a Cyfral or Metakom frame
#define TM01X_FRAME_DATA_BYTES 5U

// Timings measured with a logic analyzer on a TM01A blank, see the write-protocol
// breakdown at
// https://domofon-master2009.narod.ru/publ/logicheskij_analizator_i_primer_ego_primenenija_dlja_issledovanija_protokola_zapisi_zagotovki_tm_01/1-1-0-16

// 5 ms pause after each bit to let the blank complete the write
#define TM01X_BIT_DELAY_US        5000
#define TM01X_WRITE_FLAG_DELAY_US 5000
// Pause after the blank is switched into the target mode
#define TM01X_FINALIZE_DELAY_US   10000

/** 36-bit frames a TM01x blank accepts. */
typedef enum {
    Tm01xFrameCyfral,
    Tm01xFrameMetakom,

    Tm01xFrameMax,
} Tm01xFrame;

typedef struct {
    uint8_t frame_prefix;
    uint8_t finalize_command;
} Tm01xFrameInfo;

static const Tm01xFrameInfo tm01x_frames[] = {
    [Tm01xFrameCyfral] = {.frame_prefix = 0x01U, .finalize_command = TM01X_CMD_FINALIZE_CYFRAL},
    [Tm01xFrameMetakom] = {.frame_prefix = 0x02U, .finalize_command = TM01X_CMD_FINALIZE_METAKOM},
};

_Static_assert(
    sizeof(tm01x_frames) / sizeof(tm01x_frames[0]) == Tm01xFrameMax,
    "Every Tm01xFrame needs a frame prefix and a finalize command");

// The check above only proves that the last row is present. A frame inserted into the
// middle of the enum would leave a zeroed row and still pass, so the count is pinned too.
_Static_assert(
    Tm01xFrameMax == 2,
    "A frame added to Tm01xFrame needs a row in tm01x_frames and an update here");

/** Get a bit of the write stream: Cyfral and Metakom data is programmed inverted, MSB first. */
static bool tm01x_frame_bit(const uint8_t rom[TM01X_ROM_SIZE], size_t bit_index) {
    return !bit_lib_get_bit(rom, bit_index);
}

/**
 * Lay out a Cyfral or Metakom code in the ROM image a TM01x blank is written with.
 *
 * The frame is 36 bits: a 4-bit prefix followed by the 32 code bits, most
 * significant bit first. The remaining bits are not programmed and stay zero.
 */
static void tm01x_make_image(uint8_t rom[TM01X_ROM_SIZE], uint8_t prefix, uint32_t code) {
    rom[0] = (uint8_t)(((prefix & 0x0F) << 4) | (code >> 28));
    rom[1] = (uint8_t)(code >> 20);
    rom[2] = (uint8_t)(code >> 12);
    rom[3] = (uint8_t)(code >> 4);
    rom[4] = (uint8_t)((code & 0x0F) << 4);
    rom[5] = 0;
    rom[6] = 0;
    rom[7] = 0;
}

/**
 * Convert ROM data read back from a blank into the representation
 * tm01x_make_image() produces.
 *
 * Only the inversion is a chip convention: the frame is kept in the order it was
 * shifted in. The per-byte reversal comes from the two transfer directions - the
 * write path shifts the image out MSB-first, while onewire_host_read() assembles
 * each byte LSB-first - so a plain read yields inverted, bit-reversed bytes. This
 * function restores the original image in place.
 */
static void tm01x_normalize_read(uint8_t rom[TM01X_ROM_SIZE]) {
    for(size_t i = 0; i < TM01X_FRAME_DATA_BYTES; ++i) {
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

static bool
    tm01x_read_and_verify_frame(OneWireHost* host, const uint8_t expected[TM01X_ROM_SIZE]) {
    if(!onewire_host_reset(host)) {
        return false;
    }

    onewire_host_write(host, TM01X_CMD_READ_ROM);

    uint8_t actual[TM01X_ROM_SIZE];
    onewire_host_read_bytes(host, actual, TM01X_ROM_SIZE);
    tm01x_normalize_read(actual);

    return memcmp(actual, expected, sizeof(actual)) == 0;
}

static bool tm01x_write_frame(OneWireHost* host, Tm01xFrame frame, uint32_t code) {
    furi_check(frame < Tm01xFrameMax);

    uint8_t rom[TM01X_ROM_SIZE];
    tm01x_make_image(rom, tm01x_frames[frame].frame_prefix, code);

    if(!tm01x_begin_write(host)) {
        return false;
    }

    for(size_t bit_index = 0; bit_index < TM01X_FRAME_BIT_COUNT; ++bit_index) {
        onewire_host_write_bit(host, tm01x_frame_bit(rom, bit_index));
        furi_delay_us(TM01X_BIT_DELAY_US);
    }

    // A finalized blank no longer answers 0x33, so the data has to be verified
    // before the blank is finalized
    if(!tm01x_read_and_verify_frame(host, rom)) {
        return false;
    }

    // Finalizing is one-way: a finalized blank cannot be written again, not even as a Dallas key
    onewire_host_reset(host);
    onewire_host_write(host, tm01x_frames[frame].finalize_command);
    onewire_host_write_bit(host, true);
    furi_delay_us(TM01X_FINALIZE_DELAY_US);

    return true;
}

bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size) {
    furi_check(data_size == TM01X_ROM_SIZE);

    if(!tm01x_begin_write(host)) {
        return false;
    }

    for(size_t i = 0; i < data_size; ++i) {
        tm01x_write_byte(host, data[i]);
    }

    return tm01x_read_and_verify_dallas(host, data, data_size);
}

bool tm01x_write_cyfral(OneWireHost* host, uint32_t code) {
    return tm01x_write_frame(host, Tm01xFrameCyfral, code);
}

bool tm01x_write_metakom(OneWireHost* host, uint32_t code) {
    return tm01x_write_frame(host, Tm01xFrameMetakom, code);
}
