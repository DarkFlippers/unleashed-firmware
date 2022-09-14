#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include "lfrfid_protocols.h"
#include <lfrfid/tools/bit_lib.h>

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define HID_DATA_SIZE 23
#define HID_PREAMBLE_SIZE 1

#define HID_ENCODED_DATA_SIZE (HID_PREAMBLE_SIZE + HID_DATA_SIZE + HID_PREAMBLE_SIZE)
#define HID_ENCODED_BIT_SIZE ((HID_PREAMBLE_SIZE + HID_DATA_SIZE) * 8)
#define HID_DECODED_DATA_SIZE (12)
#define HID_DECODED_BIT_SIZE ((HID_ENCODED_BIT_SIZE - HID_PREAMBLE_SIZE * 8) / 2)

#define HID_PREAMBLE 0x1D

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolHIDExDecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
    uint32_t pulse;
} ProtocolHIDExEncoder;

typedef struct {
    ProtocolHIDExDecoder decoder;
    ProtocolHIDExEncoder encoder;
    uint8_t encoded_data[HID_ENCODED_DATA_SIZE];
    uint8_t data[HID_DECODED_DATA_SIZE];
    size_t protocol_size;
} ProtocolHIDEx;

ProtocolHIDEx* protocol_hid_ex_generic_alloc(void) {
    ProtocolHIDEx* protocol = malloc(sizeof(ProtocolHIDEx));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
};

void protocol_hid_ex_generic_free(ProtocolHIDEx* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_hid_ex_generic_get_data(ProtocolHIDEx* protocol) {
    return protocol->data;
};

void protocol_hid_ex_generic_decoder_start(ProtocolHIDEx* protocol) {
    memset(protocol->encoded_data, 0, HID_ENCODED_DATA_SIZE);
};

static bool protocol_hid_ex_generic_can_be_decoded(const uint8_t* data) {
    // check preamble
    if(data[0] != HID_PREAMBLE || data[HID_PREAMBLE_SIZE + HID_DATA_SIZE] != HID_PREAMBLE) {
        return false;
    }

    // check for manchester encoding
    for(size_t i = HID_PREAMBLE_SIZE; i < (HID_PREAMBLE_SIZE + HID_DATA_SIZE); i++) {
        for(size_t n = 0; n < 4; n++) {
            uint8_t bit_pair = (data[i] >> (n * 2)) & 0b11;
            if(bit_pair == 0b11 || bit_pair == 0b00) {
                return false;
            }
        }
    }

    return true;
}

static void protocol_hid_ex_generic_decode(const uint8_t* from, uint8_t* to) {
    size_t bit_index = 0;
    for(size_t i = HID_PREAMBLE_SIZE; i < (HID_PREAMBLE_SIZE + HID_DATA_SIZE); i++) {
        for(size_t n = 0; n < 4; n++) {
            uint8_t bit_pair = (from[i] >> (6 - (n * 2))) & 0b11;
            if(bit_pair == 0b01) {
                bit_lib_set_bit(to, bit_index, 0);
            } else if(bit_pair == 0b10) {
                bit_lib_set_bit(to, bit_index, 1);
            }
            bit_index++;
        }
    }
}

bool protocol_hid_ex_generic_decoder_feed(ProtocolHIDEx* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;
    bool result = false;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            bit_lib_push_bit(protocol->encoded_data, HID_ENCODED_DATA_SIZE, value);
            if(protocol_hid_ex_generic_can_be_decoded(protocol->encoded_data)) {
                protocol_hid_ex_generic_decode(protocol->encoded_data, protocol->data);
                result = true;
            }
        }
    }

    return result;
};

static void protocol_hid_ex_generic_encode(ProtocolHIDEx* protocol) {
    protocol->encoded_data[0] = HID_PREAMBLE;

    size_t bit_index = 0;
    for(size_t i = 0; i < HID_DECODED_BIT_SIZE; i++) {
        bool bit = bit_lib_get_bit(protocol->data, i);
        if(bit) {
            bit_lib_set_bit(protocol->encoded_data, 8 + bit_index, 1);
            bit_lib_set_bit(protocol->encoded_data, 8 + bit_index + 1, 0);
        } else {
            bit_lib_set_bit(protocol->encoded_data, 8 + bit_index, 0);
            bit_lib_set_bit(protocol->encoded_data, 8 + bit_index + 1, 1);
        }
        bit_index += 2;
    }
}

bool protocol_hid_ex_generic_encoder_start(ProtocolHIDEx* protocol) {
    protocol->encoder.encoded_index = 0;
    protocol->encoder.pulse = 0;
    protocol_hid_ex_generic_encode(protocol);

    return true;
};

LevelDuration protocol_hid_ex_generic_encoder_yield(ProtocolHIDEx* protocol) {
    bool level = 0;
    uint32_t duration = 0;

    // if pulse is zero, we need to output high, otherwise we need to output low
    if(protocol->encoder.pulse == 0) {
        // get bit
        uint8_t bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);

        // get pulse from oscillator
        bool advance = fsk_osc_next(protocol->encoder.fsk_osc, bit, &duration);

        if(advance) {
            bit_lib_increment_index(protocol->encoder.encoded_index, HID_ENCODED_BIT_SIZE);
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

bool protocol_hid_ex_generic_write_data(ProtocolHIDEx* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_hid_ex_generic_encoder_start(protocol);
    protocol_hid_ex_generic_decode(protocol->encoded_data, protocol->data);

    protocol_hid_ex_generic_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_FSK2a | LFRFID_T5577_BITRATE_RF_50 |
                                  (6 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.block[4] = bit_lib_get_bits_32(protocol->encoded_data, 96, 32);
        request->t5577.block[5] = bit_lib_get_bits_32(protocol->encoded_data, 128, 32);
        request->t5577.block[6] = bit_lib_get_bits_32(protocol->encoded_data, 160, 32);
        request->t5577.blocks_to_write = 7;
        result = true;
    }
    return result;
};

void protocol_hid_ex_generic_render_data(ProtocolHIDEx* protocol, string_t result) {
    // TODO: parser and render functions
    UNUSED(protocol);
    string_printf(result, "Generic HID Extended\r\nData: Unknown");
};

const ProtocolBase protocol_hid_ex_generic = {
    .name = "HIDExt",
    .manufacturer = "Generic",
    .data_size = HID_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_hid_ex_generic_alloc,
    .free = (ProtocolFree)protocol_hid_ex_generic_free,
    .get_data = (ProtocolGetData)protocol_hid_ex_generic_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_hid_ex_generic_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_hid_ex_generic_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_hid_ex_generic_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_hid_ex_generic_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_hid_ex_generic_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_hid_ex_generic_render_data,
    .write_data = (ProtocolWriteData)protocol_hid_ex_generic_write_data,
};