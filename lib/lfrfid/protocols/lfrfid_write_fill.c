#include "lfrfid_write_fill.h"

#include <bit_lib/bit_lib.h>

// The maxblock field is 3 bits wide. t5577.h names the shift but not the width.
#define LFRFID_T5577_MAXBLOCK_FIELD_MASK (0x7)

void lfrfid_write_fill_t5577(LFRFIDWriteRequest* request, uint32_t block0, const uint8_t* bits) {
    const uint8_t blocks = (block0 >> LFRFID_T5577_MAXBLOCK_SHIFT) &
                           LFRFID_T5577_MAXBLOCK_FIELD_MASK;

    request->t5577.block[0] = block0;

    for(uint8_t block = 1; block <= blocks; block++) {
        request->t5577.block[block] = bit_lib_get_bits_32(bits, (block - 1) * 32, 32);
    }

    request->t5577.blocks_to_write = blocks + 1;
}

void lfrfid_write_fill_em4305(
    LFRFIDWriteRequest* request,
    uint32_t word4,
    const uint8_t* bits,
    bool invert) {
    const uint8_t words = EM4x05_GET_NUM_BLOCKS(word4);

    request->em4305.word[EM4x05_FIRST_USER_BLOCK - 1] = word4;

    for(uint8_t word = 0; word < words; word++) {
        uint32_t value = bit_lib_get_bits_32(bits, word * 32, 32);

        // Halves swap as they reverse, so the whole word comes out reversed.
        value = ((uint32_t)bit_lib_reverse_16_fast(value & 0xFFFF) << 16) |
                bit_lib_reverse_16_fast(value >> 16);

        if(invert) value = ~value;

        request->em4305.word[EM4x05_FIRST_USER_BLOCK + word] = value;
    }

    // The config word is written with the data, so the mask covers it too.
    request->em4305.mask = ((1u << (words + 1)) - 1) << (EM4x05_FIRST_USER_BLOCK - 1);
}
