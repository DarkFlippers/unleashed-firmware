#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define INDALA26_PREAMBLE_BIT_SIZE  (33)
#define INDALA26_PREAMBLE_DATA_SIZE (5)

#define INDALA26_ENCODED_BIT_SIZE (64)
#define INDALA26_ENCODED_DATA_SIZE \
    (((INDALA26_ENCODED_BIT_SIZE) / 8) + INDALA26_PREAMBLE_DATA_SIZE)
#define INDALA26_ENCODED_DATA_LAST ((INDALA26_ENCODED_BIT_SIZE) / 8)

#define INDALA26_DECODED_BIT_SIZE  (28)
#define INDALA26_DECODED_DATA_SIZE (4)

#define INDALA26_US_PER_BIT             (255)
#define INDALA26_ENCODER_PULSES_PER_BIT (16)

typedef struct {
    uint8_t data_index;
    uint8_t bit_clock_index;
    bool last_bit;
    bool current_polarity;
    bool pulse_phase;
} ProtocolIndalaEncoder;

typedef struct {
    uint8_t encoded_data[INDALA26_ENCODED_DATA_SIZE];
    uint8_t negative_encoded_data[INDALA26_ENCODED_DATA_SIZE];
    uint8_t corrupted_encoded_data[INDALA26_ENCODED_DATA_SIZE];
    uint8_t corrupted_negative_encoded_data[INDALA26_ENCODED_DATA_SIZE];

    uint8_t data[INDALA26_DECODED_DATA_SIZE];
    ProtocolIndalaEncoder encoder;
} ProtocolIndala;

ProtocolIndala* protocol_indala26_alloc(void) {
    ProtocolIndala* protocol = malloc(sizeof(ProtocolIndala));
    return protocol;
}

void protocol_indala26_free(ProtocolIndala* protocol) {
    free(protocol);
}

uint8_t* protocol_indala26_get_data(ProtocolIndala* protocol) {
    return protocol->data;
}

void protocol_indala26_decoder_start(ProtocolIndala* protocol) {
    memset(protocol->encoded_data, 0, INDALA26_ENCODED_DATA_SIZE);
    memset(protocol->negative_encoded_data, 0, INDALA26_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_encoded_data, 0, INDALA26_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_negative_encoded_data, 0, INDALA26_ENCODED_DATA_SIZE);
}

static bool protocol_indala26_check_preamble(uint8_t* data, size_t bit_index) {
    // Preamble 10100000 00000000 00000000 00000000 1
    if(*(uint32_t*)&data[bit_index / 8] != 0b00000000000000000000000010100000) return false;
    if(bit_lib_get_bit(data, bit_index + 32) != 1) return false;
    return true;
}

static bool protocol_indala26_can_be_decoded(uint8_t* data) {
    if(!protocol_indala26_check_preamble(data, 0)) return false;
    if(!protocol_indala26_check_preamble(data, 64)) return false;
    if(bit_lib_get_bit(data, 61) != 0) return false;
    if(bit_lib_get_bit(data, 60) != 0) return false;
    return true;
}

static bool protocol_indala26_decoder_feed_internal(bool polarity, uint32_t time, uint8_t* data) {
    time += (INDALA26_US_PER_BIT / 2);

    size_t bit_count = (time / INDALA26_US_PER_BIT);
    bool result = false;

    if(bit_count < INDALA26_ENCODED_BIT_SIZE) {
        for(size_t i = 0; i < bit_count; i++) {
            bit_lib_push_bit(data, INDALA26_ENCODED_DATA_SIZE, polarity);
            if(protocol_indala26_can_be_decoded(data)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

static void protocol_indala26_decoder_save(uint8_t* data_to, const uint8_t* data_from) {
    bit_lib_copy_bits(data_to, 0, 22, data_from, 33);
    bit_lib_copy_bits(data_to, 22, 5, data_from, 55);
    bit_lib_copy_bits(data_to, 27, 2, data_from, 62);
}

bool protocol_indala26_decoder_feed(ProtocolIndala* protocol, bool level, uint32_t duration) {
    bool result = false;

    if(duration > (INDALA26_US_PER_BIT / 2)) {
        if(protocol_indala26_decoder_feed_internal(level, duration, protocol->encoded_data)) {
            protocol_indala26_decoder_save(protocol->data, protocol->encoded_data);
            FURI_LOG_D("Indala26", "Positive");
            result = true;
            return result;
        }

        if(protocol_indala26_decoder_feed_internal(
               !level, duration, protocol->negative_encoded_data)) {
            protocol_indala26_decoder_save(protocol->data, protocol->negative_encoded_data);
            FURI_LOG_D("Indala26", "Negative");
            result = true;
            return result;
        }
    }

    if(duration > (INDALA26_US_PER_BIT / 4)) {
        // Try to decode wrong phase synced data
        if(level) {
            duration += 120;
        } else {
            if(duration > 120) {
                duration -= 120;
            }
        }

        if(protocol_indala26_decoder_feed_internal(
               level, duration, protocol->corrupted_encoded_data)) {
            protocol_indala26_decoder_save(protocol->data, protocol->corrupted_encoded_data);
            FURI_LOG_D("Indala26", "Positive Corrupted");

            result = true;
            return result;
        }

        if(protocol_indala26_decoder_feed_internal(
               !level, duration, protocol->corrupted_negative_encoded_data)) {
            protocol_indala26_decoder_save(
                protocol->data, protocol->corrupted_negative_encoded_data);
            FURI_LOG_D("Indala26", "Negative Corrupted");

            result = true;
            return result;
        }
    }

    return result;
}

bool protocol_indala26_encoder_start(ProtocolIndala* protocol) {
    memset(protocol->encoded_data, 0, INDALA26_ENCODED_DATA_SIZE);
    *(uint32_t*)&protocol->encoded_data[0] = 0b00000000000000000000000010100000;
    bit_lib_set_bit(protocol->encoded_data, 32, 1);
    bit_lib_copy_bits(protocol->encoded_data, 33, 22, protocol->data, 0);
    bit_lib_copy_bits(protocol->encoded_data, 55, 5, protocol->data, 22);
    bit_lib_copy_bits(protocol->encoded_data, 62, 2, protocol->data, 27);

    protocol->encoder.last_bit =
        bit_lib_get_bit(protocol->encoded_data, INDALA26_ENCODED_BIT_SIZE - 1);
    protocol->encoder.data_index = 0;
    protocol->encoder.current_polarity = true;
    protocol->encoder.pulse_phase = true;
    protocol->encoder.bit_clock_index = 0;

    return true;
}

LevelDuration protocol_indala26_encoder_yield(ProtocolIndala* protocol) {
    LevelDuration level_duration;
    ProtocolIndalaEncoder* encoder = &protocol->encoder;

    if(encoder->pulse_phase) {
        level_duration = level_duration_make(encoder->current_polarity, 1);
        encoder->pulse_phase = false;
    } else {
        level_duration = level_duration_make(!encoder->current_polarity, 1);
        encoder->pulse_phase = true;

        encoder->bit_clock_index++;
        if(encoder->bit_clock_index >= INDALA26_ENCODER_PULSES_PER_BIT) {
            encoder->bit_clock_index = 0;

            bool current_bit = bit_lib_get_bit(protocol->encoded_data, encoder->data_index);

            if(current_bit != encoder->last_bit) {
                encoder->current_polarity = !encoder->current_polarity;
            }

            encoder->last_bit = current_bit;

            bit_lib_increment_index(encoder->data_index, INDALA26_ENCODED_BIT_SIZE);
        }
    }

    return level_duration;
}

// factory code
static uint8_t get_fc(const uint8_t* data) {
    uint8_t fc = 0;

    fc = fc << 1 | bit_lib_get_bit(data, 24);
    fc = fc << 1 | bit_lib_get_bit(data, 16);
    fc = fc << 1 | bit_lib_get_bit(data, 11);
    fc = fc << 1 | bit_lib_get_bit(data, 14);
    fc = fc << 1 | bit_lib_get_bit(data, 15);
    fc = fc << 1 | bit_lib_get_bit(data, 20);
    fc = fc << 1 | bit_lib_get_bit(data, 6);
    fc = fc << 1 | bit_lib_get_bit(data, 25);

    return fc;
}

// card number
static uint16_t get_cn(const uint8_t* data) {
    uint16_t cn = 0;

    cn = cn << 1 | bit_lib_get_bit(data, 9);
    cn = cn << 1 | bit_lib_get_bit(data, 12);
    cn = cn << 1 | bit_lib_get_bit(data, 10);
    cn = cn << 1 | bit_lib_get_bit(data, 7);
    cn = cn << 1 | bit_lib_get_bit(data, 19);
    cn = cn << 1 | bit_lib_get_bit(data, 3);
    cn = cn << 1 | bit_lib_get_bit(data, 2);
    cn = cn << 1 | bit_lib_get_bit(data, 18);
    cn = cn << 1 | bit_lib_get_bit(data, 13);
    cn = cn << 1 | bit_lib_get_bit(data, 0);
    cn = cn << 1 | bit_lib_get_bit(data, 4);
    cn = cn << 1 | bit_lib_get_bit(data, 21);
    cn = cn << 1 | bit_lib_get_bit(data, 23);
    cn = cn << 1 | bit_lib_get_bit(data, 26);
    cn = cn << 1 | bit_lib_get_bit(data, 17);
    cn = cn << 1 | bit_lib_get_bit(data, 8);

    return cn;
}

void protocol_indala26_render_data_internal(
    ProtocolIndala* protocol,
    FuriString* result,
    bool brief) {
    bool wiegand_correct = true;
    bool checksum_correct = true;

    const uint8_t fc = get_fc(protocol->data);
    const uint16_t card = get_cn(protocol->data);
    const uint32_t fc_and_card = fc << 16 | card;
    const uint8_t checksum = bit_lib_get_bit(protocol->data, 27) << 1 |
                             bit_lib_get_bit(protocol->data, 28);
    const bool even_parity = bit_lib_get_bit(protocol->data, 1);
    const bool odd_parity = bit_lib_get_bit(protocol->data, 5);

    // indala checksum
    uint8_t checksum_sum = 0;
    checksum_sum += ((fc_and_card >> 14) & 1);
    checksum_sum += ((fc_and_card >> 12) & 1);
    checksum_sum += ((fc_and_card >> 9) & 1);
    checksum_sum += ((fc_and_card >> 8) & 1);
    checksum_sum += ((fc_and_card >> 6) & 1);
    checksum_sum += ((fc_and_card >> 5) & 1);
    checksum_sum += ((fc_and_card >> 2) & 1);
    checksum_sum += ((fc_and_card >> 0) & 1);
    checksum_sum = checksum_sum & 0b1;

    if(checksum_sum == 1 && checksum == 0b01) {
    } else if(checksum_sum == 0 && checksum == 0b10) {
    } else {
        checksum_correct = false;
    }

    // wiegand parity
    uint8_t even_parity_sum = 0;
    for(int8_t i = 12; i < 24; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            even_parity_sum++;
        }
    }
    if(even_parity_sum % 2 != even_parity) wiegand_correct = false;

    uint8_t odd_parity_sum = 1;
    for(int8_t i = 0; i < 12; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            odd_parity_sum++;
        }
    }
    if(odd_parity_sum % 2 != odd_parity) wiegand_correct = false;

    if(brief) {
        furi_string_printf(
            result,
            "FC: %u\n"
            "Card: %u",
            fc,
            card);
    } else {
        furi_string_printf(
            result,
            "FC: %u\n"
            "Card: %u\n"
            "Parity: %c\n"
            "Checksum: %c",
            fc,
            card,
            (wiegand_correct ? '+' : '-'),
            (checksum_correct ? '+' : '-'));
    }
}
void protocol_indala26_render_data(ProtocolIndala* protocol, FuriString* result) {
    protocol_indala26_render_data_internal(protocol, result, false);
}
void protocol_indala26_render_brief_data(ProtocolIndala* protocol, FuriString* result) {
    protocol_indala26_render_data_internal(protocol, result, true);
}

bool protocol_indala26_write_data(ProtocolIndala* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_indala26_encoder_start(protocol);

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

const ProtocolBase protocol_indala26 = {
    .name = "Indala26",
    .manufacturer = "Motorola",
    .data_size = INDALA26_DECODED_DATA_SIZE,
    .features = LFRFIDFeaturePSK,
    .validate_count = 6,
    .alloc = (ProtocolAlloc)protocol_indala26_alloc,
    .free = (ProtocolFree)protocol_indala26_free,
    .get_data = (ProtocolGetData)protocol_indala26_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_indala26_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_indala26_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_indala26_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_indala26_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_indala26_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_indala26_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_indala26_write_data,
};
