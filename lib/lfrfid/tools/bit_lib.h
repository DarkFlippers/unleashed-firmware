#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TOPBIT(X) (1 << (X - 1))

typedef enum {
    BitLibParityEven,
    BitLibParityOdd,
    BitLibParityAlways0,
    BitLibParityAlways1,
} BitLibParity;

/** @brief Increment and wrap around a value.
 *  @param index value to increment
 *  @param length wrap-around range
 */
#define bit_lib_increment_index(index, length) (index = (((index) + 1) % (length)))

/** @brief Test if a bit is set.
 *  @param data value to test
 *  @param index bit index to test
 */
#define bit_lib_bit_is_set(data, index) ((data & (1 << (index))) != 0)

/** @brief Test if a bit is not set.
 *  @param data value to test
 *  @param index bit index to test
 */
#define bit_lib_bit_is_not_set(data, index) ((data & (1 << (index))) == 0)

/** @brief Push a bit into a byte array.
 *  @param data array to push bit into
 *  @param data_size array size
 *  @param bit bit to push
 */
void bit_lib_push_bit(uint8_t* data, size_t data_size, bool bit);

/** @brief Set a bit in a byte array.
 *  @param data array to set bit in
 *  @param position The position of the bit to set.
 *  @param bit bit value to set
 */
void bit_lib_set_bit(uint8_t* data, size_t position, bool bit);

/** @brief Set the bit at the given position to the given value.
 * @param data The data to set the bit in.
 * @param position The position of the bit to set.
 * @param byte The data to set the bit to.
 * @param length The length of the data.
 */
void bit_lib_set_bits(uint8_t* data, size_t position, uint8_t byte, uint8_t length);

/** @brief Get the bit of a byte.
 * @param data The byte to get the bits from.
 * @param position The position of the bit.
 * @return The bit.
 */
bool bit_lib_get_bit(const uint8_t* data, size_t position);

/**
 * @brief Get the bits of a data, as uint8_t.
 * @param data The data to get the bits from.
 * @param position The position of the first bit.
 * @param length The length of the bits.
 * @return The bits.
 */
uint8_t bit_lib_get_bits(const uint8_t* data, size_t position, uint8_t length);

/**
 * @brief Get the bits of a data, as uint16_t.
 * @param data The data to get the bits from.
 * @param position The position of the first bit.
 * @param length The length of the bits.
 * @return The bits.
 */
uint16_t bit_lib_get_bits_16(const uint8_t* data, size_t position, uint8_t length);

/**
 * @brief Get the bits of a data, as uint32_t.
 * @param data The data to get the bits from.
 * @param position The position of the first bit.
 * @param length The length of the bits.
 * @return The bits.
 */
uint32_t bit_lib_get_bits_32(const uint8_t* data, size_t position, uint8_t length);

/**
 * @brief Test parity of given bits
 * @param bits Bits to test parity of
 * @param parity Parity to test against
 * @return true if parity is correct, false otherwise
 */
bool bit_lib_test_parity_32(uint32_t bits, BitLibParity parity);

/**
 * @brief Test parity of bit array, check parity for every parity_length block from start
 * 
 * @param data Bit array
 * @param position Start position
 * @param length Bit count
 * @param parity Parity to test against
 * @param parity_length Parity block length
 * @return true 
 * @return false 
 */
bool bit_lib_test_parity(
    const uint8_t* data,
    size_t position,
    uint8_t length,
    BitLibParity parity,
    uint8_t parity_length);

/**
 * @brief Add parity to bit array
 * 
 * @param data Source bit array
 * @param position Start position
 * @param dest Destination bit array
 * @param dest_position Destination position
 * @param source_length Source bit count
 * @param parity_length Parity block length
 * @param parity Parity to test against
 * @return size_t 
 */
size_t bit_lib_add_parity(
    const uint8_t* data,
    size_t position,
    uint8_t* dest,
    size_t dest_position,
    uint8_t source_length,
    uint8_t parity_length,
    BitLibParity parity);

/**
 * @brief Remove bit every n in array and shift array left. Useful to remove parity.
 * 
 * @param data Bit array
 * @param position Start position
 * @param length Bit count
 * @param n every n bit will be removed
 * @return size_t 
 */
size_t bit_lib_remove_bit_every_nth(uint8_t* data, size_t position, uint8_t length, uint8_t n);

/**
 * @brief Copy bits from source to destination.
 * 
 * @param data destination array
 * @param position position in destination array
 * @param length length of bits to copy
 * @param source source array
 * @param source_position position in source array
 */
void bit_lib_copy_bits(
    uint8_t* data,
    size_t position,
    size_t length,
    const uint8_t* source,
    size_t source_position);

/**
 * @brief Reverse bits in bit array
 * 
 * @param data Bit array
 * @param position start position
 * @param length length of bits to reverse
 */
void bit_lib_reverse_bits(uint8_t* data, size_t position, uint8_t length);

/**
 * @brief Count 1 bits in data
 * 
 * @param data 
 * @return uint8_t set bit count
 */
uint8_t bit_lib_get_bit_count(uint32_t data);

/**
 * @brief Print data as bit array
 * 
 * @param data 
 * @param length 
 */
void bit_lib_print_bits(const uint8_t* data, size_t length);

typedef struct {
    const char mark;
    const size_t start;
    const size_t length;
} BitLibRegion;

/**
 * @brief Print data as bit array and mark regions. Regions needs to be sorted by start position.
 * 
 * @param regions 
 * @param region_count 
 * @param data 
 * @param length 
 */
void bit_lib_print_regions(
    const BitLibRegion* regions,
    size_t region_count,
    const uint8_t* data,
    size_t length);

/**
 * @brief Reverse bits in uint16_t, faster than generic bit_lib_reverse_bits.
 * 
 * @param data 
 * @return uint16_t 
 */
uint16_t bit_lib_reverse_16_fast(uint16_t data);

/**
 * @brief Reverse bits in uint8_t, faster than generic bit_lib_reverse_bits.
 * 
 * @param byte Byte
 * @return uint8_t the reversed byte
 */
uint8_t bit_lib_reverse_8_fast(uint8_t byte);

/**
 * @brief Slow, but generic CRC8 implementation
 * 
 * @param data 
 * @param data_size 
 * @param polynom CRC polynom
 * @param init init value
 * @param ref_in true if the right bit is older
 * @param ref_out true to reverse output
 * @param xor_out xor output with this value
 * @return uint8_t 
 */
uint16_t bit_lib_crc8(
    uint8_t const* data,
    size_t data_size,
    uint8_t polynom,
    uint8_t init,
    bool ref_in,
    bool ref_out,
    uint8_t xor_out);

/**
 * @brief Slow, but generic CRC16 implementation
 * 
 * @param data 
 * @param data_size 
 * @param polynom CRC polynom
 * @param init init value
 * @param ref_in true if the right bit is older
 * @param ref_out true to reverse output
 * @param xor_out xor output with this value
 * @return uint16_t 
 */
uint16_t bit_lib_crc16(
    uint8_t const* data,
    size_t data_size,
    uint16_t polynom,
    uint16_t init,
    bool ref_in,
    bool ref_out,
    uint16_t xor_out);

#ifdef __cplusplus
}
#endif