#include "bit_buffer.h"

#include <furi.h>

#define BITS_IN_BYTE (8)

struct BitBuffer {
    uint8_t* data;
    uint8_t* parity;
    size_t capacity_bytes;
    size_t size_bits;
};

BitBuffer* bit_buffer_alloc(size_t capacity_bytes) {
    furi_check(capacity_bytes);

    BitBuffer* buf = malloc(sizeof(BitBuffer));

    buf->data = malloc(capacity_bytes);
    size_t parity_buf_size = (capacity_bytes + BITS_IN_BYTE - 1) / BITS_IN_BYTE;
    buf->parity = malloc(parity_buf_size);
    buf->capacity_bytes = capacity_bytes;
    buf->size_bits = 0;

    return buf;
}

void bit_buffer_free(BitBuffer* buf) {
    furi_check(buf);

    free(buf->data);
    free(buf->parity);
    free(buf);
}

void bit_buffer_reset(BitBuffer* buf) {
    furi_check(buf);

    memset(buf->data, 0, buf->capacity_bytes);
    size_t parity_buf_size = (buf->capacity_bytes + BITS_IN_BYTE - 1) / BITS_IN_BYTE;
    memset(buf->parity, 0, parity_buf_size);
    buf->size_bits = 0;
}

void bit_buffer_copy(BitBuffer* buf, const BitBuffer* other) {
    furi_check(buf);
    furi_check(other);

    if(buf == other) return;

    furi_check(buf->capacity_bytes * BITS_IN_BYTE >= other->size_bits);

    memcpy(buf->data, other->data, bit_buffer_get_size_bytes(other));
    buf->size_bits = other->size_bits;
}

void bit_buffer_copy_right(BitBuffer* buf, const BitBuffer* other, size_t start_index) {
    furi_check(buf);
    furi_check(other);
    furi_check(bit_buffer_get_size_bytes(other) > start_index);
    furi_check(buf->capacity_bytes >= bit_buffer_get_size_bytes(other) - start_index);

    memcpy(buf->data, other->data + start_index, bit_buffer_get_size_bytes(other) - start_index);
    buf->size_bits = other->size_bits - start_index * BITS_IN_BYTE;
}

void bit_buffer_copy_left(BitBuffer* buf, const BitBuffer* other, size_t end_index) {
    furi_check(buf);
    furi_check(other);
    furi_check(bit_buffer_get_capacity_bytes(buf) >= end_index);
    furi_check(bit_buffer_get_size_bytes(other) >= end_index);

    memcpy(buf->data, other->data, end_index);
    buf->size_bits = end_index * BITS_IN_BYTE;
}

void bit_buffer_copy_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes) {
    furi_check(buf);
    furi_check(data);
    furi_check(buf->capacity_bytes >= size_bytes);

    memcpy(buf->data, data, size_bytes);
    buf->size_bits = size_bytes * BITS_IN_BYTE;
}

void bit_buffer_copy_bits(BitBuffer* buf, const uint8_t* data, size_t size_bits) {
    furi_check(buf);
    furi_check(data);
    furi_check(buf->capacity_bytes * BITS_IN_BYTE >= size_bits);

    size_t size_bytes = (size_bits + BITS_IN_BYTE - 1) / BITS_IN_BYTE;
    memcpy(buf->data, data, size_bytes);
    buf->size_bits = size_bits;
}

void bit_buffer_copy_bytes_with_parity(BitBuffer* buf, const uint8_t* data, size_t size_bits) {
    furi_check(buf);
    furi_check(data);

    size_t bits_processed = 0;
    size_t curr_byte = 0;

    if(size_bits < BITS_IN_BYTE + 1) {
        buf->size_bits = size_bits;
        buf->data[0] = data[0];
    } else {
        furi_check(size_bits % (BITS_IN_BYTE + 1) == 0);
        while(bits_processed < size_bits) {
            buf->data[curr_byte] = data[bits_processed / BITS_IN_BYTE] >>
                                   (bits_processed % BITS_IN_BYTE);
            buf->data[curr_byte] |= data[bits_processed / BITS_IN_BYTE + 1]
                                    << (BITS_IN_BYTE - bits_processed % BITS_IN_BYTE);
            uint8_t bit =
                FURI_BIT(data[bits_processed / BITS_IN_BYTE + 1], bits_processed % BITS_IN_BYTE);

            if((bits_processed % BITS_IN_BYTE) == 0) {
                buf->parity[curr_byte / BITS_IN_BYTE] = bit;
            } else {
                buf->parity[curr_byte / BITS_IN_BYTE] |= bit << (bits_processed % BITS_IN_BYTE);
            }
            bits_processed += BITS_IN_BYTE + 1;
            curr_byte++;
        }
        buf->size_bits = curr_byte * BITS_IN_BYTE;
    }
}

void bit_buffer_write_bytes(const BitBuffer* buf, void* dest, size_t size_bytes) {
    furi_check(buf);
    furi_check(dest);
    furi_check(bit_buffer_get_size_bytes(buf) <= size_bytes);

    memcpy(dest, buf->data, bit_buffer_get_size_bytes(buf));
}

void bit_buffer_write_bytes_with_parity(
    const BitBuffer* buf,
    void* dest,
    size_t size_bytes,
    size_t* bits_written) {
    furi_check(buf);
    furi_check(dest);
    furi_check(bits_written);

    size_t buf_size_bytes = bit_buffer_get_size_bytes(buf);
    size_t buf_size_with_parity_bytes =
        (buf_size_bytes * (BITS_IN_BYTE + 1) + BITS_IN_BYTE) / BITS_IN_BYTE;
    furi_check(buf_size_with_parity_bytes <= size_bytes);

    uint8_t next_par_bit = 0;
    uint16_t curr_bit_pos = 0;
    uint8_t* bitstream = dest;

    for(size_t i = 0; i < buf_size_bytes; i++) {
        next_par_bit = FURI_BIT(buf->parity[i / BITS_IN_BYTE], i % BITS_IN_BYTE);
        if(curr_bit_pos % BITS_IN_BYTE == 0) {
            bitstream[curr_bit_pos / BITS_IN_BYTE] = buf->data[i];
            curr_bit_pos += BITS_IN_BYTE;
            bitstream[curr_bit_pos / BITS_IN_BYTE] = next_par_bit;
            curr_bit_pos++;
        } else {
            bitstream[curr_bit_pos / BITS_IN_BYTE] |= buf->data[i]
                                                      << (curr_bit_pos % BITS_IN_BYTE);
            bitstream[curr_bit_pos / BITS_IN_BYTE + 1] =
                buf->data[i] >> (BITS_IN_BYTE - curr_bit_pos % BITS_IN_BYTE);
            bitstream[curr_bit_pos / BITS_IN_BYTE + 1] |= next_par_bit
                                                          << (curr_bit_pos % BITS_IN_BYTE);
            curr_bit_pos += BITS_IN_BYTE + 1;
        }
    }

    *bits_written = curr_bit_pos;
}

void bit_buffer_write_bytes_mid(
    const BitBuffer* buf,
    void* dest,
    size_t start_index,
    size_t size_bytes) {
    furi_check(buf);
    furi_check(dest);
    furi_check(start_index + size_bytes <= bit_buffer_get_size_bytes(buf));

    memcpy(dest, buf->data + start_index, size_bytes);
}

bool bit_buffer_has_partial_byte(const BitBuffer* buf) {
    furi_check(buf);

    return (buf->size_bits % BITS_IN_BYTE) != 0;
}

bool bit_buffer_starts_with_byte(const BitBuffer* buf, uint8_t byte) {
    furi_check(buf);

    return bit_buffer_get_size_bytes(buf) && (buf->data[0] == byte);
}

size_t bit_buffer_get_capacity_bytes(const BitBuffer* buf) {
    furi_check(buf);

    return buf->capacity_bytes;
}

size_t bit_buffer_get_size(const BitBuffer* buf) {
    furi_check(buf);

    return buf->size_bits;
}

size_t bit_buffer_get_size_bytes(const BitBuffer* buf) {
    furi_check(buf);

    return (buf->size_bits / BITS_IN_BYTE) + (buf->size_bits % BITS_IN_BYTE ? 1 : 0);
}

uint8_t bit_buffer_get_byte(const BitBuffer* buf, size_t index) {
    furi_check(buf);
    furi_check(buf->capacity_bytes > index);

    return buf->data[index];
}

uint8_t bit_buffer_get_byte_from_bit(const BitBuffer* buf, size_t index_bits) {
    furi_check(buf);
    furi_check(buf->capacity_bytes * BITS_IN_BYTE > index_bits);

    const size_t byte_index = index_bits / BITS_IN_BYTE;
    const size_t bit_offset = index_bits % BITS_IN_BYTE;

    const uint8_t lo = buf->data[byte_index] >> bit_offset;
    const uint8_t hi = buf->data[byte_index + 1] << (BITS_IN_BYTE - bit_offset);

    return lo | hi;
}

const uint8_t* bit_buffer_get_data(const BitBuffer* buf) {
    furi_check(buf);

    return buf->data;
}

const uint8_t* bit_buffer_get_parity(const BitBuffer* buf) {
    furi_check(buf);

    return buf->parity;
}

void bit_buffer_set_byte(BitBuffer* buf, size_t index, uint8_t byte) {
    furi_check(buf);

    const size_t size_bytes = bit_buffer_get_size_bytes(buf);
    furi_check(size_bytes > index);

    buf->data[index] = byte;
}

void bit_buffer_set_byte_with_parity(BitBuffer* buff, size_t index, uint8_t byte, bool parity) {
    furi_check(buff);
    furi_check(buff->size_bits / BITS_IN_BYTE > index);

    buff->data[index] = byte;
    if((index % BITS_IN_BYTE) == 0) {
        buff->parity[index / BITS_IN_BYTE] = parity;
    } else {
        buff->parity[index / BITS_IN_BYTE] |= parity << (index % BITS_IN_BYTE);
    }
}

void bit_buffer_set_size(BitBuffer* buf, size_t new_size) {
    furi_check(buf);
    furi_check(buf->capacity_bytes * BITS_IN_BYTE >= new_size);

    buf->size_bits = new_size;
}

void bit_buffer_set_size_bytes(BitBuffer* buf, size_t new_size_bytes) {
    furi_check(buf);
    furi_check(buf->capacity_bytes >= new_size_bytes);

    buf->size_bits = new_size_bytes * BITS_IN_BYTE;
}

void bit_buffer_append(BitBuffer* buf, const BitBuffer* other) {
    bit_buffer_append_right(buf, other, 0);
}

void bit_buffer_append_right(BitBuffer* buf, const BitBuffer* other, size_t start_index) {
    furi_check(buf);
    furi_check(other);

    const size_t size_bytes = bit_buffer_get_size_bytes(buf);
    const size_t other_size_bytes = bit_buffer_get_size_bytes(other) - start_index;

    furi_check(buf->capacity_bytes >= size_bytes + other_size_bytes);

    memcpy(buf->data + size_bytes, other->data + start_index, other_size_bytes);
    buf->size_bits += other->size_bits - start_index * BITS_IN_BYTE;
}

void bit_buffer_append_byte(BitBuffer* buf, uint8_t byte) {
    furi_check(buf);

    const size_t data_size_bytes = bit_buffer_get_size_bytes(buf);
    const size_t new_data_size_bytes = data_size_bytes + 1;
    furi_check(new_data_size_bytes <= buf->capacity_bytes);

    buf->data[data_size_bytes] = byte;
    buf->size_bits = new_data_size_bytes * BITS_IN_BYTE;
}

void bit_buffer_append_bytes(BitBuffer* buf, const uint8_t* data, size_t size_bytes) {
    furi_check(buf);
    furi_check(data);

    const size_t buf_size_bytes = bit_buffer_get_size_bytes(buf);
    furi_check(buf->capacity_bytes >= buf_size_bytes + size_bytes);

    memcpy(&buf->data[buf_size_bytes], data, size_bytes);
    buf->size_bits += size_bytes * BITS_IN_BYTE;
}

void bit_buffer_append_bit(BitBuffer* buf, bool bit) {
    furi_check(buf);
    furi_check(
        bit_buffer_get_size_bytes(buf) <=
        (buf->capacity_bytes - (bit_buffer_has_partial_byte(buf) ? 0 : 1)));

    if(bit) {
        const size_t byte_index = buf->size_bits / BITS_IN_BYTE;
        const size_t bit_offset = (buf->size_bits % BITS_IN_BYTE);
        buf->data[byte_index] |= 1U << bit_offset;
    }

    buf->size_bits++;
}
