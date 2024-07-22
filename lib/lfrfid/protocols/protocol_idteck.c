#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

// Example: 4944544B 351FBE4B
// 01001001 01000100 01010100 01001011       00110101 00011111 10111110 01001011
// 4    9    4    4    5    4    4    B      3    5    1    F    B    E    4    B
// 0100 1001 0100 0100 0101 0100 0100 1011   0011 0101 0001 1111 1011 1110 0100 1011

#define IDTECK_PREAMBLE_BIT_SIZE  (32)
#define IDTECK_PREAMBLE_DATA_SIZE (8)

#define IDTECK_ENCODED_BIT_SIZE  (64)
#define IDTECK_ENCODED_DATA_SIZE (((IDTECK_ENCODED_BIT_SIZE) / 8) + IDTECK_PREAMBLE_DATA_SIZE)
#define IDTECK_ENCODED_DATA_LAST ((IDTECK_ENCODED_BIT_SIZE) / 8)

#define IDTECK_DECODED_BIT_SIZE  (64)
#define IDTECK_DECODED_DATA_SIZE (8)

#define IDTECK_US_PER_BIT             (255)
#define IDTECK_ENCODER_PULSES_PER_BIT (16)

typedef struct {
    uint8_t data_index;
    uint8_t bit_clock_index;
    bool last_bit;
    bool current_polarity;
    bool pulse_phase;
} ProtocolIdteckEncoder;

typedef struct {
    uint8_t encoded_data[IDTECK_ENCODED_DATA_SIZE];
    uint8_t negative_encoded_data[IDTECK_ENCODED_DATA_SIZE];
    uint8_t corrupted_encoded_data[IDTECK_ENCODED_DATA_SIZE];
    uint8_t corrupted_negative_encoded_data[IDTECK_ENCODED_DATA_SIZE];

    uint8_t data[IDTECK_DECODED_DATA_SIZE];
    ProtocolIdteckEncoder encoder;
} ProtocolIdteck;

ProtocolIdteck* protocol_idteck_alloc(void) {
    ProtocolIdteck* protocol = malloc(sizeof(ProtocolIdteck));
    return protocol;
}

void protocol_idteck_free(ProtocolIdteck* protocol) {
    free(protocol);
}

uint8_t* protocol_idteck_get_data(ProtocolIdteck* protocol) {
    return protocol->data;
}

void protocol_idteck_decoder_start(ProtocolIdteck* protocol) {
    memset(protocol->encoded_data, 0, IDTECK_ENCODED_DATA_SIZE);
    memset(protocol->negative_encoded_data, 0, IDTECK_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_encoded_data, 0, IDTECK_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_negative_encoded_data, 0, IDTECK_ENCODED_DATA_SIZE);
}

static bool protocol_idteck_check_preamble(uint8_t* data, size_t bit_index) {
    // Preamble 01001001 01000100 01010100 01001011
    if(*(uint32_t*)&data[bit_index / 8] != 0b01001011010101000100010001001001) return false;
    return true;
}

static bool protocol_idteck_can_be_decoded(uint8_t* data) {
    if(!protocol_idteck_check_preamble(data, 0)) return false;
    return true;
}

static bool protocol_idteck_decoder_feed_internal(bool polarity, uint32_t time, uint8_t* data) {
    time += (IDTECK_US_PER_BIT / 2);

    size_t bit_count = (time / IDTECK_US_PER_BIT);
    bool result = false;

    if(bit_count < IDTECK_ENCODED_BIT_SIZE) {
        for(size_t i = 0; i < bit_count; i++) {
            bit_lib_push_bit(data, IDTECK_ENCODED_DATA_SIZE, polarity);
            if(protocol_idteck_can_be_decoded(data)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

static void protocol_idteck_decoder_save(uint8_t* data_to, const uint8_t* data_from) {
    bit_lib_copy_bits(data_to, 0, 64, data_from, 0);
}

bool protocol_idteck_decoder_feed(ProtocolIdteck* protocol, bool level, uint32_t duration) {
    bool result = false;

    if(duration > (IDTECK_US_PER_BIT / 2)) {
        if(protocol_idteck_decoder_feed_internal(level, duration, protocol->encoded_data)) {
            protocol_idteck_decoder_save(protocol->data, protocol->encoded_data);
            FURI_LOG_D("Idteck", "Positive");
            result = true;
            return result;
        }

        if(protocol_idteck_decoder_feed_internal(
               !level, duration, protocol->negative_encoded_data)) {
            protocol_idteck_decoder_save(protocol->data, protocol->negative_encoded_data);
            FURI_LOG_D("Idteck", "Negative");
            result = true;
            return result;
        }
    }

    if(duration > (IDTECK_US_PER_BIT / 4)) {
        // Try to decode wrong phase synced data
        if(level) {
            duration += 120;
        } else {
            if(duration > 120) {
                duration -= 120;
            }
        }

        if(protocol_idteck_decoder_feed_internal(
               level, duration, protocol->corrupted_encoded_data)) {
            protocol_idteck_decoder_save(protocol->data, protocol->corrupted_encoded_data);
            FURI_LOG_D("Idteck", "Positive Corrupted");

            result = true;
            return result;
        }

        if(protocol_idteck_decoder_feed_internal(
               !level, duration, protocol->corrupted_negative_encoded_data)) {
            protocol_idteck_decoder_save(
                protocol->data, protocol->corrupted_negative_encoded_data);
            FURI_LOG_D("Idteck", "Negative Corrupted");

            result = true;
            return result;
        }
    }

    return result;
}

bool protocol_idteck_encoder_start(ProtocolIdteck* protocol) {
    memset(protocol->encoded_data, 0, IDTECK_ENCODED_DATA_SIZE);
    *(uint32_t*)&protocol->encoded_data[0] = 0b01001011010101000100010001001001;
    bit_lib_copy_bits(protocol->encoded_data, 32, 32, protocol->data, 32);

    protocol->encoder.last_bit =
        bit_lib_get_bit(protocol->encoded_data, IDTECK_ENCODED_BIT_SIZE - 1);
    protocol->encoder.data_index = 0;
    protocol->encoder.current_polarity = true;
    protocol->encoder.pulse_phase = true;
    protocol->encoder.bit_clock_index = 0;

    return true;
}

LevelDuration protocol_idteck_encoder_yield(ProtocolIdteck* protocol) {
    LevelDuration level_duration;
    ProtocolIdteckEncoder* encoder = &protocol->encoder;

    if(encoder->pulse_phase) {
        level_duration = level_duration_make(encoder->current_polarity, 1);
        encoder->pulse_phase = false;
    } else {
        level_duration = level_duration_make(!encoder->current_polarity, 1);
        encoder->pulse_phase = true;

        encoder->bit_clock_index++;
        if(encoder->bit_clock_index >= IDTECK_ENCODER_PULSES_PER_BIT) {
            encoder->bit_clock_index = 0;

            bool current_bit = bit_lib_get_bit(protocol->encoded_data, encoder->data_index);

            if(current_bit != encoder->last_bit) {
                encoder->current_polarity = !encoder->current_polarity;
            }

            encoder->last_bit = current_bit;

            bit_lib_increment_index(encoder->data_index, IDTECK_ENCODED_BIT_SIZE);
        }
    }

    return level_duration;
}

// factory code
static uint32_t get_fc(const uint8_t* data) {
    uint32_t fc = 0;
    fc = bit_lib_get_bits_32(data, 0, 32);
    return fc;
}

// card number
static uint32_t get_card(const uint8_t* data) {
    uint32_t cn = 0;
    cn = bit_lib_get_bits_32(data, 32, 32);
    return cn;
}

void protocol_idteck_render_data(ProtocolIdteck* protocol, FuriString* result) {
    const uint32_t fc = get_fc(protocol->data);
    const uint32_t card = get_card(protocol->data);

    furi_string_printf(
        result,
        "FC: %08lX\n"
        "Card: %08lX",
        fc,
        card);
}

bool protocol_idteck_write_data(ProtocolIdteck* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_idteck_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_BITRATE_RF_32 | LFRFID_T5577_MODULATION_PSK1 |
                                  (2 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.blocks_to_write = 3;
        result = true;
    }
    return result;
}

const ProtocolBase protocol_idteck = {
    .name = "Idteck",
    .manufacturer = "IDTECK",
    .data_size = IDTECK_DECODED_DATA_SIZE,
    .features = LFRFIDFeaturePSK,
    .validate_count = 6,
    .alloc = (ProtocolAlloc)protocol_idteck_alloc,
    .free = (ProtocolFree)protocol_idteck_free,
    .get_data = (ProtocolGetData)protocol_idteck_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_idteck_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_idteck_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_idteck_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_idteck_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_idteck_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_idteck_render_data,
    .write_data = (ProtocolWriteData)protocol_idteck_write_data,
};
