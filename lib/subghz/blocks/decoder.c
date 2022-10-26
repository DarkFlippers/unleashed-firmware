#include "decoder.h"

#define TAG "SubGhzBlockDecoder"

void subghz_protocol_blocks_add_bit(SubGhzBlockDecoder* decoder, uint8_t bit) {
    decoder->decode_data = decoder->decode_data << 1 | bit;
    decoder->decode_count_bit++;
}

void subghz_protocol_blocks_add_to_128_bit(
    SubGhzBlockDecoder* decoder,
    uint8_t bit,
    uint64_t* head_64_bit) {
    if(++decoder->decode_count_bit > 64) {
        (*head_64_bit) = ((*head_64_bit) << 1) | (decoder->decode_data >> 63);
    }
    decoder->decode_data = decoder->decode_data << 1 | bit;
}

uint8_t subghz_protocol_blocks_get_hash_data(SubGhzBlockDecoder* decoder, size_t len) {
    uint8_t hash = 0;
    uint8_t* p = (uint8_t*)&decoder->decode_data;
    for(size_t i = 0; i < len; i++) {
        hash ^= p[i];
    }
    return hash;
}
