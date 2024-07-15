#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define NEXWATCH_PREAMBLE_BIT_SIZE  (8)
#define NEXWATCH_PREAMBLE_DATA_SIZE (1)

#define NEXWATCH_ENCODED_BIT_SIZE  (96)
#define NEXWATCH_ENCODED_DATA_SIZE ((NEXWATCH_ENCODED_BIT_SIZE) / 8)

#define NEXWATCH_DECODED_BIT_SIZE  (NEXWATCH_DECODED_DATA_SIZE * 8)
#define NEXWATCH_DECODED_DATA_SIZE (8)

#define NEXWATCH_US_PER_BIT             (255)
#define NEXWATCH_ENCODER_PULSES_PER_BIT (16)

typedef struct {
    uint8_t magic;
    char desc[13];
    uint8_t chk;
} ProtocolNexwatchMagic;

ProtocolNexwatchMagic magic_items[] = {
    {0xBE, "Quadrakey", 0},
    {0x88, "Nexkey", 0},
    {0x86, "Honeywell", 0}};

typedef struct {
    uint8_t data_index;
    uint8_t bit_clock_index;
    bool last_bit;
    bool current_polarity;
    bool pulse_phase;
} ProtocolNexwatchEncoder;

typedef struct {
    uint8_t encoded_data[NEXWATCH_ENCODED_DATA_SIZE];
    uint8_t negative_encoded_data[NEXWATCH_ENCODED_DATA_SIZE];
    uint8_t corrupted_encoded_data[NEXWATCH_ENCODED_DATA_SIZE];
    uint8_t corrupted_negative_encoded_data[NEXWATCH_ENCODED_DATA_SIZE];

    uint8_t data[NEXWATCH_DECODED_DATA_SIZE];
    ProtocolNexwatchEncoder encoder;
} ProtocolNexwatch;

ProtocolNexwatch* protocol_nexwatch_alloc(void) {
    ProtocolNexwatch* protocol = malloc(sizeof(ProtocolNexwatch));
    return protocol;
}

void protocol_nexwatch_free(ProtocolNexwatch* protocol) {
    free(protocol);
}

uint8_t* protocol_nexwatch_get_data(ProtocolNexwatch* protocol) {
    return protocol->data;
}

void protocol_nexwatch_decoder_start(ProtocolNexwatch* protocol) {
    memset(protocol->encoded_data, 0, NEXWATCH_ENCODED_DATA_SIZE);
    memset(protocol->negative_encoded_data, 0, NEXWATCH_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_encoded_data, 0, NEXWATCH_ENCODED_DATA_SIZE);
    memset(protocol->corrupted_negative_encoded_data, 0, NEXWATCH_ENCODED_DATA_SIZE);
}

static bool protocol_nexwatch_check_preamble(uint8_t* data, size_t bit_index) {
    // 01010110
    if(bit_lib_get_bits(data, bit_index, 8) != 0b01010110) return false;
    return true;
}

static uint8_t protocol_nexwatch_parity_swap(uint8_t parity) {
    uint8_t a = ((parity >> 3) & 1);
    a |= (((parity >> 1) & 1) << 1);
    a |= (((parity >> 2) & 1) << 2);
    a |= ((parity & 1) << 3);
    return a;
}

static uint8_t protocol_nexwatch_parity(const uint8_t hexid[5]) {
    uint8_t p = 0;
    for(uint8_t i = 0; i < 5; i++) {
        p ^= ((hexid[i]) & 0xF0) >> 4;
        p ^= ((hexid[i]) & 0x0F);
    }
    return protocol_nexwatch_parity_swap(p);
}

static uint8_t protocol_nexwatch_checksum(uint8_t magic, uint32_t id, uint8_t parity) {
    uint8_t a = ((id >> 24) & 0xFF);
    a -= ((id >> 16) & 0xFF);
    a -= ((id >> 8) & 0xFF);
    a -= (id & 0xFF);
    a -= magic;
    a -= (bit_lib_reverse_8_fast(parity) >> 4);
    return bit_lib_reverse_8_fast(a);
}

static bool protocol_nexwatch_can_be_decoded(uint8_t* data) {
    if(!protocol_nexwatch_check_preamble(data, 0)) return false;

    // Check for reserved word (32-bit)
    if(bit_lib_get_bits_32(data, 8, 32) != 0) {
        return false;
    }

    uint8_t parity = bit_lib_get_bits(data, 76, 4);

    // parity check
    // from 32b hex id, 4b mode
    uint8_t hex[5] = {0};
    for(uint8_t i = 0; i < 5; i++) {
        hex[i] = bit_lib_get_bits(data, 40 + (i * 8), 8);
    }
    //mode is only 4 bits.
    hex[4] &= 0xf0;
    uint8_t calc_parity = protocol_nexwatch_parity(hex);

    if(calc_parity != parity) {
        return false;
    }

    return true;
}

static bool protocol_nexwatch_decoder_feed_internal(bool polarity, uint32_t time, uint8_t* data) {
    time += (NEXWATCH_US_PER_BIT / 2);

    size_t bit_count = (time / NEXWATCH_US_PER_BIT);
    bool result = false;

    if(bit_count < NEXWATCH_ENCODED_BIT_SIZE) {
        for(size_t i = 0; i < bit_count; i++) {
            bit_lib_push_bit(data, NEXWATCH_ENCODED_DATA_SIZE, polarity);
            if(protocol_nexwatch_can_be_decoded(data)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

static void protocol_nexwatch_descramble(uint32_t* id, uint32_t* scrambled) {
    // 255 = Not used/Unknown other values are the bit offset in the ID/FC values
    const uint8_t hex_2_id[] = {31, 27, 23, 19, 15, 11, 7, 3, 30, 26, 22, 18, 14, 10, 6, 2,
                                29, 25, 21, 17, 13, 9,  5, 1, 28, 24, 20, 16, 12, 8,  4, 0};

    *id = 0;
    for(uint8_t idx = 0; idx < 32; idx++) {
        bool bit_state = (*scrambled >> hex_2_id[idx]) & 1;
        *id |= (bit_state << (31 - idx));
    }
}

static void protocol_nexwatch_decoder_save(uint8_t* data_to, const uint8_t* data_from) {
    uint32_t id = bit_lib_get_bits_32(data_from, 40, 32);
    data_to[4] = (uint8_t)id;
    data_to[3] = (uint8_t)(id >>= 8);
    data_to[2] = (uint8_t)(id >>= 8);
    data_to[1] = (uint8_t)(id >>= 8);
    data_to[0] = (uint8_t)(id >>= 8);
    uint32_t check = bit_lib_get_bits_32(data_from, 72, 24);
    data_to[7] = (uint8_t)check;
    data_to[6] = (uint8_t)(check >>= 8);
    data_to[5] = (uint8_t)(check >>= 8);
}

bool protocol_nexwatch_decoder_feed(ProtocolNexwatch* protocol, bool level, uint32_t duration) {
    bool result = false;

    if(duration > (NEXWATCH_US_PER_BIT / 2)) {
        if(protocol_nexwatch_decoder_feed_internal(level, duration, protocol->encoded_data)) {
            protocol_nexwatch_decoder_save(protocol->data, protocol->encoded_data);
            result = true;
            return result;
        }

        if(protocol_nexwatch_decoder_feed_internal(
               !level, duration, protocol->negative_encoded_data)) {
            protocol_nexwatch_decoder_save(protocol->data, protocol->negative_encoded_data);
            result = true;
            return result;
        }
    }

    if(duration > (NEXWATCH_US_PER_BIT / 4)) {
        // Try to decode wrong phase synced data
        if(level) {
            duration += 120;
        } else {
            if(duration > 120) {
                duration -= 120;
            }
        }

        if(protocol_nexwatch_decoder_feed_internal(
               level, duration, protocol->corrupted_encoded_data)) {
            protocol_nexwatch_decoder_save(protocol->data, protocol->corrupted_encoded_data);

            result = true;
            return result;
        }

        if(protocol_nexwatch_decoder_feed_internal(
               !level, duration, protocol->corrupted_negative_encoded_data)) {
            protocol_nexwatch_decoder_save(
                protocol->data, protocol->corrupted_negative_encoded_data);

            result = true;
            return result;
        }
    }

    return result;
}

bool protocol_nexwatch_encoder_start(ProtocolNexwatch* protocol) {
    memset(protocol->encoded_data, 0, NEXWATCH_ENCODED_DATA_SIZE);
    *(uint32_t*)&protocol->encoded_data[0] = 0b00000000000000000000000001010110;
    bit_lib_copy_bits(protocol->encoded_data, 32, 32, protocol->data, 0);
    bit_lib_copy_bits(protocol->encoded_data, 64, 32, protocol->data, 32);

    protocol->encoder.last_bit =
        bit_lib_get_bit(protocol->encoded_data, NEXWATCH_ENCODED_BIT_SIZE - 1);
    protocol->encoder.data_index = 0;
    protocol->encoder.current_polarity = true;
    protocol->encoder.pulse_phase = true;
    protocol->encoder.bit_clock_index = 0;

    return true;
}

LevelDuration protocol_nexwatch_encoder_yield(ProtocolNexwatch* protocol) {
    LevelDuration level_duration;
    ProtocolNexwatchEncoder* encoder = &protocol->encoder;

    if(encoder->pulse_phase) {
        level_duration = level_duration_make(encoder->current_polarity, 1);
        encoder->pulse_phase = false;
    } else {
        level_duration = level_duration_make(!encoder->current_polarity, 1);
        encoder->pulse_phase = true;

        encoder->bit_clock_index++;
        if(encoder->bit_clock_index >= NEXWATCH_ENCODER_PULSES_PER_BIT) {
            encoder->bit_clock_index = 0;

            bool current_bit = bit_lib_get_bit(protocol->encoded_data, encoder->data_index);

            if(current_bit != encoder->last_bit) {
                encoder->current_polarity = !encoder->current_polarity;
            }

            encoder->last_bit = current_bit;

            bit_lib_increment_index(encoder->data_index, NEXWATCH_ENCODED_BIT_SIZE);
        }
    }

    return level_duration;
}

static void protocol_nexwatch_render_data_internal(
    ProtocolNexwatch* protocol,
    FuriString* result,
    bool brief) {
    uint32_t id = 0;
    uint32_t scrambled = bit_lib_get_bits_32(protocol->data, 8, 32);
    protocol_nexwatch_descramble(&id, &scrambled);

    uint8_t m_idx;
    uint8_t mode = bit_lib_get_bits(protocol->data, 40, 4);
    uint8_t parity = bit_lib_get_bits(protocol->data, 44, 4);
    uint8_t chk = bit_lib_get_bits(protocol->data, 48, 8);

    for(m_idx = 0; m_idx < COUNT_OF(magic_items); m_idx++) {
        magic_items[m_idx].chk = protocol_nexwatch_checksum(magic_items[m_idx].magic, id, parity);
        if(magic_items[m_idx].chk == chk) {
            break;
        }
    }

    const char* type = m_idx < COUNT_OF(magic_items) ? magic_items[m_idx].desc : "Unknown";

    if(brief) {
        furi_string_printf(
            result,
            "ID: %lu\n"
            "Mode: %hhu; Type: %s",
            id,
            mode,
            type);
    } else {
        furi_string_printf(
            result,
            "ID: %lu\n"
            "Mode: %hhu\n"
            "Type: %s",
            id,
            mode,
            type);
    }
}

void protocol_nexwatch_render_data(ProtocolNexwatch* protocol, FuriString* result) {
    protocol_nexwatch_render_data_internal(protocol, result, false);
}

void protocol_nexwatch_render_brief_data(ProtocolNexwatch* protocol, FuriString* result) {
    protocol_nexwatch_render_data_internal(protocol, result, true);
}

bool protocol_nexwatch_write_data(ProtocolNexwatch* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_nexwatch_encoder_start(protocol);
    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_PSK1 | LFRFID_T5577_BITRATE_RF_32 |
                                  (3 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
}

const ProtocolBase protocol_nexwatch = {
    .name = "Nexwatch",
    .manufacturer = "Honeywell",
    .data_size = NEXWATCH_DECODED_DATA_SIZE,
    .features = LFRFIDFeaturePSK,
    .validate_count = 6,
    .alloc = (ProtocolAlloc)protocol_nexwatch_alloc,
    .free = (ProtocolFree)protocol_nexwatch_free,
    .get_data = (ProtocolGetData)protocol_nexwatch_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_nexwatch_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_nexwatch_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_nexwatch_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_nexwatch_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_nexwatch_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_nexwatch_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_nexwatch_write_data,
};
