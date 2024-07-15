#include <furi.h>
#include "toolbox/level_duration.h"
#include "protocol_jablotron.h"
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define JABLOTRON_ENCODED_BIT_SIZE   (64)
#define JABLOTRON_ENCODED_BYTE_SIZE  (((JABLOTRON_ENCODED_BIT_SIZE) / 8))
#define JABLOTRON_PREAMBLE_BIT_SIZE  (16)
#define JABLOTRON_PREAMBLE_BYTE_SIZE (2)
#define JABLOTRON_ENCODED_BYTE_FULL_SIZE \
    (JABLOTRON_ENCODED_BYTE_SIZE + JABLOTRON_PREAMBLE_BYTE_SIZE)

#define JABLOTRON_DECODED_DATA_SIZE (5)

#define JABLOTRON_SHORT_TIME  (256)
#define JABLOTRON_LONG_TIME   (512)
#define JABLOTRON_JITTER_TIME (120)

#define JABLOTRON_SHORT_TIME_LOW  (JABLOTRON_SHORT_TIME - JABLOTRON_JITTER_TIME)
#define JABLOTRON_SHORT_TIME_HIGH (JABLOTRON_SHORT_TIME + JABLOTRON_JITTER_TIME)
#define JABLOTRON_LONG_TIME_LOW   (JABLOTRON_LONG_TIME - JABLOTRON_JITTER_TIME)
#define JABLOTRON_LONG_TIME_HIGH  (JABLOTRON_LONG_TIME + JABLOTRON_JITTER_TIME)

typedef struct {
    bool last_short;
    bool last_level;
    size_t encoded_index;
    uint8_t encoded_data[JABLOTRON_ENCODED_BYTE_FULL_SIZE];
    uint8_t data[JABLOTRON_DECODED_DATA_SIZE];
} ProtocolJablotron;

ProtocolJablotron* protocol_jablotron_alloc(void) {
    ProtocolJablotron* protocol = malloc(sizeof(ProtocolJablotron));
    return protocol;
}

void protocol_jablotron_free(ProtocolJablotron* protocol) {
    free(protocol);
}

uint8_t* protocol_jablotron_get_data(ProtocolJablotron* proto) {
    return proto->data;
}

void protocol_jablotron_decoder_start(ProtocolJablotron* protocol) {
    memset(protocol->encoded_data, 0, JABLOTRON_ENCODED_BYTE_FULL_SIZE);
    protocol->last_short = false;
}

uint8_t protocol_jablotron_checksum(uint8_t* bits) {
    uint8_t chksum = 0;
    for(uint8_t i = 16; i < 56; i += 8) {
        chksum += bit_lib_get_bits(bits, i, 8);
    }
    chksum ^= 0x3A;
    return chksum;
}

uint64_t protocol_jablotron_card_id(uint8_t* bytes) {
    uint64_t id = 0;
    for(int i = 0; i < 5; i++) {
        id *= 100;
        id += ((bytes[i] & 0xF0) >> 4) * 10 + (bytes[i] & 0x0F);
    }
    return id;
}

static bool protocol_jablotron_can_be_decoded(ProtocolJablotron* protocol) {
    // check 11 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 0, 16) != 0b1111111111111111) return false;
    // check next 11 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 64, 16) != 0b1111111111111111) return false;

    uint8_t checksum = bit_lib_get_bits(protocol->encoded_data, 56, 8);
    if(checksum != protocol_jablotron_checksum(protocol->encoded_data)) return false;

    return true;
}

void protocol_jablotron_decode(ProtocolJablotron* protocol) {
    bit_lib_copy_bits(protocol->data, 0, 40, protocol->encoded_data, 16);
}

bool protocol_jablotron_decoder_feed(ProtocolJablotron* protocol, bool level, uint32_t duration) {
    UNUSED(level);
    bool pushed = false;

    // Bi-Phase Manchester decoding
    if(duration >= JABLOTRON_SHORT_TIME_LOW && duration <= JABLOTRON_SHORT_TIME_HIGH) {
        if(protocol->last_short == false) {
            protocol->last_short = true;
        } else {
            pushed = true;
            bit_lib_push_bit(protocol->encoded_data, JABLOTRON_ENCODED_BYTE_FULL_SIZE, false);
            protocol->last_short = false;
        }
    } else if(duration >= JABLOTRON_LONG_TIME_LOW && duration <= JABLOTRON_LONG_TIME_HIGH) {
        if(protocol->last_short == false) {
            pushed = true;
            bit_lib_push_bit(protocol->encoded_data, JABLOTRON_ENCODED_BYTE_FULL_SIZE, true);
        } else {
            // reset
            protocol->last_short = false;
        }
    } else {
        // reset
        protocol->last_short = false;
    }

    if(pushed && protocol_jablotron_can_be_decoded(protocol)) {
        protocol_jablotron_decode(protocol);
        return true;
    }

    return false;
}

bool protocol_jablotron_encoder_start(ProtocolJablotron* protocol) {
    // preamble
    bit_lib_set_bits(protocol->encoded_data, 0, 0b11111111, 8);
    bit_lib_set_bits(protocol->encoded_data, 8, 0b11111111, 8);

    // Full code
    bit_lib_copy_bits(protocol->encoded_data, 16, 40, protocol->data, 0);

    // Checksum
    bit_lib_set_bits(
        protocol->encoded_data, 56, protocol_jablotron_checksum(protocol->encoded_data), 8);

    protocol->encoded_index = 0;
    protocol->last_short = false;
    protocol->last_level = false;
    return true;
}

LevelDuration protocol_jablotron_encoder_yield(ProtocolJablotron* protocol) {
    uint32_t duration;
    protocol->last_level = !protocol->last_level;

    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_index);

    // Bi-Phase Manchester encoder
    if(bit) {
        // one long pulse for 1
        duration = JABLOTRON_LONG_TIME / 8;
        bit_lib_increment_index(protocol->encoded_index, JABLOTRON_ENCODED_BIT_SIZE);
    } else {
        // two short pulses for 0
        duration = JABLOTRON_SHORT_TIME / 8;
        if(protocol->last_short) {
            bit_lib_increment_index(protocol->encoded_index, JABLOTRON_ENCODED_BIT_SIZE);
            protocol->last_short = false;
        } else {
            protocol->last_short = true;
        }
    }

    return level_duration_make(protocol->last_level, duration);
}

void protocol_jablotron_render_data(ProtocolJablotron* protocol, FuriString* result) {
    uint64_t id = protocol_jablotron_card_id(protocol->data);
    furi_string_printf(result, "Card: %llX", id);
}

bool protocol_jablotron_write_data(ProtocolJablotron* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_jablotron_encoder_start(protocol);
    protocol_jablotron_decode(protocol);

    protocol_jablotron_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_DIPHASE | LFRFID_T5577_BITRATE_RF_64 |
                                  (2 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.blocks_to_write = 3;
        result = true;
    }
    return result;
}

const ProtocolBase protocol_jablotron = {
    .name = "Jablotron",
    .manufacturer = "Jablotron",
    .data_size = JABLOTRON_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_jablotron_alloc,
    .free = (ProtocolFree)protocol_jablotron_free,
    .get_data = (ProtocolGetData)protocol_jablotron_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_jablotron_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_jablotron_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_jablotron_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_jablotron_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_jablotron_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_jablotron_render_data,
    .write_data = (ProtocolWriteData)protocol_jablotron_write_data,
};
