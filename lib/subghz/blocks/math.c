#include "math.h"

uint64_t subghz_protocol_blocks_reverse_key(uint64_t key, uint8_t count_bit) {
    uint64_t key_reverse = 0;
    for(uint8_t i = 0; i < count_bit; i++) {
        key_reverse = key_reverse << 1 | bit_read(key, i);
    }
    return key_reverse;
}

uint8_t subghz_protocol_blocks_get_parity(uint64_t key, uint8_t count_bit) {
    uint8_t parity = 0;
    for(uint8_t i = 0; i < count_bit; i++) {
        parity += bit_read(key, i);
    }
    return parity & 0x01;
}

uint8_t subghz_protocol_blocks_crc4(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    unsigned remainder = init << 4; // LSBs are unused
    unsigned poly = polynomial << 4;
    unsigned bit;

    while(nBytes--) {
        remainder ^= *message++;
        for(bit = 0; bit < 8; bit++) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ poly;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 4 & 0x0f; // discard the LSBs
}

uint8_t subghz_protocol_blocks_crc7(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    unsigned remainder = init << 1; // LSB is unused
    unsigned poly = polynomial << 1;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ poly;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 1 & 0x7f; // discard the LSB
}

uint8_t subghz_protocol_blocks_crc8(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    uint8_t remainder = init;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ polynomial;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}

uint8_t subghz_protocol_blocks_crc8le(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    uint8_t remainder = subghz_protocol_blocks_reverse_key(init, 8);
    unsigned byte, bit;
    polynomial = subghz_protocol_blocks_reverse_key(polynomial, 8);

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 1) {
                remainder = (remainder >> 1) ^ polynomial;
            } else {
                remainder = (remainder >> 1);
            }
        }
    }
    return remainder;
}

uint16_t subghz_protocol_blocks_crc16lsb(
    uint8_t const message[],
    unsigned nBytes,
    uint16_t polynomial,
    uint16_t init) {
    uint16_t remainder = init;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 1) {
                remainder = (remainder >> 1) ^ polynomial;
            } else {
                remainder = (remainder >> 1);
            }
        }
    }
    return remainder;
}

uint16_t subghz_protocol_blocks_crc16(
    uint8_t const message[],
    unsigned nBytes,
    uint16_t polynomial,
    uint16_t init) {
    uint16_t remainder = init;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte] << 8;
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x8000) {
                remainder = (remainder << 1) ^ polynomial;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}

uint8_t subghz_protocol_blocks_lfsr_digest8(
    uint8_t const message[],
    unsigned bytes,
    uint8_t gen,
    uint8_t key) {
    uint8_t sum = 0;
    for(unsigned k = 0; k < bytes; ++k) {
        uint8_t data = message[k];
        for(int i = 7; i >= 0; --i) {
            // XOR key into sum if data bit is set
            if((data >> i) & 1) sum ^= key;

            // roll the key right (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if(key & 1)
                key = (key >> 1) ^ gen;
            else
                key = (key >> 1);
        }
    }
    return sum;
}

uint8_t subghz_protocol_blocks_lfsr_digest8_reflect(
    uint8_t const message[],
    int bytes,
    uint8_t gen,
    uint8_t key) {
    uint8_t sum = 0;
    // Process message from last byte to first byte (reflected)
    for(int k = bytes - 1; k >= 0; --k) {
        uint8_t data = message[k];
        // Process individual bits of each byte (reflected)
        for(int i = 0; i < 8; ++i) {
            // XOR key into sum if data bit is set
            if((data >> i) & 1) {
                sum ^= key;
            }

            // roll the key left (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if(key & 0x80)
                key = (key << 1) ^ gen;
            else
                key = (key << 1);
        }
    }
    return sum;
}

uint16_t subghz_protocol_blocks_lfsr_digest16(
    uint8_t const message[],
    unsigned bytes,
    uint16_t gen,
    uint16_t key) {
    uint16_t sum = 0;
    for(unsigned k = 0; k < bytes; ++k) {
        uint8_t data = message[k];
        for(int i = 7; i >= 0; --i) {
            // if data bit is set then xor with key
            if((data >> i) & 1) sum ^= key;

            // roll the key right (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if(key & 1)
                key = (key >> 1) ^ gen;
            else
                key = (key >> 1);
        }
    }
    return sum;
}

uint8_t subghz_protocol_blocks_add_bytes(uint8_t const message[], unsigned num_bytes) {
    int result = 0;
    for(unsigned i = 0; i < num_bytes; ++i) {
        result += message[i];
    }
    return (uint8_t)result;
}

int subghz_protocol_blocks_parity8(uint8_t byte) {
    byte ^= byte >> 4;
    byte &= 0xf;
    return (0x6996 >> byte) & 1;
}

int subghz_protocol_blocks_parity_bytes(uint8_t const message[], unsigned num_bytes) {
    int result = 0;
    for(unsigned i = 0; i < num_bytes; ++i) {
        result ^= subghz_protocol_blocks_parity8(message[i]);
    }
    return result;
}

uint8_t subghz_protocol_blocks_xor_bytes(uint8_t const message[], unsigned num_bytes) {
    uint8_t result = 0;
    for(unsigned i = 0; i < num_bytes; ++i) {
        result ^= message[i];
    }
    return result;
}