#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit) ((value) |= (1UL << (bit)))
#define bit_clear(value, bit) ((value) &= ~(1UL << (bit)))
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))
#define DURATION_DIFF(x, y) ((x < y) ? (y - x) : (x - y))
#define abs(x) ((x) > 0 ? (x) : -(x))

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Flip the data bitwise.
 * @param key In data
 * @param count_bit number of data bits
 * @return Reverse data
 */
uint64_t subghz_protocol_blocks_reverse_key(uint64_t key, uint8_t count_bit);

/**
 * Get parity the data bitwise.
 * @param key In data
 * @param count_bit number of data bits
 * @return parity
 */
uint8_t subghz_protocol_blocks_get_parity(uint64_t key, uint8_t count_bit);

/**
 * CRC-4.
 * @param message array of bytes to check
 * @param nBytes number of bytes in message
 * @param polynomial CRC polynomial
 * @param init starting crc value
 * @return CRC value
 */
uint8_t subghz_protocol_blocks_crc4(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init);

/**
 * CRC-7.
 * @param message array of bytes to check
 * @param nBytes number of bytes in message
 * @param polynomial CRC polynomial
 * @param init starting crc value
 * @return CRC value
 */
uint8_t subghz_protocol_blocks_crc7(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init);

/**
 * Generic Cyclic Redundancy Check CRC-8.
 * Example polynomial: 0x31 = x8 + x5 + x4 + 1 (x8 is implicit)
 * Example polynomial: 0x80 = x8 + x7 (a normal bit-by-bit parity XOR)
 * @param message array of bytes to check
 * @param nBytes number of bytes in message
 * @param polynomial byte is from x^7 to x^0 (x^8 is implicitly one)
 * @param init starting crc value
 * @return CRC value
 */
uint8_t subghz_protocol_blocks_crc8(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init);

#ifdef __cplusplus
}
#endif
