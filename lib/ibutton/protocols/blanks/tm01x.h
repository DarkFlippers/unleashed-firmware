#pragma once

#include <core/kernel.h>
#include <one_wire/one_wire_host.h>

/** Size of the ROM data block used by all TM01x write routines, in bytes. */
#define TM01X_ROM_SIZE                 8U
/** Number of bits actually programmed into a misc-protocol blank (Cyfral/Metakom). */
#define TM01X_PROTOCOL_MISC_BIT_COUNT  36U
/** Number of ROM bytes that carry meaningful data in a misc protocol. */
#define TM01X_PROTOCOL_MISC_DATA_BYTES 5U

/** Key mode a TM01x blank can be finalized into. */
typedef enum {
    Tm01xKeyModeCyfral,
    Tm01xKeyModeMetakom,
} Tm01xKeyMode;

/**
 * Get the value of a bit in a Dallas write stream.
 *
 * Dallas ROM data is programmed least significant bit first and is not
 * inverted.
 *
 * @param value      Byte being programmed.
 * @param bit_index  Bit position, 0 to 7.
 *
 * @returns Bit value to shift out.
 */
bool tm01x_dallas_get_bit(uint8_t value, size_t bit_index);

/**
 * Get the value of a bit in a misc-protocol write stream.
 *
 * Misc-protocol images are programmed inverted, most significant bit first,
 * and only the first TM01X_PROTOCOL_MISC_BIT_COUNT bits are sent.
 *
 * @param rom        ROM image built by tm01x_protocol_misc_make_image().
 * @param bit_index  Bit position, 0 to TM01X_PROTOCOL_MISC_BIT_COUNT - 1.
 *
 * @returns Bit value to shift out.
 */
bool tm01x_protocol_misc_get_bit(const uint8_t rom[TM01X_ROM_SIZE], size_t bit_index);

/**
 * Reverse the bit order of a 32-bit word.
 *
 * Protocols differ in which end of a word is sent first, so converting
 * between their conventions is an explicit step rather than an implicit
 * detail of the write routine.
 *
 * @param value  Value to reverse.
 *
 * @returns Value with bit 0 and bit 31 swapped, and so on.
 */
uint32_t tm01x_bit_reverse32(uint32_t value);

/**
 * Lay out a 36-bit misc-protocol code in the ROM image a TM01x blank is
 * written with.
 *
 * The code is stored as a 4-bit prefix followed by 32 data bits, most
 * significant bit first. Bits beyond TM01X_PROTOCOL_MISC_BIT_COUNT are never
 * programmed and are left zero.
 *
 * @param[out] rom        Buffer of exactly TM01X_ROM_SIZE bytes.
 * @param[in]  prefix     Leading 4 bits of the code frame, 0 to 0x0F.
 * @param[in]  wire_bits  32 data bits, most significant bit sent first.
 */
void tm01x_protocol_misc_make_image(
    uint8_t rom[TM01X_ROM_SIZE],
    uint8_t prefix,
    uint32_t wire_bits);

/**
 * Convert ROM data read back from a TM01x blank in a misc protocol into the
 * representation produced by tm01x_protocol_misc_make_image().
 *
 * Misc-protocol images are stored inverted and in reverse bit order, so a
 * plain 1-Wire read yields inverted, bit-reversed bytes. This function
 * restores the original image in place.
 *
 * @param[in,out] rom  Buffer of exactly TM01X_ROM_SIZE bytes.
 */
void tm01x_protocol_misc_normalize_read(uint8_t rom[TM01X_ROM_SIZE]);

/**
 * Write a Dallas (DS1990) ROM code into a TM01x blank.
 *
 * @param host       OneWire host instance.
 * @param data       ROM data, exactly TM01X_ROM_SIZE bytes.
 * @param data_size  Must be equal to TM01X_ROM_SIZE.
 *
 * @returns true if the data read back from the blank matches what was written.
 */
bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size);

/**
 * Write a 36-bit misc-protocol code into a TM01x blank and finalize it.
 *
 * The image is written, read back and verified before the blank is switched
 * into the requested mode: a blank that failed verification is left in Dallas
 * mode, so it can be written again.
 *
 * @param host       OneWire host instance.
 * @param mode       Mode to finalize the blank into.
 * @param rom        ROM image, exactly TM01X_ROM_SIZE bytes.
 *
 * @returns true if the blank was written, verified and finalized successfully.
 */
bool tm01x_write_protocol_misc(
    OneWireHost* host,
    Tm01xKeyMode mode,
    const uint8_t rom[TM01X_ROM_SIZE]);
