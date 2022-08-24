#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include "lfrfid_protocols.h"

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define H10301_DECODED_DATA_SIZE (3)
#define H10301_ENCODED_DATA_SIZE_U32 (3)
#define H10301_ENCODED_DATA_SIZE (sizeof(uint32_t) * H10301_ENCODED_DATA_SIZE_U32)

#define H10301_BIT_SIZE (sizeof(uint32_t) * 8)
#define H10301_BIT_MAX_SIZE (H10301_BIT_SIZE * H10301_DECODED_DATA_SIZE)

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolH10301Decoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
    uint32_t pulse;
} ProtocolH10301Encoder;

typedef struct {
    ProtocolH10301Decoder decoder;
    ProtocolH10301Encoder encoder;
    uint32_t encoded_data[H10301_ENCODED_DATA_SIZE_U32];
    uint8_t data[H10301_DECODED_DATA_SIZE];
} ProtocolH10301;

ProtocolH10301* protocol_h10301_alloc(void) {
    ProtocolH10301* protocol = malloc(sizeof(ProtocolH10301));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
};

void protocol_h10301_free(ProtocolH10301* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_h10301_get_data(ProtocolH10301* protocol) {
    return protocol->data;
};

void protocol_h10301_decoder_start(ProtocolH10301* protocol) {
    memset(protocol->encoded_data, 0, sizeof(uint32_t) * 3);
};

static void protocol_h10301_decoder_store_data(ProtocolH10301* protocol, bool data) {
    protocol->encoded_data[0] = (protocol->encoded_data[0] << 1) |
                                ((protocol->encoded_data[1] >> 31) & 1);
    protocol->encoded_data[1] = (protocol->encoded_data[1] << 1) |
                                ((protocol->encoded_data[2] >> 31) & 1);
    protocol->encoded_data[2] = (protocol->encoded_data[2] << 1) | data;
}

static bool protocol_h10301_can_be_decoded(const uint32_t* card_data) {
    const uint8_t* encoded_data = (const uint8_t*)card_data;

    // packet preamble
    // raw data
    if(*(encoded_data + 3) != 0x1D) {
        return false;
    }

    // encoded company/oem
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0
    if((*card_data >> 10 & 0x3FFF) != 0x1556) {
        return false;
    }

    // encoded format/length
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0 and word 1
    if((((*card_data & 0x3FF) << 12) | ((*(card_data + 1) >> 20) & 0xFFF)) != 0x155556) {
        return false;
    }

    // data decoding
    uint32_t result = 0;

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 9; i >= 0; i--) {
        switch((*(card_data + 1) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return false;
            break;
        }
    }

    // decode from word 2
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
        switch((*(card_data + 2) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return false;
            break;
        }
    }

    // trailing parity (odd) test
    uint8_t parity_sum = 0;
    for(int8_t i = 0; i < 13; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) != 1) {
        return false;
    }

    // leading parity (even) test
    parity_sum = 0;
    for(int8_t i = 13; i < 26; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) == 1) {
        return false;
    }

    return true;
}

static void protocol_h10301_decode(const uint32_t* card_data, uint8_t* decoded_data) {
    // data decoding
    uint32_t result = 0;

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 9; i >= 0; i--) {
        switch((*(card_data + 1) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            break;
        }
    }

    // decode from word 2
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
        switch((*(card_data + 2) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            break;
        }
    }

    uint8_t data[H10301_DECODED_DATA_SIZE] = {
        (uint8_t)(result >> 17), (uint8_t)(result >> 9), (uint8_t)(result >> 1)};

    memcpy(decoded_data, &data, H10301_DECODED_DATA_SIZE);
}

bool protocol_h10301_decoder_feed(ProtocolH10301* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;
    bool result = false;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            protocol_h10301_decoder_store_data(protocol, value);
            if(protocol_h10301_can_be_decoded(protocol->encoded_data)) {
                protocol_h10301_decode(protocol->encoded_data, protocol->data);
                result = true;
                break;
            }
        }
    }

    return result;
};

static void protocol_h10301_write_raw_bit(bool bit, uint8_t position, uint32_t* card_data) {
    if(bit) {
        card_data[position / H10301_BIT_SIZE] |=
            1UL << (H10301_BIT_SIZE - (position % H10301_BIT_SIZE) - 1);
    } else {
        card_data[position / H10301_BIT_SIZE] &=
            ~(1UL << (H10301_BIT_SIZE - (position % H10301_BIT_SIZE) - 1));
    }
}

static void protocol_h10301_write_bit(bool bit, uint8_t position, uint32_t* card_data) {
    protocol_h10301_write_raw_bit(bit, position + 0, card_data);
    protocol_h10301_write_raw_bit(!bit, position + 1, card_data);
}

void protocol_h10301_encode(const uint8_t* decoded_data, uint8_t* encoded_data) {
    uint32_t card_data[H10301_DECODED_DATA_SIZE] = {0, 0, 0};

    uint32_t fc_cn = (decoded_data[0] << 16) | (decoded_data[1] << 8) | decoded_data[2];

    // even parity sum calculation (high 12 bits of data)
    uint8_t even_parity_sum = 0;
    for(int8_t i = 12; i < 24; i++) {
        if(((fc_cn >> i) & 1) == 1) {
            even_parity_sum++;
        }
    }

    // odd parity sum calculation (low 12 bits of data)
    uint8_t odd_parity_sum = 1;
    for(int8_t i = 0; i < 12; i++) {
        if(((fc_cn >> i) & 1) == 1) {
            odd_parity_sum++;
        }
    }

    // 0x1D preamble
    protocol_h10301_write_raw_bit(0, 0, card_data);
    protocol_h10301_write_raw_bit(0, 1, card_data);
    protocol_h10301_write_raw_bit(0, 2, card_data);
    protocol_h10301_write_raw_bit(1, 3, card_data);
    protocol_h10301_write_raw_bit(1, 4, card_data);
    protocol_h10301_write_raw_bit(1, 5, card_data);
    protocol_h10301_write_raw_bit(0, 6, card_data);
    protocol_h10301_write_raw_bit(1, 7, card_data);

    // company / OEM code 1
    protocol_h10301_write_bit(0, 8, card_data);
    protocol_h10301_write_bit(0, 10, card_data);
    protocol_h10301_write_bit(0, 12, card_data);
    protocol_h10301_write_bit(0, 14, card_data);
    protocol_h10301_write_bit(0, 16, card_data);
    protocol_h10301_write_bit(0, 18, card_data);
    protocol_h10301_write_bit(1, 20, card_data);

    // card format / length 1
    protocol_h10301_write_bit(0, 22, card_data);
    protocol_h10301_write_bit(0, 24, card_data);
    protocol_h10301_write_bit(0, 26, card_data);
    protocol_h10301_write_bit(0, 28, card_data);
    protocol_h10301_write_bit(0, 30, card_data);
    protocol_h10301_write_bit(0, 32, card_data);
    protocol_h10301_write_bit(0, 34, card_data);
    protocol_h10301_write_bit(0, 36, card_data);
    protocol_h10301_write_bit(0, 38, card_data);
    protocol_h10301_write_bit(0, 40, card_data);
    protocol_h10301_write_bit(1, 42, card_data);

    // even parity bit
    protocol_h10301_write_bit((even_parity_sum % 2), 44, card_data);

    // data
    for(uint8_t i = 0; i < 24; i++) {
        protocol_h10301_write_bit((fc_cn >> (23 - i)) & 1, 46 + (i * 2), card_data);
    }

    // odd parity bit
    protocol_h10301_write_bit((odd_parity_sum % 2), 94, card_data);

    memcpy(encoded_data, &card_data, H10301_ENCODED_DATA_SIZE);
}

bool protocol_h10301_encoder_start(ProtocolH10301* protocol) {
    protocol_h10301_encode(protocol->data, (uint8_t*)protocol->encoded_data);
    protocol->encoder.encoded_index = 0;
    protocol->encoder.pulse = 0;

    return true;
};

LevelDuration protocol_h10301_encoder_yield(ProtocolH10301* protocol) {
    bool level = 0;
    uint32_t duration = 0;

    // if pulse is zero, we need to output high, otherwise we need to output low
    if(protocol->encoder.pulse == 0) {
        // get bit
        uint8_t bit =
            (protocol->encoded_data[protocol->encoder.encoded_index / H10301_BIT_SIZE] >>
             ((H10301_BIT_SIZE - 1) - (protocol->encoder.encoded_index % H10301_BIT_SIZE))) &
            1;

        // get pulse from oscillator
        bool advance = fsk_osc_next(protocol->encoder.fsk_osc, bit, &duration);

        if(advance) {
            protocol->encoder.encoded_index++;
            if(protocol->encoder.encoded_index >= (H10301_BIT_MAX_SIZE)) {
                protocol->encoder.encoded_index = 0;
            }
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

bool protocol_h10301_write_data(ProtocolH10301* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_h10301_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_FSK2a | LFRFID_T5577_BITRATE_RF_50 |
                                  (3 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = protocol->encoded_data[0];
        request->t5577.block[2] = protocol->encoded_data[1];
        request->t5577.block[3] = protocol->encoded_data[2];
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
};

void protocol_h10301_render_data(ProtocolH10301* protocol, string_t result) {
    uint8_t* data = protocol->data;
    string_printf(
        result,
        "FC: %u\r\n"
        "Card: %u",
        data[0],
        (uint16_t)((data[1] << 8) | (data[2])));
};

const ProtocolBase protocol_h10301 = {
    .name = "H10301",
    .manufacturer = "HID",
    .data_size = H10301_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_h10301_alloc,
    .free = (ProtocolFree)protocol_h10301_free,
    .get_data = (ProtocolGetData)protocol_h10301_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_h10301_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_h10301_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_h10301_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_h10301_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_h10301_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_h10301_render_data,
    .write_data = (ProtocolWriteData)protocol_h10301_write_data,
};