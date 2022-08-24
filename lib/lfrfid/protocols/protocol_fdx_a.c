#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include "lfrfid_protocols.h"
#include <lfrfid/tools/bit_lib.h>

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define FDXA_DATA_SIZE 10
#define FDXA_PREAMBLE_SIZE 2

#define FDXA_ENCODED_DATA_SIZE (FDXA_PREAMBLE_SIZE + FDXA_DATA_SIZE + FDXA_PREAMBLE_SIZE)
#define FDXA_ENCODED_BIT_SIZE ((FDXA_PREAMBLE_SIZE + FDXA_DATA_SIZE) * 8)
#define FDXA_DECODED_DATA_SIZE (5)
#define FDXA_DECODED_BIT_SIZE ((FDXA_ENCODED_BIT_SIZE - FDXA_PREAMBLE_SIZE * 8) / 2)

#define FDXA_PREAMBLE_0 0x55
#define FDXA_PREAMBLE_1 0x1D

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolFDXADecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
    uint32_t pulse;
} ProtocolFDXAEncoder;

typedef struct {
    ProtocolFDXADecoder decoder;
    ProtocolFDXAEncoder encoder;
    uint8_t encoded_data[FDXA_ENCODED_DATA_SIZE];
    uint8_t data[FDXA_DECODED_DATA_SIZE];
    size_t protocol_size;
} ProtocolFDXA;

ProtocolFDXA* protocol_fdx_a_alloc(void) {
    ProtocolFDXA* protocol = malloc(sizeof(ProtocolFDXA));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
};

void protocol_fdx_a_free(ProtocolFDXA* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_fdx_a_get_data(ProtocolFDXA* protocol) {
    return protocol->data;
};

void protocol_fdx_a_decoder_start(ProtocolFDXA* protocol) {
    memset(protocol->encoded_data, 0, FDXA_ENCODED_DATA_SIZE);
};

static bool protocol_fdx_a_decode(const uint8_t* from, uint8_t* to) {
    size_t bit_index = 0;
    for(size_t i = FDXA_PREAMBLE_SIZE; i < (FDXA_PREAMBLE_SIZE + FDXA_DATA_SIZE); i++) {
        for(size_t n = 0; n < 4; n++) {
            uint8_t bit_pair = (from[i] >> (6 - (n * 2))) & 0b11;
            if(bit_pair == 0b01) {
                bit_lib_set_bit(to, bit_index, 0);
            } else if(bit_pair == 0b10) {
                bit_lib_set_bit(to, bit_index, 1);
            } else {
                return false;
            }
            bit_index++;
        }
    }

    return true;
}

static bool protocol_fdx_a_can_be_decoded(const uint8_t* data) {
    // check preamble
    if(data[0] != FDXA_PREAMBLE_0 || data[1] != FDXA_PREAMBLE_1 || data[12] != FDXA_PREAMBLE_0 ||
       data[13] != FDXA_PREAMBLE_1) {
        return false;
    }

    // check for manchester encoding
    uint8_t decoded_data[FDXA_DECODED_DATA_SIZE];
    if(!protocol_fdx_a_decode(data, decoded_data)) return false;

    uint8_t parity_sum = 0;
    for(size_t i = 0; i < FDXA_DECODED_DATA_SIZE; i++) {
        parity_sum += bit_lib_test_parity_32(decoded_data[i], BitLibParityOdd);
        decoded_data[i] &= 0x7F;
    }

    return (parity_sum == 0);
}

bool protocol_fdx_a_decoder_feed(ProtocolFDXA* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;
    bool result = false;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            bit_lib_push_bit(protocol->encoded_data, FDXA_ENCODED_DATA_SIZE, value);
            if(protocol_fdx_a_can_be_decoded(protocol->encoded_data)) {
                protocol_fdx_a_decode(protocol->encoded_data, protocol->data);
                result = true;
            }
        }
    }

    return result;
};

static void protocol_fdx_a_encode(ProtocolFDXA* protocol) {
    protocol->encoded_data[0] = FDXA_PREAMBLE_0;
    protocol->encoded_data[1] = FDXA_PREAMBLE_1;

    size_t bit_index = 0;
    for(size_t i = 0; i < FDXA_DECODED_BIT_SIZE; i++) {
        bool bit = bit_lib_get_bit(protocol->data, i);
        if(bit) {
            bit_lib_set_bit(protocol->encoded_data, 16 + bit_index, 1);
            bit_lib_set_bit(protocol->encoded_data, 16 + bit_index + 1, 0);
        } else {
            bit_lib_set_bit(protocol->encoded_data, 16 + bit_index, 0);
            bit_lib_set_bit(protocol->encoded_data, 16 + bit_index + 1, 1);
        }
        bit_index += 2;
    }
}

bool protocol_fdx_a_encoder_start(ProtocolFDXA* protocol) {
    protocol->encoder.encoded_index = 0;
    protocol->encoder.pulse = 0;
    protocol_fdx_a_encode(protocol);

    return true;
};

LevelDuration protocol_fdx_a_encoder_yield(ProtocolFDXA* protocol) {
    bool level = 0;
    uint32_t duration = 0;

    // if pulse is zero, we need to output high, otherwise we need to output low
    if(protocol->encoder.pulse == 0) {
        // get bit
        uint8_t bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);

        // get pulse from oscillator
        bool advance = fsk_osc_next(protocol->encoder.fsk_osc, bit, &duration);

        if(advance) {
            bit_lib_increment_index(protocol->encoder.encoded_index, FDXA_ENCODED_BIT_SIZE);
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
};

bool protocol_fdx_a_write_data(ProtocolFDXA* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_fdx_a_encoder_start(protocol);

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

void protocol_fdx_a_render_data(ProtocolFDXA* protocol, string_t result) {
    uint8_t data[FDXA_DECODED_DATA_SIZE];
    memcpy(data, protocol->data, FDXA_DECODED_DATA_SIZE);

    uint8_t parity_sum = 0;
    for(size_t i = 0; i < FDXA_DECODED_DATA_SIZE; i++) {
        parity_sum += bit_lib_test_parity_32(data[i], BitLibParityOdd);
        data[i] &= 0x7F;
    }

    string_printf(
        result,
        "ID: %02X%02X%02X%02X%02X\r\n"
        "Parity: %s",
        data[0],
        data[1],
        data[2],
        data[3],
        data[4],
        parity_sum == 0 ? "+" : "-");
};

const ProtocolBase protocol_fdx_a = {
    .name = "FDX-A",
    .manufacturer = "FECAVA",
    .data_size = FDXA_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_fdx_a_alloc,
    .free = (ProtocolFree)protocol_fdx_a_free,
    .get_data = (ProtocolGetData)protocol_fdx_a_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_fdx_a_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_fdx_a_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_fdx_a_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_fdx_a_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_fdx_a_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_fdx_a_render_data,
    .write_data = (ProtocolWriteData)protocol_fdx_a_write_data,
};