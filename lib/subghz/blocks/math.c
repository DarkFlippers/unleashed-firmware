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