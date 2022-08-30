#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include <lfrfid/tools/bit_lib.h>
#include "lfrfid_protocols.h"

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define AWID_DECODED_DATA_SIZE (9)

#define AWID_ENCODED_BIT_SIZE (96)
#define AWID_ENCODED_DATA_SIZE (((AWID_ENCODED_BIT_SIZE) / 8) + 1)
#define AWID_ENCODED_DATA_LAST (AWID_ENCODED_DATA_SIZE - 1)

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolAwidDecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
} ProtocolAwidEncoder;

typedef struct {
    ProtocolAwidDecoder decoder;
    ProtocolAwidEncoder encoder;
    uint8_t encoded_data[AWID_ENCODED_DATA_SIZE];
    uint8_t data[AWID_DECODED_DATA_SIZE];
} ProtocolAwid;

ProtocolAwid* protocol_awid_alloc(void) {
    ProtocolAwid* protocol = malloc(sizeof(ProtocolAwid));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 6, MAX_TIME, 5);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 50);

    return protocol;
};

void protocol_awid_free(ProtocolAwid* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_awid_get_data(ProtocolAwid* protocol) {
    return protocol->data;
};

void protocol_awid_decoder_start(ProtocolAwid* protocol) {
    memset(protocol->encoded_data, 0, AWID_ENCODED_DATA_SIZE);
};

static bool protocol_awid_can_be_decoded(uint8_t* data) {
    bool result = false;

    // Index map
    // 0            10            20            30              40            50              60
    // |            |             |             |               |             |               |
    // 01234567 890 1 234 5 678 9 012 3 456 7 890 1 234 5 678 9 012 3 456 7 890 1 234 5 678 9 012 3 - to 96
    // -----------------------------------------------------------------------------
    // 00000001 000 1 110 1 101 1 011 1 101 1 010 0 000 1 000 1 010 0 001 0 110 1 100 0 000 1 000 1
    // preamble bbb o bbb o bbw o fff o fff o ffc o ccc o ccc o ccc o ccc o ccc o wxx o xxx o xxx o - to 96
    //          |---26 bit---|    |-----117----||-------------142-------------|
    // b = format bit len, o = odd parity of last 3 bits
    // f = facility code, c = card number
    // w = wiegand parity
    // (26 bit format shown)

    do {
        // check preamble and spacing
        if(data[0] != 0b00000001 || data[AWID_ENCODED_DATA_LAST] != 0b00000001) break;

        // check odd parity for every 4 bits starting from the second byte
        bool parity_error = bit_lib_test_parity(data, 8, 88, BitLibParityOdd, 4);
        if(parity_error) break;

        bit_lib_remove_bit_every_nth(data, 8, 88, 4);

        // Avoid detection for invalid formats
        uint8_t len = bit_lib_get_bits(data, 8, 8);
        if(len != 26 && len != 50 && len != 37 && len != 34) break;

        result = true;
    } while(false);

    return result;
}

static void protocol_awid_decode(uint8_t* encoded_data, uint8_t* decoded_data) {
    bit_lib_copy_bits(decoded_data, 0, 66, encoded_data, 8);
}

bool protocol_awid_decoder_feed(ProtocolAwid* protocol, bool level, uint32_t duration) {
    bool value;
    uint32_t count;
    bool result = false;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    if(count > 0) {
        for(size_t i = 0; i < count; i++) {
            bit_lib_push_bit(protocol->encoded_data, AWID_ENCODED_DATA_SIZE, value);
            if(protocol_awid_can_be_decoded(protocol->encoded_data)) {
                protocol_awid_decode(protocol->encoded_data, protocol->data);

                result = true;
                break;
            }
        }
    }

    return result;
};

static void protocol_awid_encode(const uint8_t* decoded_data, uint8_t* encoded_data) {
    memset(encoded_data, 0, AWID_ENCODED_DATA_SIZE);

    // preamble
    bit_lib_set_bits(encoded_data, 0, 0b00000001, 8);

    for(size_t i = 0; i < 88 / 4; i++) {
        uint8_t value = bit_lib_get_bits(decoded_data, i * 3, 3) << 1;
        value |= bit_lib_test_parity_32(value, BitLibParityOdd);
        bit_lib_set_bits(encoded_data, 8 + i * 4, value, 4);
    }
};

bool protocol_awid_encoder_start(ProtocolAwid* protocol) {
    protocol_awid_encode(protocol->data, (uint8_t*)protocol->encoded_data);
    protocol->encoder.encoded_index = 0;
    fsk_osc_reset(protocol->encoder.fsk_osc);
    return true;
};

LevelDuration protocol_awid_encoder_yield(ProtocolAwid* protocol) {
    bool level;
    uint32_t duration;

    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);
    bool advance = fsk_osc_next_half(protocol->encoder.fsk_osc, bit, &level, &duration);

    if(advance) {
        bit_lib_increment_index(protocol->encoder.encoded_index, AWID_ENCODED_BIT_SIZE);
    }
    return level_duration_make(level, duration);
};

void protocol_awid_render_data(ProtocolAwid* protocol, string_t result) {
    // Index map
    // 0           10         20        30          40        50        60
    // |           |          |         |           |         |         |
    // 01234567 8 90123456 7890123456789012 3 456789012345678901234567890123456
    // ------------------------------------------------------------------------
    // 00011010 1 01110101 0000000010001110 1 000000000000000000000000000000000
    // bbbbbbbb w ffffffff cccccccccccccccc w xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // |26 bit|   |-117--| |-----142------|
    // b = format bit len, o = odd parity of last 3 bits
    // f = facility code, c = card number
    // w = wiegand parity
    // (26 bit format shown)

    uint8_t* decoded_data = protocol->data;
    uint8_t format_length = decoded_data[0];

    string_cat_printf(result, "Format: %d\r\n", format_length);
    if(format_length == 26) {
        uint8_t facility;
        bit_lib_copy_bits(&facility, 0, 8, decoded_data, 9);

        uint16_t card_id;
        bit_lib_copy_bits((uint8_t*)&card_id, 8, 8, decoded_data, 17);
        bit_lib_copy_bits((uint8_t*)&card_id, 0, 8, decoded_data, 25);
        string_cat_printf(result, "Facility: %d\r\n", facility);
        string_cat_printf(result, "Card: %d", card_id);
    } else {
        // print 66 bits as hex
        string_cat_printf(result, "Data: ");
        for(size_t i = 0; i < AWID_DECODED_DATA_SIZE; i++) {
            string_cat_printf(result, "%02X", decoded_data[i]);
        }
    }
};

void protocol_awid_render_brief_data(ProtocolAwid* protocol, string_t result) {
    uint8_t* decoded_data = protocol->data;
    uint8_t format_length = decoded_data[0];

    string_cat_printf(result, "Format: %d\r\n", format_length);
    if(format_length == 26) {
        uint8_t facility;
        bit_lib_copy_bits(&facility, 0, 8, decoded_data, 9);

        uint16_t card_id;
        bit_lib_copy_bits((uint8_t*)&card_id, 8, 8, decoded_data, 17);
        bit_lib_copy_bits((uint8_t*)&card_id, 0, 8, decoded_data, 25);
        string_cat_printf(result, "ID: %03u,%05u", facility, card_id);
    } else {
        string_cat_printf(result, "Data: unknown");
    }
};

bool protocol_awid_write_data(ProtocolAwid* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_awid_encode(protocol->data, (uint8_t*)protocol->encoded_data);

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

const ProtocolBase protocol_awid = {
    .name = "AWID",
    .manufacturer = "AWIG",
    .data_size = AWID_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_awid_alloc,
    .free = (ProtocolFree)protocol_awid_free,
    .get_data = (ProtocolGetData)protocol_awid_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_awid_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_awid_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_awid_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_awid_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_awid_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_awid_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_awid_write_data,
};