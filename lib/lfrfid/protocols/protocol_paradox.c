#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include <lfrfid/tools/bit_lib.h>
#include "lfrfid_protocols.h"

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define PARADOX_DECODED_DATA_SIZE (6)

#define PARADOX_PREAMBLE_LENGTH (8)
#define PARADOX_ENCODED_BIT_SIZE (96)
#define PARADOX_ENCODED_DATA_SIZE (((PARADOX_ENCODED_BIT_SIZE) / 8) + 1)
#define PARADOX_ENCODED_DATA_LAST (PARADOX_ENCODED_DATA_SIZE - 1)

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolParadoxDecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
} ProtocolParadoxEncoder;

typedef struct {
    ProtocolParadoxDecoder decoder;
    ProtocolParadoxEncoder encoder;
    uint8_t encoded_data[PARADOX_ENCODED_DATA_SIZE];
    uint8_t data[PARADOX_DECODED_DATA_SIZE];
} ProtocolParadox;

ProtocolParadox* protocol_paradox_alloc(void) {
    ProtocolParadox* protocol = malloc(sizeof(ProtocolParadox));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
};

void protocol_paradox_free(ProtocolParadox* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_paradox_get_data(ProtocolParadox* protocol) {
    return protocol->data;
};

void protocol_paradox_decoder_start(ProtocolParadox* protocol) {
    memset(protocol->encoded_data, 0, PARADOX_ENCODED_DATA_SIZE);
};

static bool protocol_paradox_can_be_decoded(ProtocolParadox* protocol) {
    // check preamble
    if(protocol->encoded_data[0] != 0b00001111 ||
       protocol->encoded_data[PARADOX_ENCODED_DATA_LAST] != 0b00001111)
        return false;

    for(uint32_t i = PARADOX_PREAMBLE_LENGTH; i < 96; i += 2) {
        if(bit_lib_get_bit(protocol->encoded_data, i) ==
           bit_lib_get_bit(protocol->encoded_data, i + 1)) {
            return false;
        }
    }

    return true;
}

static void protocol_paradox_decode(uint8_t* encoded_data, uint8_t* decoded_data) {
    for(uint32_t i = PARADOX_PREAMBLE_LENGTH; i < 96; i += 2) {
        if(bit_lib_get_bits(encoded_data, i, 2) == 0b01) {
            bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 0);
        } else if(bit_lib_get_bits(encoded_data, i, 2) == 0b10) {
            bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 1);
        }
    }
    bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 0);
    bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 0);
    bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 0);
    bit_lib_push_bit(decoded_data, PARADOX_DECODED_DATA_SIZE, 0);
}

bool protocol_paradox_decoder_feed(ProtocolParadox* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            bit_lib_push_bit(protocol->encoded_data, PARADOX_ENCODED_DATA_SIZE, value);
            if(protocol_paradox_can_be_decoded(protocol)) {
                protocol_paradox_decode(protocol->encoded_data, protocol->data);

                return true;
            }
        }
    }

    return false;
};

static void protocol_paradox_encode(const uint8_t* decoded_data, uint8_t* encoded_data) {
    // preamble
    bit_lib_set_bits(encoded_data, 0, 0b00001111, 8);

    for(size_t i = 0; i < 44; i++) {
        if(bit_lib_get_bit(decoded_data, i)) {
            bit_lib_set_bits(encoded_data, PARADOX_PREAMBLE_LENGTH + i * 2, 0b10, 2);
        } else {
            bit_lib_set_bits(encoded_data, PARADOX_PREAMBLE_LENGTH + i * 2, 0b01, 2);
        }
    }
};

bool protocol_paradox_encoder_start(ProtocolParadox* protocol) {
    protocol_paradox_encode(protocol->data, (uint8_t*)protocol->encoded_data);
    protocol->encoder.encoded_index = 0;
    fsk_osc_reset(protocol->encoder.fsk_osc);
    return true;
};

LevelDuration protocol_paradox_encoder_yield(ProtocolParadox* protocol) {
    bool level;
    uint32_t duration;

    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);
    bool advance = fsk_osc_next_half(protocol->encoder.fsk_osc, bit, &level, &duration);

    if(advance) {
        bit_lib_increment_index(protocol->encoder.encoded_index, PARADOX_ENCODED_BIT_SIZE);
    }
    return level_duration_make(level, duration);
};

void protocol_paradox_render_data(ProtocolParadox* protocol, string_t result) {
    uint8_t* decoded_data = protocol->data;
    uint8_t fc = bit_lib_get_bits(decoded_data, 10, 8);
    uint16_t card_id = bit_lib_get_bits_16(decoded_data, 18, 16);

    string_cat_printf(result, "Facility: %u\r\n", fc);
    string_cat_printf(result, "Card: %lu\r\n", card_id);
    string_cat_printf(result, "Data: ");
    for(size_t i = 0; i < PARADOX_DECODED_DATA_SIZE; i++) {
        string_cat_printf(result, "%02X", decoded_data[i]);
    }
};

void protocol_paradox_render_brief_data(ProtocolParadox* protocol, string_t result) {
    uint8_t* decoded_data = protocol->data;

    uint8_t fc = bit_lib_get_bits(decoded_data, 10, 8);
    uint16_t card_id = bit_lib_get_bits_16(decoded_data, 18, 16);

    string_cat_printf(result, "FC: %03u, Card: %05u", fc, card_id);
};

bool protocol_paradox_write_data(ProtocolParadox* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_paradox_encode(protocol->data, (uint8_t*)protocol->encoded_data);
    protocol_paradox_decode(protocol->encoded_data, protocol->data);

    protocol_paradox_encode(protocol->data, (uint8_t*)protocol->encoded_data);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_FSK2a | LFRFID_T5577_BITRATE_RF_50 |
                                  (3 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
};

const ProtocolBase protocol_paradox = {
    .name = "Paradox",
    .manufacturer = "Paradox",
    .data_size = PARADOX_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_paradox_alloc,
    .free = (ProtocolFree)protocol_paradox_free,
    .get_data = (ProtocolGetData)protocol_paradox_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_paradox_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_paradox_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_paradox_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_paradox_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_paradox_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_paradox_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_paradox_write_data,
};