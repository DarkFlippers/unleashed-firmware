#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define VIKING_CLOCK_PER_BIT (32)

#define VIKING_ENCODED_BIT_SIZE       (64)
#define VIKING_ENCODED_BYTE_SIZE      (((VIKING_ENCODED_BIT_SIZE) / 8))
#define VIKING_PREAMBLE_BIT_SIZE      (24)
#define VIKING_PREAMBLE_BYTE_SIZE     (3)
#define VIKING_ENCODED_BYTE_FULL_SIZE (VIKING_ENCODED_BYTE_SIZE + VIKING_PREAMBLE_BYTE_SIZE)
#define VIKING_DECODED_DATA_SIZE      4

#define VIKING_READ_SHORT_TIME  (128)
#define VIKING_READ_LONG_TIME   (256)
#define VIKING_READ_JITTER_TIME (60)

#define VIKING_READ_SHORT_TIME_LOW  (VIKING_READ_SHORT_TIME - VIKING_READ_JITTER_TIME)
#define VIKING_READ_SHORT_TIME_HIGH (VIKING_READ_SHORT_TIME + VIKING_READ_JITTER_TIME)
#define VIKING_READ_LONG_TIME_LOW   (VIKING_READ_LONG_TIME - VIKING_READ_JITTER_TIME)
#define VIKING_READ_LONG_TIME_HIGH  (VIKING_READ_LONG_TIME + VIKING_READ_JITTER_TIME)

typedef struct {
    uint8_t data[VIKING_DECODED_DATA_SIZE];
    uint8_t encoded_data[VIKING_ENCODED_BYTE_FULL_SIZE];

    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolViking;

ProtocolViking* protocol_viking_alloc(void) {
    ProtocolViking* proto = malloc(sizeof(ProtocolViking));
    return (void*)proto;
}

void protocol_viking_free(ProtocolViking* protocol) {
    free(protocol);
}

uint8_t* protocol_viking_get_data(ProtocolViking* protocol) {
    return protocol->data;
}

static void protocol_viking_decode(ProtocolViking* protocol) {
    // Copy Card ID
    bit_lib_copy_bits(protocol->data, 0, 32, protocol->encoded_data, 24);
}

static bool protocol_viking_can_be_decoded(ProtocolViking* protocol) {
    // check 24 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 0, 16) != 0b1111001000000000) return false;
    if(bit_lib_get_bits(protocol->encoded_data, 16, 8) != 0b00000000) return false;

    // check next 24 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 64, 16) != 0b1111001000000000) return false;
    if(bit_lib_get_bits(protocol->encoded_data, 80, 8) != 0b00000000) return false;

    // Checksum
    uint32_t checksum = bit_lib_get_bits(protocol->encoded_data, 0, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 8, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 16, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 24, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 32, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 40, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 48, 8) ^
                        bit_lib_get_bits(protocol->encoded_data, 56, 8) ^ 0xA8;
    if(checksum != 0) return false;

    return true;
}

void protocol_viking_decoder_start(ProtocolViking* protocol) {
    memset(protocol->encoded_data, 0, VIKING_ENCODED_BYTE_FULL_SIZE);
    manchester_advance(
        protocol->decoder_manchester_state,
        ManchesterEventReset,
        &protocol->decoder_manchester_state,
        NULL);
}

bool protocol_viking_decoder_feed(ProtocolViking* protocol, bool level, uint32_t duration) {
    bool result = false;

    ManchesterEvent event = ManchesterEventReset;

    if(duration > VIKING_READ_SHORT_TIME_LOW && duration < VIKING_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > VIKING_READ_LONG_TIME_LOW && duration < VIKING_READ_LONG_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventLongHigh;
        } else {
            event = ManchesterEventLongLow;
        }
    }

    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok = manchester_advance(
            protocol->decoder_manchester_state, event, &protocol->decoder_manchester_state, &data);

        if(data_ok) {
            bit_lib_push_bit(protocol->encoded_data, VIKING_ENCODED_BYTE_FULL_SIZE, data);

            if(protocol_viking_can_be_decoded(protocol)) {
                protocol_viking_decode(protocol);
                result = true;
            }
        }
    }

    return result;
}

bool protocol_viking_encoder_start(ProtocolViking* protocol) {
    // Preamble
    bit_lib_set_bits(protocol->encoded_data, 0, 0b11110010, 8);
    bit_lib_set_bits(protocol->encoded_data, 8, 0b00000000, 8);
    bit_lib_set_bits(protocol->encoded_data, 16, 0b00000000, 8);

    // Card Id
    bit_lib_copy_bits(protocol->encoded_data, 24, 32, protocol->data, 0);

    // Checksum
    uint32_t id = bit_lib_get_bits_32(protocol->data, 0, 32);
    uint8_t checksum = ((id >> 24) & 0xFF) ^ ((id >> 16) & 0xFF) ^ ((id >> 8) & 0xFF) ^
                       (id & 0xFF) ^ 0xF2 ^ 0xA8;
    bit_lib_set_bits(protocol->encoded_data, 56, checksum, 8);

    return true;
}

LevelDuration protocol_viking_encoder_yield(ProtocolViking* protocol) {
    bool level = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_data_index);
    uint32_t duration = VIKING_CLOCK_PER_BIT / 2;

    if(protocol->encoded_polarity) {
        protocol->encoded_polarity = false;
    } else {
        level = !level;

        protocol->encoded_polarity = true;
        bit_lib_increment_index(protocol->encoded_data_index, VIKING_ENCODED_BIT_SIZE);
    }

    return level_duration_make(level, duration);
}

bool protocol_viking_write_data(ProtocolViking* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_viking_encoder_start(protocol);
    protocol_viking_decode(protocol);

    protocol_viking_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_32 |
             (2 << LFRFID_T5577_MAXBLOCK_SHIFT));
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.blocks_to_write = 3;
        result = true;
    } else if(request->write_type == LFRFIDWriteTypeEM4305) {
        request->em4305.word[4] =
            (EM4x05_MODULATION_MANCHESTER | EM4x05_SET_BITRATE(32) | (6 << EM4x05_MAXBLOCK_SHIFT));
        uint32_t encoded_data_reversed[2] = {0};
        for(uint8_t i = 0; i < 64; i++) {
            encoded_data_reversed[i / 32] =
                (encoded_data_reversed[i / 32] << 1) |
                (bit_lib_get_bit(protocol->encoded_data, (63 - i)) & 1);
        }
        request->em4305.word[5] = encoded_data_reversed[1];
        request->em4305.word[6] = encoded_data_reversed[0];
        request->em4305.mask = 0x70;
        result = true;
    }
    return result;
}

void protocol_viking_render_data(ProtocolViking* protocol, FuriString* result) {
    furi_string_printf(result, "ID: %08lX", bit_lib_get_bits_32(protocol->data, 0, 32));
}

const ProtocolBase protocol_viking = {
    .name = "Viking",
    .manufacturer = "Viking",
    .data_size = VIKING_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_viking_alloc,
    .free = (ProtocolFree)protocol_viking_free,
    .get_data = (ProtocolGetData)protocol_viking_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_viking_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_viking_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_viking_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_viking_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_viking_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_viking_render_data,
    .write_data = (ProtocolWriteData)protocol_viking_write_data,
};
