#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit)           \
    ({                                \
        __typeof__(value) _one = (1); \
        (value) |= (_one << (bit));   \
    })
#define bit_clear(value, bit)         \
    ({                                \
        __typeof__(value) _one = (1); \
        (value) &= ~(_one << (bit));  \
    })
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))
#define DURATION_DIFF(x, y)             (((x) < (y)) ? ((y) - (x)) : ((x) - (y)))

#ifdef __cplusplus
extern "C" {
#endif

/** Flip the data bitwise
 *
 * @param      key        In data
 * @param      bit_count  number of data bits
 *
 * @return     Reverse data
 */
uint64_t subghz_protocol_blocks_reverse_key(uint64_t key, uint8_t bit_count);

/** Get parity the data bitwise
 *
 * @param      key        In data
 * @param      bit_count  number of data bits
 *
 * @return     parity
 */
uint8_t subghz_protocol_blocks_get_parity(uint64_t key, uint8_t bit_count);

/** CRC-4
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  CRC polynomial
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint8_t subghz_protocol_blocks_crc4(
    uint8_t const message[],
    size_t size,
    uint8_t polynomial,
    uint8_t init);

/** CRC-7
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  CRC polynomial
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint8_t subghz_protocol_blocks_crc7(
    uint8_t const message[],
    size_t size,
    uint8_t polynomial,
    uint8_t init);

/** Generic Cyclic Redundancy Check CRC-8. Example polynomial: 0x31 = x8 + x5 +
 * x4 + 1 (x8 is implicit) Example polynomial: 0x80 = x8 + x7 (a normal
 * bit-by-bit parity XOR)
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  byte is from x^7 to x^0 (x^8 is implicitly one)
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint8_t subghz_protocol_blocks_crc8(
    uint8_t const message[],
    size_t size,
    uint8_t polynomial,
    uint8_t init);

/** "Little-endian" Cyclic Redundancy Check CRC-8 LE Input and output are
 * reflected, i.e. least significant bit is shifted in first
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  CRC polynomial
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint8_t subghz_protocol_blocks_crc8le(
    uint8_t const message[],
    size_t size,
    uint8_t polynomial,
    uint8_t init);

/** CRC-16 LSB. Input and output are reflected, i.e. least significant bit is
 * shifted in first. Note that poly and init already need to be reflected
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  CRC polynomial
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint16_t subghz_protocol_blocks_crc16lsb(
    uint8_t const message[],
    size_t size,
    uint16_t polynomial,
    uint16_t init);

/** CRC-16
 *
 * @param      message     array of bytes to check
 * @param      size        number of bytes in message
 * @param      polynomial  CRC polynomial
 * @param      init        starting crc value
 *
 * @return     CRC value
 */
uint16_t subghz_protocol_blocks_crc16(
    uint8_t const message[],
    size_t size,
    uint16_t polynomial,
    uint16_t init);

/** Digest-8 by "LFSR-based Toeplitz hash"
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to digest
 * @param      gen      key stream generator, needs to includes the MSB if the
 *                      LFSR is rolling
 * @param      key      initial key
 *
 * @return     digest value
 */
uint8_t subghz_protocol_blocks_lfsr_digest8(
    uint8_t const message[],
    size_t size,
    uint8_t gen,
    uint8_t key);

/** Digest-8 by "LFSR-based Toeplitz hash", byte reflect, bit reflect
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to digest
 * @param      gen      key stream generator, needs to includes the MSB if the
 *                      LFSR is rolling
 * @param      key      initial key
 *
 * @return     digest value
 */
uint8_t subghz_protocol_blocks_lfsr_digest8_reflect(
    uint8_t const message[],
    size_t size,
    uint8_t gen,
    uint8_t key);

/** Digest-16 by "LFSR-based Toeplitz hash"
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to digest
 * @param      gen      key stream generator, needs to includes the MSB if the
 *                      LFSR is rolling
 * @param      key      initial key
 *
 * @return     digest value
 */
uint16_t subghz_protocol_blocks_lfsr_digest16(
    uint8_t const message[],
    size_t size,
    uint16_t gen,
    uint16_t key);

/** Compute Addition of a number of bytes
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to sum
 *
 * @return     summation value
 */
uint8_t subghz_protocol_blocks_add_bytes(uint8_t const message[], size_t size);

/** Compute bit parity of a single byte (8 bits)
 *
 * @param      byte  single byte to check
 *
 * @return     1 odd parity, 0 even parity
 */
uint8_t subghz_protocol_blocks_parity8(uint8_t byte);

/** Compute bit parity of a number of bytes
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to sum
 *
 * @return     1 odd parity, 0 even parity
 */
uint8_t subghz_protocol_blocks_parity_bytes(uint8_t const message[], size_t size);

/** Compute XOR (byte-wide parity) of a number of bytes
 *
 * @param      message  bytes of message data
 * @param      size     number of bytes to sum
 *
 * @return     summation value, per bit-position 1 odd parity, 0 even parity
 */
uint8_t subghz_protocol_blocks_xor_bytes(uint8_t const message[], size_t size);

#ifdef __cplusplus
}
#endif
