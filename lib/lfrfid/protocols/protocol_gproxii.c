#include <furi.h>
#include "toolbox/level_duration.h"
#include "protocol_gproxii.h"
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define GPROXII_PREAMBLE_BIT_SIZE (6)
#define GPROXII_ENCODED_BIT_SIZE  (90)
#define GPROXII_ENCODED_BYTE_FULL_SIZE \
    (((GPROXII_PREAMBLE_BIT_SIZE + GPROXII_ENCODED_BIT_SIZE) / 8))

#define GPROXII_DATA_SIZE (12)

#define GPROXII_SHORT_TIME  (256)
#define GPROXII_LONG_TIME   (512)
#define GPROXII_JITTER_TIME (120)

#define GPROXII_SHORT_TIME_LOW  (GPROXII_SHORT_TIME - GPROXII_JITTER_TIME)
#define GPROXII_SHORT_TIME_HIGH (GPROXII_SHORT_TIME + GPROXII_JITTER_TIME)
#define GPROXII_LONG_TIME_LOW   (GPROXII_LONG_TIME - GPROXII_JITTER_TIME)
#define GPROXII_LONG_TIME_HIGH  (GPROXII_LONG_TIME + GPROXII_JITTER_TIME)

typedef struct {
    bool last_short;
    bool last_level;
    size_t encoded_index;
    uint8_t decoded_data[GPROXII_ENCODED_BYTE_FULL_SIZE];
    uint8_t data[GPROXII_ENCODED_BYTE_FULL_SIZE];
} ProtocolGProxII;

ProtocolGProxII* protocol_gproxii_alloc(void) {
    ProtocolGProxII* protocol = malloc(sizeof(ProtocolGProxII));
    return protocol;
}

void protocol_gproxii_free(ProtocolGProxII* protocol) {
    free(protocol);
}

uint8_t* protocol_gproxii_get_data(ProtocolGProxII* protocol) {
    return protocol->data;
}

bool wiegand_check(uint64_t fc_and_card, bool even_parity, bool odd_parity, int card_len) {
    uint8_t even_parity_sum = 0;
    uint8_t odd_parity_sum = 1;
    switch(card_len) {
    case 26:
        for(int8_t i = 12; i < 24; i++) {
            if(((fc_and_card >> i) & 1) == 1) {
                even_parity_sum++;
            }
        }
        if(even_parity_sum % 2 != even_parity) return false;

        for(int8_t i = 0; i < 12; i++) {
            if(((fc_and_card >> i) & 1) == 1) {
                odd_parity_sum++;
            }
        }
        if(odd_parity_sum % 2 != odd_parity) return false;
        break;
    case 36:
        for(int8_t i = 17; i < 34; i++) {
            if(((fc_and_card >> i) & 1) == 1) {
                even_parity_sum++;
            }
        }
        if(even_parity_sum % 2 != even_parity) return false;

        for(int8_t i = 0; i < 17; i++) {
            if(((fc_and_card >> i) & 1) == 1) {
                odd_parity_sum++;
            }
        }
        if(odd_parity_sum % 2 != odd_parity) return false;
        break;
    default:
        furi_crash();
    }
    return true;
}

void protocol_gproxii_decoder_start(ProtocolGProxII* protocol) {
    memset(protocol->data, 0, GPROXII_ENCODED_BYTE_FULL_SIZE);
    memset(protocol->decoded_data, 0, GPROXII_DATA_SIZE);
    protocol->last_short = false;
}

static bool protocol_gproxii_can_be_decoded(ProtocolGProxII* protocol) {
    // 96 bit with 5 bit zero parity
    // 0           10            20            30            40            50            60            70            80            90
    // |           |             |             |             |             |             |             |             |             |
    // 012345 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5 6789 0 1234 5
    // ------------------------------------------------------------------------------------------------------------------------------------
    // 111110 0000 0 1001 0 1101 0 1111 0 1000 0 1001 0 0000 0 1001 0 0000 0 1001 0 0000 0 1001 0 0000 0 1001 0 0000 0 1000 0 0000 0 1001 0

    // Remove header and reverse bytes on the remaining 72 bits
    //
    // 0          10         20         30          40         50         60         70
    // |          |          |          |           |          |          |          |
    // 01234567 89012345 67890123 45678901 23456789 01234567 89012345 67890123 45678901
    // --------------------------------------------------------------------------------
    // 00001001 11011111 10001001 00001001 00001001 00001001 00001001 00001000 00001001 - Without parity
    // 10010000 11111011 10010001 10010000 10010000 10010000 10010000 00010000 10010000 - Reversed
    // 10010000 01101011 00000001 00000000 00000000 00000000 00000000 10000000 00000000 - XOR all bytes from 1 using byte 0

    // 72 Bit Guardall/Verex/Chubb GProx II 26 bit key with 16 bit profile
    // 0          10          20        30          40         50          60        70
    // |          |           |         |           |          |           |         |
    // 01234567 890123 45 6789012345678901 2 34567890 1234567890123456 7 89012345678901
    // --------------------------------------------------------------------------------
    // XORVALUE LLLLLL DD PPPPPPPPPPPPPPPP E FFFFFFFF CCCCCCCCCCCCCCCC O UUUUUUUUUUUUUU
    // 10010000 011010 11 0000000100000000 0 00000000 0000000000000001 0 00000000000000 - Profile: 256 FC: 0 Card: 1

    // 72 Bit Guardall/Verex/Chubb GProx II 36 bit key with 16 bit profile
    // 0          10          20         30          40         50        60          70
    // |          |           |          |           |          |         |           |
    // 01234567 890123 45 67890123 45678901 2 34567890123456 78901234567890123456 7 8901
    // --------------------------------------------------------------------------------
    // XORVALUE LLLLLL DD PPPPPPPP PPPPPPPP E UUUUUUFFFFFFFF UUUUCCCCCCCCCCCCCCCC O UUUU
    // 10111000 100100 10 00000001 00000000 0 00000000010100 00001000100010111000 1 0000 - Profile: 256 FC: 20 Card: 35000

    // X = XOR Key, L = Message length, D = 2 bit check digits, P = Profile, E = Wiegand leading even parity
    // F = Faclity code, C = Card number, O = Wiegand trailing odd parity, U = Unused bits

    // Check 6 bits preamble 111110
    if(bit_lib_get_bits(protocol->data, 0, 6) != 0b111110) return false;

    // Check always 0 parity on every 5th bit after preamble
    if(!bit_lib_test_parity(protocol->data, 6, GPROXII_ENCODED_BIT_SIZE, BitLibParityAlways0, 5)) {
        return false;
    }

    // Start GProx II decode
    bit_lib_copy_bits(protocol->decoded_data, 0, GPROXII_ENCODED_BIT_SIZE, protocol->data, 6);

    // Remove parity
    bit_lib_remove_bit_every_nth(protocol->decoded_data, 0, GPROXII_ENCODED_BIT_SIZE, 5);

    // Reverse bytes
    for(int i = 0; i < 9; i++) {
        protocol->decoded_data[i] = bit_lib_reverse_8_fast(protocol->decoded_data[i]);
    }

    // DeXOR from byte 1 using byte 0
    for(int i = 1; i < 9; i++) {
        protocol->decoded_data[i] = protocol->decoded_data[0] ^ protocol->decoded_data[i];
    }

    // Check card length is either 26 or 36
    int card_len = bit_lib_get_bits(protocol->decoded_data, 8, 6);

    // wiegand parity
    if(card_len == 26) {
        uint64_t fc_and_card = bit_lib_get_bits_64(protocol->decoded_data, 33, 24);
        bool even_parity = bit_lib_get_bits(protocol->decoded_data, 32, 1);
        bool odd_parity = bit_lib_get_bits(protocol->decoded_data, 57, 1);
        if(!wiegand_check(fc_and_card, even_parity, odd_parity, card_len)) return false;
    } else if(card_len == 36) {
        uint64_t fc_and_card = bit_lib_get_bits_64(protocol->decoded_data, 33, 34);
        uint8_t even_parity = bit_lib_get_bits(protocol->decoded_data, 32, 1);
        uint8_t odd_parity = bit_lib_get_bits(protocol->decoded_data, 67, 1);
        if(!wiegand_check(fc_and_card, even_parity, odd_parity, card_len)) return false;
    } else {
        return false; // If we don't get a 26 or 36 it's not a known card type
    }

    return true;
}

bool protocol_gproxii_decoder_feed(ProtocolGProxII* protocol, bool level, uint32_t duration) {
    UNUSED(level);
    bool pushed = false;

    // Bi-Phase Manchester decoding inverse. Short = 1, Long = 0
    if(duration >= GPROXII_SHORT_TIME_LOW && duration <= GPROXII_SHORT_TIME_HIGH) {
        if(protocol->last_short == false) {
            protocol->last_short = true;
        } else {
            pushed = true;
            bit_lib_push_bit(protocol->data, GPROXII_ENCODED_BYTE_FULL_SIZE, true);
            protocol->last_short = false;
        }
    } else if(duration >= GPROXII_LONG_TIME_LOW && duration <= GPROXII_LONG_TIME_HIGH) {
        if(protocol->last_short == false) {
            pushed = true;
            bit_lib_push_bit(protocol->data, GPROXII_ENCODED_BYTE_FULL_SIZE, false);
        } else {
            // reset
            protocol->last_short = false;
        }
    } else {
        // reset
        protocol->last_short = false;
    }

    if(pushed && protocol_gproxii_can_be_decoded(protocol)) {
        return true;
    }

    return false;
}

bool protocol_gproxii_encoder_start(ProtocolGProxII* protocol) {
    protocol->encoded_index = 0;
    protocol->last_short = false;
    protocol->last_level = false;
    return true;
}

LevelDuration protocol_gproxii_encoder_yield(ProtocolGProxII* protocol) {
    uint32_t duration;
    protocol->last_level = !protocol->last_level;

    bool bit = bit_lib_get_bit(protocol->data, protocol->encoded_index);

    // Bi-Phase Manchester encoder inverted
    if(bit) {
        // two short pulses for 1
        duration = GPROXII_SHORT_TIME / 8;
        if(protocol->last_short) {
            bit_lib_increment_index(protocol->encoded_index, 96);
            protocol->last_short = false;
        } else {
            protocol->last_short = true;
        }
    } else {
        // one long pulse for 0
        duration = GPROXII_LONG_TIME / 8;
        bit_lib_increment_index(protocol->encoded_index, 96);
    }
    return level_duration_make(protocol->last_level, duration);
}

void protocol_gproxii_render_data(ProtocolGProxII* protocol, FuriString* result) {
    protocol_gproxii_can_be_decoded(protocol);
    int xor_code = bit_lib_get_bits(protocol->decoded_data, 0, 8);
    int card_len = bit_lib_get_bits(protocol->decoded_data, 8, 6);
    int crc_code = bit_lib_get_bits(protocol->decoded_data, 14, 2);

    if(card_len == 26) { // 26 Bit card
        // Print FC, Card and Length
        furi_string_cat_printf(
            result,
            "FC: %u Card: %u LEN: %hhu\n",
            bit_lib_get_bits(protocol->decoded_data, 33, 8),
            bit_lib_get_bits_16(protocol->decoded_data, 41, 16),
            card_len);
        // XOR Key, CRC and Profile
        furi_string_cat_printf(
            result,
            "XOR: %hhu CRC: %hhu P: %04hX",
            xor_code,
            crc_code,
            bit_lib_get_bits_16(protocol->decoded_data, 16, 16));
    } else if(card_len == 36) { // 36 Bit card
        // Print FC, Card and Length
        furi_string_cat_printf(
            result,
            "FC: %u Card: %u LEN: %hhu\n",
            bit_lib_get_bits_16(protocol->decoded_data, 33, 14),
            bit_lib_get_bits_16(protocol->decoded_data, 51, 16),
            card_len);
        // XOR Key, CRC and Profile
        furi_string_cat_printf(
            result,
            "XOR: %hhu CRC: %hhu P: %04hX",
            xor_code,
            crc_code,
            bit_lib_get_bits_16(protocol->decoded_data, 16, 16));
    } else {
        furi_string_cat_printf(result, "Read Error\n");
    }
}

bool protocol_gproxii_write_data(ProtocolGProxII* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_BIPHASE | LFRFID_T5577_BITRATE_RF_64 |
                                  (3 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->data, 64, 32);
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
}

const ProtocolBase protocol_gproxii = {
    .name = "GProxII",
    .manufacturer = "Guardall",
    .data_size = GPROXII_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_gproxii_alloc,
    .free = (ProtocolFree)protocol_gproxii_free,
    .get_data = (ProtocolGetData)protocol_gproxii_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_gproxii_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_gproxii_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_gproxii_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_gproxii_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_gproxii_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_gproxii_render_data,
    .write_data = (ProtocolWriteData)protocol_gproxii_write_data,
};
