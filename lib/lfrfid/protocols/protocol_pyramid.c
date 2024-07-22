#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include "lfrfid_protocols.h"
#include <bit_lib/bit_lib.h>

#define JITTER_TIME (20)
#define MIN_TIME    (64 - JITTER_TIME)
#define MAX_TIME    (80 + JITTER_TIME)

#define PYRAMID_DATA_SIZE     13
#define PYRAMID_PREAMBLE_SIZE 3

#define PYRAMID_ENCODED_DATA_SIZE \
    (PYRAMID_PREAMBLE_SIZE + PYRAMID_DATA_SIZE + PYRAMID_PREAMBLE_SIZE)
#define PYRAMID_ENCODED_BIT_SIZE  ((PYRAMID_PREAMBLE_SIZE + PYRAMID_DATA_SIZE) * 8)
#define PYRAMID_DECODED_DATA_SIZE (4)
#define PYRAMID_DECODED_BIT_SIZE  ((PYRAMID_ENCODED_BIT_SIZE - PYRAMID_PREAMBLE_SIZE * 8) / 2)

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolPyramidDecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
    uint32_t pulse;
} ProtocolPyramidEncoder;

typedef struct {
    ProtocolPyramidDecoder decoder;
    ProtocolPyramidEncoder encoder;
    uint8_t encoded_data[PYRAMID_ENCODED_DATA_SIZE];
    uint8_t data[PYRAMID_DECODED_DATA_SIZE];
} ProtocolPyramid;

ProtocolPyramid* protocol_pyramid_alloc(void) {
    ProtocolPyramid* protocol = malloc(sizeof(ProtocolPyramid));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
}

void protocol_pyramid_free(ProtocolPyramid* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
}

uint8_t* protocol_pyramid_get_data(ProtocolPyramid* protocol) {
    return protocol->data;
}

void protocol_pyramid_decoder_start(ProtocolPyramid* protocol) {
    memset(protocol->encoded_data, 0, PYRAMID_ENCODED_DATA_SIZE);
}

static bool protocol_pyramid_can_be_decoded(uint8_t* data) {
    // check preamble
    if(bit_lib_get_bits_16(data, 0, 16) != 0b0000000000000001 ||
       bit_lib_get_bits(data, 16, 8) != 0b00000001) {
        return false;
    }

    if(bit_lib_get_bits_16(data, 128, 16) != 0b0000000000000001 ||
       bit_lib_get_bits(data, 136, 8) != 0b00000001) {
        return false;
    }

    uint8_t checksum = bit_lib_get_bits(data, 120, 8);
    uint8_t checksum_data[13] = {0x00};
    for(uint8_t i = 0; i < 13; i++) {
        checksum_data[i] = bit_lib_get_bits(data, 16 + (i * 8), 8);
    }

    uint8_t calc_checksum = bit_lib_crc8(checksum_data, 13, 0x31, 0x00, true, true, 0x00);
    if(checksum != calc_checksum) return false;

    // Remove parity
    bit_lib_remove_bit_every_nth(data, 8, 15 * 8, 8);

    // Determine Startbit and format
    int j;
    for(j = 0; j < 105; ++j) {
        if(bit_lib_get_bit(data, j)) break;
    }
    uint8_t fmt_len = 105 - j;

    // Only support 26bit format for now
    if(fmt_len != 26) return false;

    return true;
}

static void protocol_pyramid_decode(ProtocolPyramid* protocol) {
    // Format
    bit_lib_set_bits(protocol->data, 0, 26, 8);

    // Facility Code
    bit_lib_copy_bits(protocol->data, 8, 8, protocol->encoded_data, 73 + 8);

    // Card Number
    bit_lib_copy_bits(protocol->data, 16, 16, protocol->encoded_data, 81 + 8);
}

bool protocol_pyramid_decoder_feed(ProtocolPyramid* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;
    bool result = false;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            bit_lib_push_bit(protocol->encoded_data, PYRAMID_ENCODED_DATA_SIZE, value);
            if(protocol_pyramid_can_be_decoded(protocol->encoded_data)) {
                protocol_pyramid_decode(protocol);
                result = true;
            }
        }
    }

    return result;
}

bool protocol_pyramid_get_parity(const uint8_t* bits, uint8_t type, int length) {
    int x;
    for(x = 0; length > 0; --length)
        x += bit_lib_get_bit(bits, length - 1);
    x %= 2;
    return x ^ type;
}

void protocol_pyramid_add_wiegand_parity(
    uint8_t* target,
    uint8_t target_position,
    uint8_t* source,
    uint8_t length) {
    bit_lib_set_bit(
        target, target_position, protocol_pyramid_get_parity(source, 0 /* even */, length / 2));
    bit_lib_copy_bits(target, target_position + 1, length, source, 0);
    bit_lib_set_bit(
        target,
        target_position + length + 1,
        protocol_pyramid_get_parity(source + length / 2, 1 /* odd */, length / 2));
}

static void protocol_pyramid_encode(ProtocolPyramid* protocol) {
    memset(protocol->encoded_data, 0, sizeof(protocol->encoded_data));

    uint8_t pre[16];
    memset(pre, 0, sizeof(pre));

    // Format start bit
    bit_lib_set_bit(pre, 79, 1);

    uint8_t wiegand[3];
    memset(wiegand, 0, sizeof(wiegand));

    // FC
    bit_lib_copy_bits(wiegand, 0, 8, protocol->data, 8);

    // CardNum
    bit_lib_copy_bits(wiegand, 8, 16, protocol->data, 16);

    // Wiegand parity
    protocol_pyramid_add_wiegand_parity(pre, 80, wiegand, 24);

    bit_lib_add_parity(pre, 8, protocol->encoded_data, 8, 102, 8, 1);

    // Add checksum
    uint8_t checksum_buffer[13];
    for(uint8_t i = 0; i < 13; i++)
        checksum_buffer[i] = bit_lib_get_bits(protocol->encoded_data, 16 + (i * 8), 8);

    uint8_t crc = bit_lib_crc8(checksum_buffer, 13, 0x31, 0x00, true, true, 0x00);
    bit_lib_set_bits(protocol->encoded_data, 120, crc, 8);
}

bool protocol_pyramid_encoder_start(ProtocolPyramid* protocol) {
    protocol->encoder.encoded_index = 0;
    protocol->encoder.pulse = 0;
    protocol_pyramid_encode(protocol);

    return true;
}

LevelDuration protocol_pyramid_encoder_yield(ProtocolPyramid* protocol) {
    bool level = 0;
    uint32_t duration = 0;

    // if pulse is zero, we need to output high, otherwise we need to output low
    if(protocol->encoder.pulse == 0) {
        // get bit
        uint8_t bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);

        // get pulse from oscillator
        bool advance = fsk_osc_next(protocol->encoder.fsk_osc, bit, &duration);

        if(advance) {
            bit_lib_increment_index(protocol->encoder.encoded_index, PYRAMID_ENCODED_BIT_SIZE);
        }

        // duration diveded by 2 because we need to output high and low
        duration = duration / 2;
        protocol->encoder.pulse = duration;
        level = true;
    } else {
        // output low half and reset pulse
        duration = protocol->encoder.pulse;
        protocol->encoder.pulse = 0;
        level = false;
    }

    return level_duration_make(level, duration);
}

bool protocol_pyramid_write_data(ProtocolPyramid* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_pyramid_encode(protocol);
    bit_lib_remove_bit_every_nth(protocol->encoded_data, 8, 15 * 8, 8);
    protocol_pyramid_decode(protocol);

    protocol_pyramid_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_FSK2a | LFRFID_T5577_BITRATE_RF_50 |
                                  (4 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.block[4] = bit_lib_get_bits_32(protocol->encoded_data, 96, 32);
        request->t5577.blocks_to_write = 5;
        result = true;
    }
    return result;
}

void protocol_pyramid_render_data(ProtocolPyramid* protocol, FuriString* result) {
    uint8_t* decoded_data = protocol->data;
    uint8_t format_length = decoded_data[0];

    furi_string_printf(result, "Format: %hhu\n", format_length);
    if(format_length == 26) {
        uint8_t facility;
        bit_lib_copy_bits(&facility, 0, 8, decoded_data, 8);

        uint16_t card_id;
        bit_lib_copy_bits((uint8_t*)&card_id, 8, 8, decoded_data, 16);
        bit_lib_copy_bits((uint8_t*)&card_id, 0, 8, decoded_data, 24);
        furi_string_cat_printf(result, "FC: %03hhu; Card: %05hu", facility, card_id);
    } else {
        furi_string_cat_printf(result, "Data: Unknown");
    }
}

const ProtocolBase protocol_pyramid = {
    .name = "Pyramid",
    .manufacturer = "Farpointe",
    .data_size = PYRAMID_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_pyramid_alloc,
    .free = (ProtocolFree)protocol_pyramid_free,
    .get_data = (ProtocolGetData)protocol_pyramid_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_pyramid_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_pyramid_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_pyramid_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_pyramid_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_pyramid_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_pyramid_render_data,
    .write_data = (ProtocolWriteData)protocol_pyramid_write_data,
};
