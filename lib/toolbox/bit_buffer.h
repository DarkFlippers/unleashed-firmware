/** Bit Buffer
 *
 * Various bits and bytes manipulation tools.
 *
 * @file bit_buffer.h
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BitBuffer BitBuffer;

/** Allocate a BitBuffer instance.
 *
 * @param[in]  capacity_bytes  maximum buffer capacity, in bytes
 *
 * @return     pointer to the allocated BitBuffer instance
 */
BitBuffer* bit_buffer_alloc(size_t capacity_bytes);

/** Delete a BitBuffer instance.
 *
 * @param[in,out] buf   pointer to a BitBuffer instance
 */
void bit_buffer_free(BitBuffer* buf);

/** Clear all data from a BitBuffer instance.
 *
 * @param[in,out] buf   pointer to a BitBuffer instance
 */
void bit_buffer_reset(BitBuffer* buf);

// Copy and write

/** Copy another BitBuffer instance's contents to this one, replacing all of the
 * original data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size.
 *
 * @param[in,out] buf    pointer to a BitBuffer instance to copy into
 * @param[in]     other  pointer to a BitBuffer instance to copy from
 */
void bit_buffer_copy(BitBuffer* buf, const BitBuffer* other);

/** Copy all BitBuffer instance's contents to this one, starting from
 * start_index, replacing all of the original data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size counting from start_index.
 *
 * @param[in,out] buf          pointer to a BitBuffer instance to copy into
 * @param[in]     other        pointer to a BitBuffer instance to copy from
 * @param[in]     start_index  index to begin copying source data from
 */
void bit_buffer_copy_right(BitBuffer* buf, const BitBuffer* other, size_t start_index);

/** Copy all BitBuffer instance's contents to this one, ending with end_index,
 * replacing all of the original data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size counting to end_index.
 *
 * @param[in,out] buf        pointer to a BitBuffer instance to copy into
 * @param[in]     other      pointer to a BitBuffer instance to copy from
 * @param[in]     end_index  index to end copying source data at
 */
void bit_buffer_copy_left(BitBuffer* buf, const BitBuffer* other, size_t end_index);

/** Copy a byte array to a BitBuffer instance, replacing all of the original
 * data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size.
 *
 * @param[in,out] buf         pointer to a BitBuffer instance to copy into
 * @param[in]     data        pointer to the byte array to be copied
 * @param[in]     size_bytes  size of the data to be copied, in bytes
 */
void bit_buffer_copy_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes);

/** Copy a byte array to a BitBuffer instance, replacing all of the original
 * data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size.
 *
 * @param[in,out] buf        pointer to a BitBuffer instance to copy into
 * @param[in]     data       pointer to the byte array to be copied
 * @param[in]     size_bits  size of the data to be copied, in bits
 */
void bit_buffer_copy_bits(BitBuffer* buf, const uint8_t* data, size_t size_bits);

/** Copy a byte with parity array to a BitBuffer instance, replacing all of the
 * original data.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size.
 *
 * @param[in,out] buf        pointer to a BitBuffer instance to copy into
 * @param[in]     data       pointer to the byte array to be copied
 * @param[in]     size_bits  size of the data to be copied, in bits
 * @note          Parity bits are placed starting with the most significant bit
 *                of each byte and moving up.
 * @note          Example: DDDDDDDD PDDDDDDD DPDDDDDD DDP...
 */
void bit_buffer_copy_bytes_with_parity(BitBuffer* buf, const uint8_t* data, size_t size_bits);

/** Write a BitBuffer instance's entire contents to an arbitrary memory location.
 *
 * @warning    The destination memory must be allocated. Additionally, the
 *             destination capacity must be no less than the source data size.
 *
 * @param[in]  buf         pointer to a BitBuffer instance to write from
 * @param[out] dest        pointer to the destination memory location
 * @param[in]  size_bytes  maximum destination data size, in bytes
 */
void bit_buffer_write_bytes(const BitBuffer* buf, void* dest, size_t size_bytes);

/** Write a BitBuffer instance's entire contents to an arbitrary memory location.
 *
 * Additionally, place a parity bit after each byte.
 *
 * @warning    The destination memory must be allocated. Additionally, the
 *             destination capacity must be no less than the source data size
 *             plus parity.
 *
 * @param[in]  buf           pointer to a BitBuffer instance to write from
 * @param[out] dest          pointer to the destination memory location
 * @param[in]  size_bytes    maximum destination data size, in bytes
 * @param[out] bits_written  actual number of bits written, in bits
 * @note       Parity bits are placed starting with the most significant bit of
 *             each byte and moving up.
 * @note       Example: DDDDDDDD PDDDDDDD DPDDDDDD DDP...
 */
void bit_buffer_write_bytes_with_parity(
    const BitBuffer* buf,
    void* dest,
    size_t size_bytes,
    size_t* bits_written);

/** Write a slice of BitBuffer instance's contents to an arbitrary memory
 * location.
 *
 * @warning    The destination memory must be allocated. Additionally, the
 *             destination capacity must be no less than the requested slice
 *             size.
 *
 * @param[in]  buf          pointer to a BitBuffer instance to write from
 * @param[out] dest         pointer to the destination memory location
 * @param[in]  start_index  index to begin copying source data from
 * @param[in]  size_bytes   data slice size, in bytes
 */
void bit_buffer_write_bytes_mid(
    const BitBuffer* buf,
    void* dest,
    size_t start_index,
    size_t size_bytes);

// Checks

/** Check whether a BitBuffer instance contains a partial byte (i.e.\ the bit
 * count is not divisible by 8).
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be checked
 *
 * @return     true if the instance contains a partial byte, false otherwise
 */
bool bit_buffer_has_partial_byte(const BitBuffer* buf);

/** Check whether a BitBuffer instance's contents start with the designated byte.
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be checked
 * @param[in]  byte  byte value to be checked against
 *
 * @return     true if data starts with designated byte, false otherwise
 */
bool bit_buffer_starts_with_byte(const BitBuffer* buf, uint8_t byte);

// Getters

/** Get a BitBuffer instance's capacity (i.e.\ the maximum possible amount of
 * data), in bytes.
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be queried
 *
 * @return     capacity, in bytes
 */
size_t bit_buffer_get_capacity_bytes(const BitBuffer* buf);

/** Get a BitBuffer instance's data size (i.e.\ the amount of stored data), in
 * bits.
 *
 * @warning    Might be not divisible by 8 (see bit_buffer_is_partial_byte).
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be queried
 *
 * @return     data size, in bits.
 */
size_t bit_buffer_get_size(const BitBuffer* buf);

/**
 * Get a BitBuffer instance's data size (i.e.\ the amount of stored data), in
 * bytes.
 *
 * @warning    If a partial byte is present, it is also counted.
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be queried
 *
 * @return     data size, in bytes.
 */
size_t bit_buffer_get_size_bytes(const BitBuffer* buf);

/** Get a byte value at a specified index in a BitBuffer instance.
 *
 * @warning    The index must be valid (i.e.\ less than the instance's data size
 *             in bytes).
 *
 * @param[in]  buf    pointer to a BitBuffer instance to be queried
 * @param[in]  index  index of the byte in question
 *
 * @return     byte value
 */
uint8_t bit_buffer_get_byte(const BitBuffer* buf, size_t index);

/** Get a byte value starting from the specified bit index in a BitBuffer
 * instance.
 *
 * @warning    The resulting byte might correspond to a single byte (if the
 *             index is a multiple of 8), or two overlapping bytes combined. The
 *             index must be valid (i.e.\ less than the instance's data size in
 *             bits).
 *
 * @param[in]  buf         pointer to a BitBuffer instance to be queried
 * @param[in]  index_bits  bit index of the byte in question
 *
 * @return     byte value
 */
uint8_t bit_buffer_get_byte_from_bit(const BitBuffer* buf, size_t index_bits);

/** Get the pointer to a BitBuffer instance's underlying data.
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be queried
 *
 * @return     pointer to the underlying data
 */
const uint8_t* bit_buffer_get_data(const BitBuffer* buf);

/** Get the pointer to the parity data of a BitBuffer instance.
 *
 * @param[in]  buf   pointer to a BitBuffer instance to be queried
 *
 * @return     pointer to the parity data
 */
const uint8_t* bit_buffer_get_parity(const BitBuffer* buf);

// Setters

/** Set byte value at a specified index in a BitBuffer instance.
 *
 * @warning       The index must be valid (i.e.\ less than the instance's data
 *                size in bytes).
 *
 * @param[in,out] buf    pointer to a BitBuffer instance to be modified
 * @param[in]     index  index of the byte in question
 * @param[in]     byte   byte value to be set at index
 */
void bit_buffer_set_byte(BitBuffer* buf, size_t index, uint8_t byte);

/** Set byte and parity bit value at a specified index in a BitBuffer instance.
 *
 * @warning       The index must be valid (i.e.\ less than the instance's data
 *                size in bytes).
 *
 * @param[in,out] buff    pointer to a BitBuffer instance to be modified
 * @param[in]     index   index of the byte in question
 * @param[in]     byte    byte value to be set at index
 * @param[in]     parity  parity bit value to be set at index
 */
void bit_buffer_set_byte_with_parity(BitBuffer* buff, size_t index, uint8_t byte, bool parity);

/** Resize a BitBuffer instance to a new size, in bits.
 *
 * @warning       May cause bugs. Use only if absolutely necessary.
 *
 * @param[in,out] buf       pointer to a BitBuffer instance to be resized
 * @param[in]     new_size  the new size of the buffer, in bits
 */
void bit_buffer_set_size(BitBuffer* buf, size_t new_size);

/** Resize a BitBuffer instance to a new size, in bytes.
 *
 * @warning       May cause bugs. Use only if absolutely necessary.
 *
 * @param[in,out] buf             pointer to a BitBuffer instance to be resized
 * @param[in]     new_size_bytes  the new size of the buffer, in bytes
 */
void bit_buffer_set_size_bytes(BitBuffer* buf, size_t new_size_bytes);

// Modification

/** Append all BitBuffer's instance contents to this one.
 *
 * @warning       The destination capacity must be no less than its original
 *                data size plus source data size.
 *
 * @param[in,out] buf    pointer to a BitBuffer instance to be appended to
 * @param[in]     other  pointer to a BitBuffer instance to be appended
 */
void bit_buffer_append(BitBuffer* buf, const BitBuffer* other);

/** Append a BitBuffer's instance contents to this one, starting from
 * start_index.
 *
 * @warning       The destination capacity must be no less than the source data
 *                size counting from start_index.
 *
 * @param[in,out] buf          pointer to a BitBuffer instance to be appended to
 * @param[in]     other        pointer to a BitBuffer instance to be appended
 * @param[in]     start_index  index to begin copying source data from
 */
void bit_buffer_append_right(BitBuffer* buf, const BitBuffer* other, size_t start_index);

/** Append a byte to a BitBuffer instance.
 *
 * @warning       The destination capacity must be no less its original data
 *                size plus one.
 *
 * @param[in,out] buf   pointer to a BitBuffer instance to be appended to
 * @param[in]     byte  byte value to be appended
 */
void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte);

/** Append a byte array to a BitBuffer instance.
 *
 * @warning       The destination capacity must be no less its original data
 *                size plus source data size.
 *
 * @param[in,out] buf         pointer to a BitBuffer instance to be appended to
 * @param[in]     data        pointer to the byte array to be appended
 * @param[in]     size_bytes  size of the data to be appended, in bytes
 */
void bit_buffer_append_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes);

/** Append a bit to a BitBuffer instance.
 *
 * @warning       The destination capacity must be sufficient to accommodate the
 *                additional bit.
 *
 * @param[in,out] buf   pointer to a BitBuffer instance to be appended to
 * @param[in]     bit   bit value to be appended
 */
void bit_buffer_append_bit(BitBuffer* buf, bool bit);

#ifdef __cplusplus
}
#endif
