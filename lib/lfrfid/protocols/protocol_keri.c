#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define KERI_PREAMBLE_BIT_SIZE  (33)
#define KERI_PREAMBLE_DATA_SIZE (5)

#define KERI_ENCODED_BIT_SIZE  (64)
#define KERI_ENCODED_DATA_SIZE (((KERI_ENCODED_BIT_SIZE) / 8) + KERI_PREAMBLE_DATA_SIZE)
#define KERI_ENCODED_DATA_LAST ((KERI_ENCODED_BIT_SIZE) / 8)

#define KERI_DECODED_BIT_SIZE  (28)
#define KERI_DECODED_DATA_SIZE (4)

#define KERI_US_PER_BIT             (255)
#define KERI_ENCODER_PULSES_PER_BIT (16)

typedef struct {
    uint8_t data_index;
    uint8_t bit_clock_index;
    bool last_bit;
    bool current_polarity;
    bool pulse_phase;
} ProtocolKeriEncoder;

typedef struct {
    uint8_t encoded_data[KERI_ENCODED_DATA_SIZE];
    uint8_t negative_encoded_data[KERI_ENCODED_DATA_SIZE];
    uint8_t corrupted_encoded_data[KERI_ENCODED_DATA_SIZE];
    uint8_t corrupted_negative_encoded_data[KERI_ENCODED_DATA_SIZE];

    uint8_t data[KERI_DECODED_DATA_SIZE];
    ProtocolKeriEncoder encoder;
} ProtocolKeri;

ProtocolKeri* protocol_keri_alloc(void) {
    ProtocolKeri* protocol = malloc(sizeof(ProtocolKeri));
    return protocol;
}

void protocol_keri_free(ProtocolKeri* protocol) {
    free(protocol);
}

uint8_t* protocol_keri_get_data(ProtocolKeri* protocol) {
    return protocol->data;
}

void protocol_keri_decoder_start(ProtocolKeri* protocol) {
    memset(protocol->encoded_data, 0, KERI_ENCODED_DATA_SIZE);
    memset(protocol->negative_encoded_data, 0, KERI_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_encoded_data, 0, KERI_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_negative_encoded_data, 0, KERI_ENCODED_DATA_SIZE);
}

static bool protocol_keri_check_preamble(uint8_t* data, size_t bit_index) {
    // Preamble 11100000 00000000 00000000 00000000 1
    if(*(uint32_t*)&data[bit_index / 8] != 0b00000000000000000000000011100000) return false;
    if(bit_lib_get_bit(data, bit_index + 32) != 1) return false;
    return true;
}

static bool protocol_keri_can_be_decoded(uint8_t* data) {
    if(!protocol_keri_check_preamble(data, 0)) return false;
    if(!protocol_keri_check_preamble(data, 64)) return false;
    ///if(bit_lib_get_bit(data, 61) != 0) return false;
    //if(bit_lib_get_bit(data, 60) != 0) return false;
    return true;
}

static bool protocol_keri_decoder_feed_internal(bool polarity, uint32_t time, uint8_t* data) {
    time += (KERI_US_PER_BIT / 2);

    size_t bit_count = (time / KERI_US_PER_BIT);
    bool result = false;

    if(bit_count < KERI_ENCODED_BIT_SIZE) {
        for(size_t i = 0; i < bit_count; i++) {
            bit_lib_push_bit(data, KERI_ENCODED_DATA_SIZE, polarity);
            if(protocol_keri_can_be_decoded(data)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

static void protocol_keri_descramble(uint32_t* fc, uint32_t* cn, uint32_t* internal_id) {
    const uint8_t card_to_id[] = {255, 255, 255, 255, 13, 12, 20, 5,   16,  6,  21,
                                  17,  8,   255, 0,   7,  10, 15, 255, 11,  4,  1,
                                  255, 18,  255, 19,  2,  14, 3,  9,   255, 255};

    const uint8_t card_to_fc[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 0,   255, 255, 255, 255, 2,   255, 255, 255,
                                  3,   255, 4,   255, 255, 255, 255, 255, 1,   255};

    *fc = 0;
    *cn = 0;
    for(uint8_t card_idx = 0; card_idx < 32; card_idx++) {
        bool bit = (*internal_id >> card_idx) & 1;
        // Card ID
        if(card_to_id[card_idx] < 32) {
            *cn = *cn | (bit << card_to_id[card_idx]);
        }
        // Card FC
        if(card_to_fc[card_idx] < 32) {
            *fc = *fc | (bit << card_to_fc[card_idx]);
        }
    }
}

static void protocol_keri_decoder_save(uint8_t* data_to, const uint8_t* data_from) {
    uint32_t id = bit_lib_get_bits_32(data_from, 32, 32);
    data_to[3] = (uint8_t)id;
    data_to[2] = (uint8_t)(id >>= 8);
    data_to[1] = (uint8_t)(id >>= 8);
    data_to[0] = (uint8_t)(id >>= 8);
}

bool protocol_keri_decoder_feed(ProtocolKeri* protocol, bool level, uint32_t duration) {
    bool result = false;

    if(duration > (KERI_US_PER_BIT / 2)) {
        if(protocol_keri_decoder_feed_internal(level, duration, protocol->encoded_data)) {
            protocol_keri_decoder_save(protocol->data, protocol->encoded_data);
            result = true;
            return result;
        }

        if(protocol_keri_decoder_feed_internal(!level, duration, protocol->negative_encoded_data)) {
            protocol_keri_decoder_save(protocol->data, protocol->negative_encoded_data);
            result = true;
            return result;
        }
    }

    if(duration > (KERI_US_PER_BIT / 4)) {
        // Try to decode wrong phase synced data
        if(level) {
            duration += 120;
        } else {
            if(duration > 120) {
                duration -= 120;
            }
        }

        if(protocol_keri_decoder_feed_internal(level, duration, protocol->corrupted_encoded_data)) {
            protocol_keri_decoder_save(protocol->data, protocol->corrupted_encoded_data);

            result = true;
            return result;
        }

        if(protocol_keri_decoder_feed_internal(
               !level, duration, protocol->corrupted_negative_encoded_data)) {
            protocol_keri_decoder_save(protocol->data, protocol->corrupted_negative_encoded_data);

            result = true;
            return result;
        }
    }

    return result;
}

bool protocol_keri_encoder_start(ProtocolKeri* protocol) {
    memset(protocol->encoded_data, 0, KERI_ENCODED_DATA_SIZE);
    *(uint32_t*)&protocol->encoded_data[0] = 0b00000000000000000000000011100000;
    bit_lib_copy_bits(protocol->encoded_data, 32, 32, protocol->data, 0);
    bit_lib_set_bits(protocol->encoded_data, 32, 1, 1);

    protocol->encoder.last_bit =
        bit_lib_get_bit(protocol->encoded_data, KERI_ENCODED_BIT_SIZE - 1);
    protocol->encoder.data_index = 0;
    protocol->encoder.current_polarity = true;
    protocol->encoder.pulse_phase = true;
    protocol->encoder.bit_clock_index = 0;

    return true;
}

LevelDuration protocol_keri_encoder_yield(ProtocolKeri* protocol) {
    LevelDuration level_duration;
    ProtocolKeriEncoder* encoder = &protocol->encoder;

    if(encoder->pulse_phase) {
        level_duration = level_duration_make(encoder->current_polarity, 1);
        encoder->pulse_phase = false;
    } else {
        level_duration = level_duration_make(!encoder->current_polarity, 1);
        encoder->pulse_phase = true;

        encoder->bit_clock_index++;
        if(encoder->bit_clock_index >= KERI_ENCODER_PULSES_PER_BIT) {
            encoder->bit_clock_index = 0;

            bool current_bit = bit_lib_get_bit(protocol->encoded_data, encoder->data_index);

            if(current_bit != encoder->last_bit) {
                encoder->current_polarity = !encoder->current_polarity;
            }

            encoder->last_bit = current_bit;

            bit_lib_increment_index(encoder->data_index, KERI_ENCODED_BIT_SIZE);
        }
    }

    return level_duration;
}

static void
    protocol_keri_render_data_internal(ProtocolKeri* protocol, FuriString* result, bool brief) {
    uint32_t data = bit_lib_get_bits_32(protocol->data, 0, 32);
    uint32_t internal_id = data & 0x7FFFFFFF;
    uint32_t fc = 0;
    uint32_t cn = 0;
    protocol_keri_descramble(&fc, &cn, &data);

    if(brief) {
        furi_string_printf(
            result,
            "Internal ID: %lu\n"
            "FC: %lu; Card: %lu",
            internal_id,
            fc,
            cn);
    } else {
        furi_string_printf(
            result,
            "Internal ID: %lu\n"
            "FC: %lu\n"
            "Card: %lu",
            internal_id,
            fc,
            cn);
    }
}

void protocol_keri_render_data(ProtocolKeri* protocol, FuriString* result) {
    protocol_keri_render_data_internal(protocol, result, false);
}

void protocol_keri_render_brief_data(ProtocolKeri* protocol, FuriString* result) {
    protocol_keri_render_data_internal(protocol, result, true);
}

bool protocol_keri_write_data(ProtocolKeri* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Start bit should be always set
    protocol->data[0] |= (1 << 7);
    protocol_keri_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_TESTMODE_DISABLED | LFRFID_T5577_X_MODE |
                                  LFRFID_T5577_MODULATION_PSK1 | LFRFID_T5577_PSKCF_RF_2 |
                                  (2 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[0] |= 0xF << 18;
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.blocks_to_write = 3;
        result = true;
    }
    return result;
}

const ProtocolBase protocol_keri = {
    .name = "Keri",
    .manufacturer = "Keri",
    .data_size = KERI_DECODED_DATA_SIZE,
    .features = LFRFIDFeaturePSK,
    .validate_count = 6,
    .alloc = (ProtocolAlloc)protocol_keri_alloc,
    .free = (ProtocolFree)protocol_keri_free,
    .get_data = (ProtocolGetData)protocol_keri_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_keri_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_keri_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_keri_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_keri_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_keri_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_keri_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_keri_write_data,
};
