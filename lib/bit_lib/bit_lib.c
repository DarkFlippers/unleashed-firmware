#include "bit_lib.h"
#include <core/check.h>
#include <stdio.h>

void bit_lib_push_bit(uint8_t* data, size_t data_size, bool bit) {
    size_t last_index = data_size - 1;

    for(size_t i = 0; i < last_index; ++i) {
        data[i] = (data[i] << 1) | ((data[i + 1] >> 7) & 1);
    }
    data[last_index] = (data[last_index] << 1) | bit;
}

void bit_lib_set_bit(uint8_t* data, size_t position, bool bit) {
    if(bit) {
        data[position / 8] |= 1UL << (7 - (position % 8));
    } else {
        data[position / 8] &= ~(1UL << (7 - (position % 8)));
    }
}

void bit_lib_set_bits(uint8_t* data, size_t position, uint8_t byte, uint8_t length) {
    furi_check(length <= 8);
    furi_check(length > 0);

    for(uint8_t i = 0; i < length; ++i) {
        uint8_t shift = (length - 1) - i;
        bit_lib_set_bit(data, position + i, (byte >> shift) & 1); //-V610
    }
}

bool bit_lib_get_bit(const uint8_t* data, size_t position) {
    return (data[position / 8] >> (7 - (position % 8))) & 1;
}

uint8_t bit_lib_get_bits(const uint8_t* data, size_t position, uint8_t length) {
    uint8_t shift = position % 8;
    if(shift == 0) {
        return data[position / 8] >> (8 - length);
    } else {
        // TODO FL-3534: fix read out of bounds
        uint8_t value = (data[position / 8] << (shift));
        value |= data[position / 8 + 1] >> (8 - shift);
        value = value >> (8 - length);
        return value;
    }
}

uint16_t bit_lib_get_bits_16(const uint8_t* data, size_t position, uint8_t length) {
    uint16_t value = 0;
    if(length <= 8) {
        value = bit_lib_get_bits(data, position, length);
    } else {
        value = bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= bit_lib_get_bits(data, position + 8, length - 8);
    }
    return value;
}

uint32_t bit_lib_get_bits_32(const uint8_t* data, size_t position, uint8_t length) {
    uint32_t value = 0;
    if(length <= 8) {
        value = bit_lib_get_bits(data, position, length);
    } else if(length <= 16) {
        value = bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= bit_lib_get_bits(data, position + 8, length - 8);
    } else if(length <= 24) {
        value = bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= bit_lib_get_bits(data, position + 16, length - 16);
    } else {
        value = (uint32_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint32_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint32_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= bit_lib_get_bits(data, position + 24, length - 24);
    }

    return value;
}

uint64_t bit_lib_get_bits_64(const uint8_t* data, size_t position, uint8_t length) {
    uint64_t value = 0;
    if(length <= 8) {
        value = bit_lib_get_bits(data, position, length);
    } else if(length <= 16) {
        value = bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= bit_lib_get_bits(data, position + 8, length - 8);
    } else if(length <= 24) {
        value = bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= bit_lib_get_bits(data, position + 16, length - 16);
    } else if(length <= 32) {
        value = (uint64_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint64_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint64_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= bit_lib_get_bits(data, position + 24, length - 24);
    } else if(length <= 40) {
        value = (uint64_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint64_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint64_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= (uint64_t)bit_lib_get_bits(data, position + 24, 8) << (length - 32);
        value |= bit_lib_get_bits(data, position + 32, length - 32);
    } else if(length <= 48) {
        value = (uint64_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint64_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint64_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= (uint64_t)bit_lib_get_bits(data, position + 24, 8) << (length - 32);
        value |= (uint64_t)bit_lib_get_bits(data, position + 32, 8) << (length - 40);
        value |= bit_lib_get_bits(data, position + 40, length - 40);
    } else if(length <= 56) {
        value = (uint64_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint64_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint64_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= (uint64_t)bit_lib_get_bits(data, position + 24, 8) << (length - 32);
        value |= (uint64_t)bit_lib_get_bits(data, position + 32, 8) << (length - 40);
        value |= (uint64_t)bit_lib_get_bits(data, position + 40, 8) << (length - 48);
        value |= bit_lib_get_bits(data, position + 48, length - 48);
    } else {
        value = (uint64_t)bit_lib_get_bits(data, position, 8) << (length - 8);
        value |= (uint64_t)bit_lib_get_bits(data, position + 8, 8) << (length - 16);
        value |= (uint64_t)bit_lib_get_bits(data, position + 16, 8) << (length - 24);
        value |= (uint64_t)bit_lib_get_bits(data, position + 24, 8) << (length - 32);
        value |= (uint64_t)bit_lib_get_bits(data, position + 32, 8) << (length - 40);
        value |= (uint64_t)bit_lib_get_bits(data, position + 40, 8) << (length - 48);
        value |= (uint64_t)bit_lib_get_bits(data, position + 48, 8) << (length - 56);
        value |= bit_lib_get_bits(data, position + 56, length - 56);
    }

    return value;
}

bool bit_lib_test_parity_32(uint32_t bits, BitLibParity parity) {
#if !defined __GNUC__
#error Please, implement parity test for non-GCC compilers
#else
    switch(parity) {
    case BitLibParityEven:
        return __builtin_parity(bits);
    case BitLibParityOdd:
        return !__builtin_parity(bits);
    default:
        furi_crash("Unknown parity");
    }
#endif
}

bool bit_lib_test_parity(
    const uint8_t* bits,
    size_t position,
    uint8_t length,
    BitLibParity parity,
    uint8_t parity_length) {
    uint32_t parity_block;
    bool result = true;
    const size_t parity_blocks_count = length / parity_length;

    for(size_t i = 0; i < parity_blocks_count; ++i) {
        switch(parity) {
        case BitLibParityEven:
        case BitLibParityOdd:
            parity_block = bit_lib_get_bits_32(bits, position + i * parity_length, parity_length);
            if(!bit_lib_test_parity_32(parity_block, parity)) {
                result = false;
            }
            break;
        case BitLibParityAlways0:
            if(bit_lib_get_bit(bits, position + i * parity_length + parity_length - 1)) {
                result = false;
            }
            break;
        case BitLibParityAlways1:
            if(!bit_lib_get_bit(bits, position + i * parity_length + parity_length - 1)) {
                result = false;
            }
            break;
        }

        if(!result) break;
    }
    return result;
}

size_t bit_lib_add_parity(
    const uint8_t* data,
    size_t position,
    uint8_t* dest,
    size_t dest_position,
    uint8_t source_length,
    uint8_t parity_length,
    BitLibParity parity) {
    uint32_t parity_word = 0;
    size_t j = 0, bit_count = 0;
    for(int word = 0; word < source_length; word += parity_length - 1) {
        for(int bit = 0; bit < parity_length - 1; bit++) {
            parity_word = (parity_word << 1) | bit_lib_get_bit(data, position + word + bit);
            bit_lib_set_bit(
                dest, dest_position + j++, bit_lib_get_bit(data, position + word + bit));
        }
        // if parity fails then return 0
        switch(parity) {
        case BitLibParityAlways0:
            bit_lib_set_bit(dest, dest_position + j++, 0);
            break; // marker bit which should be a 0
        case BitLibParityAlways1:
            bit_lib_set_bit(dest, dest_position + j++, 1);
            break; // marker bit which should be a 1
        default:
            bit_lib_set_bit(
                dest,
                dest_position + j++,
                (bit_lib_test_parity_32(parity_word, BitLibParityOdd) ^ parity) ^ 1);
            break;
        }
        bit_count += parity_length;
        parity_word = 0;
    }
    // if we got here then all the parities passed
    // return bit count
    return bit_count;
}

size_t bit_lib_remove_bit_every_nth(uint8_t* data, size_t position, uint8_t length, uint8_t n) {
    size_t counter = 0;
    size_t result_counter = 0;
    uint8_t bit_buffer = 0;
    uint8_t bit_counter = 0;

    while(counter < length) {
        if((counter + 1) % n != 0) {
            bit_buffer = (bit_buffer << 1) | bit_lib_get_bit(data, position + counter);
            bit_counter++;
        }

        if(bit_counter == 8) {
            bit_lib_set_bits(data, position + result_counter, bit_buffer, 8);
            bit_counter = 0;
            bit_buffer = 0;
            result_counter += 8;
        }
        counter++;
    }

    if(bit_counter != 0) {
        bit_lib_set_bits(data, position + result_counter, bit_buffer, bit_counter);
        result_counter += bit_counter;
    }
    return result_counter;
}

void bit_lib_copy_bits(
    uint8_t* data,
    size_t position,
    size_t length,
    const uint8_t* source,
    size_t source_position) {
    for(size_t i = 0; i < length; ++i) {
        bit_lib_set_bit(data, position + i, bit_lib_get_bit(source, source_position + i));
    }
}

void bit_lib_reverse_bits(uint8_t* data, size_t position, uint8_t length) {
    size_t i = 0;
    size_t j = length - 1;

    while(i < j) {
        bool tmp = bit_lib_get_bit(data, position + i);
        bit_lib_set_bit(data, position + i, bit_lib_get_bit(data, position + j));
        bit_lib_set_bit(data, position + j, tmp);
        i++;
        j--;
    }
}

uint8_t bit_lib_get_bit_count(uint32_t data) {
#if defined __GNUC__
    return __builtin_popcountl(data);
#else
#error Please, implement popcount for non-GCC compilers
#endif
}

void bit_lib_print_bits(const uint8_t* data, size_t length) {
    for(size_t i = 0; i < length; ++i) {
        printf("%u", bit_lib_get_bit(data, i));
    }
}

void bit_lib_print_regions(
    const BitLibRegion* regions,
    size_t region_count,
    const uint8_t* data,
    size_t length) {
    // print data
    bit_lib_print_bits(data, length);
    printf("\r\n");

    // print regions
    for(size_t c = 0; c < length; ++c) {
        bool print = false;

        for(size_t i = 0; i < region_count; i++) {
            if(regions[i].start <= c && c < regions[i].start + regions[i].length) {
                print = true;
                printf("%c", regions[i].mark);
                break;
            }
        }

        if(!print) {
            printf(" ");
        }
    }
    printf("\r\n");

    // print regions data
    for(size_t c = 0; c < length; ++c) {
        bool print = false;

        for(size_t i = 0; i < region_count; i++) {
            if(regions[i].start <= c && c < regions[i].start + regions[i].length) {
                print = true;
                printf("%u", bit_lib_get_bit(data, c));
                break;
            }
        }

        if(!print) {
            printf(" ");
        }
    }
    printf("\r\n");
}

uint16_t bit_lib_reverse_16_fast(uint16_t data) {
    uint16_t result = 0;
    result |= (data & 0x8000) >> 15;
    result |= (data & 0x4000) >> 13;
    result |= (data & 0x2000) >> 11;
    result |= (data & 0x1000) >> 9;
    result |= (data & 0x0800) >> 7;
    result |= (data & 0x0400) >> 5;
    result |= (data & 0x0200) >> 3;
    result |= (data & 0x0100) >> 1;
    result |= (data & 0x0080) << 1;
    result |= (data & 0x0040) << 3;
    result |= (data & 0x0020) << 5;
    result |= (data & 0x0010) << 7;
    result |= (data & 0x0008) << 9;
    result |= (data & 0x0004) << 11;
    result |= (data & 0x0002) << 13;
    result |= (data & 0x0001) << 15;
    return result;
}

uint8_t bit_lib_reverse_8_fast(uint8_t byte) {
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
    return byte;
}

uint16_t bit_lib_crc8(
    uint8_t const* data,
    size_t data_size,
    uint8_t polynom,
    uint8_t init,
    bool ref_in,
    bool ref_out,
    uint8_t xor_out) {
    uint8_t crc = init;

    for(size_t i = 0; i < data_size; ++i) {
        uint8_t byte = data[i];
        if(ref_in) bit_lib_reverse_bits(&byte, 0, 8);
        crc ^= byte;

        for(size_t j = 8; j > 0; --j) {
            if(crc & TOPBIT(8)) {
                crc = (crc << 1) ^ polynom;
            } else {
                crc = (crc << 1);
            }
        }
    }

    if(ref_out) bit_lib_reverse_bits(&crc, 0, 8);
    crc ^= xor_out;

    return crc;
}

uint16_t bit_lib_crc16(
    uint8_t const* data,
    size_t data_size,
    uint16_t polynom,
    uint16_t init,
    bool ref_in,
    bool ref_out,
    uint16_t xor_out) {
    uint16_t crc = init;

    for(size_t i = 0; i < data_size; ++i) {
        uint8_t byte = data[i];
        if(ref_in) byte = bit_lib_reverse_16_fast(byte) >> 8;

        for(size_t j = 0; j < 8; ++j) {
            bool c15 = (crc >> 15 & 1);
            bool bit = (byte >> (7 - j) & 1);
            crc <<= 1;
            if(c15 ^ bit) crc ^= polynom;
        }
    }

    if(ref_out) crc = bit_lib_reverse_16_fast(crc);
    crc ^= xor_out;

    return crc;
}

void bit_lib_num_to_bytes_be(uint64_t src, uint8_t len, uint8_t* dest) {
    furi_check(dest);
    furi_check(len <= 8);

    while(len--) {
        dest[len] = (uint8_t)src;
        src >>= 8;
    }
}

void bit_lib_num_to_bytes_le(uint64_t src, uint8_t len, uint8_t* dest) {
    furi_check(dest);
    furi_check(len <= 8);

    for(int i = 0; i < len; i++) {
        dest[i] = (uint8_t)(src >> (8 * i));
    }
}

uint64_t bit_lib_bytes_to_num_be(const uint8_t* src, uint8_t len) {
    furi_check(src);
    furi_check(len <= 8);

    uint64_t res = 0;
    while(len--) {
        res = (res << 8) | (*src);
        src++;
    }
    return res;
}

uint64_t bit_lib_bytes_to_num_le(const uint8_t* src, uint8_t len) {
    furi_check(src);
    furi_check(len <= 8);

    uint64_t res = 0;
    uint8_t shift = 0;
    while(len--) {
        res |= ((uint64_t)*src) << (8 * shift++);
        src++;
    }
    return res;
}

uint64_t bit_lib_bytes_to_num_bcd(const uint8_t* src, uint8_t len, bool* is_bcd) {
    furi_check(src);
    furi_check(len <= 9);

    uint64_t res = 0;
    uint8_t nibble_1, nibble_2;
    *is_bcd = true;

    for(uint8_t i = 0; i < len; i++) {
        nibble_1 = src[i] / 16;
        nibble_2 = src[i] % 16;
        if((nibble_1 > 9) || (nibble_2 > 9)) *is_bcd = false;

        res *= 10;
        res += nibble_1;

        res *= 10;
        res += nibble_2;
    }

    return res;
}
