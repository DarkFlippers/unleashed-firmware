#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <lfrfid/tools/fsk_demod.h>
#include <lfrfid/tools/fsk_osc.h>
#include <lfrfid/tools/bit_lib.h>
#include "lfrfid_protocols.h"

#define JITTER_TIME (20)
#define MIN_TIME (64 - JITTER_TIME)
#define MAX_TIME (80 + JITTER_TIME)

#define IOPROXXSF_DECODED_DATA_SIZE (4)
#define IOPROXXSF_ENCODED_DATA_SIZE (8)

#define IOPROXXSF_BIT_SIZE (8)
#define IOPROXXSF_BIT_MAX_SIZE (IOPROXXSF_BIT_SIZE * IOPROXXSF_ENCODED_DATA_SIZE)

typedef struct {
    FSKDemod* fsk_demod;
} ProtocolIOProxXSFDecoder;

typedef struct {
    FSKOsc* fsk_osc;
    uint8_t encoded_index;
} ProtocolIOProxXSFEncoder;

typedef struct {
    ProtocolIOProxXSFEncoder encoder;
    ProtocolIOProxXSFDecoder decoder;
    uint8_t encoded_data[IOPROXXSF_ENCODED_DATA_SIZE];
    uint8_t data[IOPROXXSF_DECODED_DATA_SIZE];
} ProtocolIOProxXSF;

ProtocolIOProxXSF* protocol_io_prox_xsf_alloc(void) {
    ProtocolIOProxXSF* protocol = malloc(sizeof(ProtocolIOProxXSF));
    protocol->decoder.fsk_demod = fsk_demod_alloc(MIN_TIME, 8, MAX_TIME, 6);
    protocol->encoder.fsk_osc = fsk_osc_alloc(8, 10, 64);
    return protocol;
};

void protocol_io_prox_xsf_free(ProtocolIOProxXSF* protocol) {
    fsk_demod_free(protocol->decoder.fsk_demod);
    fsk_osc_free(protocol->encoder.fsk_osc);
    free(protocol);
};

uint8_t* protocol_io_prox_xsf_get_data(ProtocolIOProxXSF* protocol) {
    return protocol->data;
};

void protocol_io_prox_xsf_decoder_start(ProtocolIOProxXSF* protocol) {
    memset(protocol->encoded_data, 0, IOPROXXSF_ENCODED_DATA_SIZE);
};

static uint8_t protocol_io_prox_xsf_compute_checksum(const uint8_t* data) {
    // Packet structure:
    //
    //0        1        2         3         4         5         6         7
    //v        v        v         v         v         v         v         v
    //01234567 8 9ABCDEF0 1 23456789 A BCDEF012 3 456789AB C DEF01234 5 6789ABCD EF
    //00000000 0 VVVVVVVV 1 WWWWWWWW 1 XXXXXXXX 1 YYYYYYYY 1 ZZZZZZZZ 1 CHECKSUM 11
    //
    // algorithm as observed by the proxmark3 folks
    // CHECKSUM == 0xFF - (V + W + X + Y + Z)

    uint8_t checksum = 0;

    for(size_t i = 1; i <= 5; i++) {
        checksum += bit_lib_get_bits(data, 9 * i, 8);
    }

    return 0xFF - checksum;
}

static bool protocol_io_prox_xsf_can_be_decoded(const uint8_t* encoded_data) {
    // Packet framing
    //
    //0        1        2        3        4        5        6        7
    //v        v        v        v        v        v        v        v
    //01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF
    //-----------------------------------------------------------------------
    //00000000 01______ _1______ __1_____ ___1____ ____1___ _____1XX XXXXXX11
    //
    // _ = variable data
    // 0 = preamble 0
    // 1 = framing 1
    // X = checksum

    // Validate the packet preamble is there...
    if(encoded_data[0] != 0b00000000) {
        return false;
    }
    if((encoded_data[1] >> 6) != 0b01) {
        return false;
    }

    // ... check for known ones...
    if(bit_lib_bit_is_not_set(encoded_data[2], 6)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[3], 5)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[4], 4)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[5], 3)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[6], 2)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[7], 1)) {
        return false;
    }
    if(bit_lib_bit_is_not_set(encoded_data[7], 0)) {
        return false;
    }

    // ... and validate our checksums.
    uint8_t checksum = protocol_io_prox_xsf_compute_checksum(encoded_data);
    uint8_t checkval = bit_lib_get_bits(encoded_data, 54, 8);

    if(checksum != checkval) {
        return false;
    }

    return true;
}

void protocol_io_prox_xsf_decode(const uint8_t* encoded_data, uint8_t* decoded_data) {
    // Packet structure:
    // (Note: the second word seems fixed; but this may not be a guarantee;
    //  it currently has no meaning.)
    //
    //0        1        2        3        4        5        6        7
    //v        v        v        v        v        v        v        v
    //01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF
    //-----------------------------------------------------------------------
    //00000000 01111000 01FFFFFF FF1VVVVV VVV1CCCC CCCC1CCC CCCCC1XX XXXXXX11
    //
    // F = facility code
    // V = version
    // C = code
    // X = checksum

    // Facility code
    decoded_data[0] = bit_lib_get_bits(encoded_data, 18, 8);

    // Version code.
    decoded_data[1] = bit_lib_get_bits(encoded_data, 27, 8);

    // Code bytes.
    decoded_data[2] = bit_lib_get_bits(encoded_data, 36, 8);
    decoded_data[3] = bit_lib_get_bits(encoded_data, 45, 8);
}

bool protocol_io_prox_xsf_decoder_feed(ProtocolIOProxXSF* protocol, bool level, uint32_t duration) {
    bool result = false;

    uint32_t count;
    bool value;

    fsk_demod_feed(protocol->decoder.fsk_demod, level, duration, &value, &count);
    for(size_t i = 0; i < count; i++) {
        bit_lib_push_bit(protocol->encoded_data, IOPROXXSF_ENCODED_DATA_SIZE, value);
        if(protocol_io_prox_xsf_can_be_decoded(protocol->encoded_data)) {
            protocol_io_prox_xsf_decode(protocol->encoded_data, protocol->data);
            result = true;
            break;
        }
    }

    return result;
};

static void protocol_io_prox_xsf_encode(const uint8_t* decoded_data, uint8_t* encoded_data) {
    // Packet to transmit:
    //
    // 0           10          20          30          40          50          60
    // v           v           v           v           v           v           v
    // 01234567 8 90123456 7 89012345 6 78901234 5 67890123 4 56789012 3 45678901 23
    // -----------------------------------------------------------------------------
    // 00000000 0 11110000 1 facility 1 version_ 1 code-one 1 code-two 1 checksum 11

    // Preamble.
    bit_lib_set_bits(encoded_data, 0, 0b00000000, 8);
    bit_lib_set_bit(encoded_data, 8, 0);

    bit_lib_set_bits(encoded_data, 9, 0b11110000, 8);
    bit_lib_set_bit(encoded_data, 17, 1);

    // Facility code.
    bit_lib_set_bits(encoded_data, 18, decoded_data[0], 8);
    bit_lib_set_bit(encoded_data, 26, 1);

    // Version
    bit_lib_set_bits(encoded_data, 27, decoded_data[1], 8);
    bit_lib_set_bit(encoded_data, 35, 1);

    // Code one
    bit_lib_set_bits(encoded_data, 36, decoded_data[2], 8);
    bit_lib_set_bit(encoded_data, 44, 1);

    // Code two
    bit_lib_set_bits(encoded_data, 45, decoded_data[3], 8);
    bit_lib_set_bit(encoded_data, 53, 1);

    // Checksum
    bit_lib_set_bits(encoded_data, 54, protocol_io_prox_xsf_compute_checksum(encoded_data), 8);
    bit_lib_set_bit(encoded_data, 62, 1);
    bit_lib_set_bit(encoded_data, 63, 1);
}

bool protocol_io_prox_xsf_encoder_start(ProtocolIOProxXSF* protocol) {
    protocol_io_prox_xsf_encode(protocol->data, protocol->encoded_data);
    protocol->encoder.encoded_index = 0;
    fsk_osc_reset(protocol->encoder.fsk_osc);
    return true;
};

LevelDuration protocol_io_prox_xsf_encoder_yield(ProtocolIOProxXSF* protocol) {
    bool level;
    uint32_t duration;

    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoder.encoded_index);
    bool advance = fsk_osc_next_half(protocol->encoder.fsk_osc, bit, &level, &duration);

    if(advance) {
        bit_lib_increment_index(protocol->encoder.encoded_index, IOPROXXSF_BIT_MAX_SIZE);
    }
    return level_duration_make(level, duration);
};

void protocol_io_prox_xsf_render_data(ProtocolIOProxXSF* protocol, string_t result) {
    uint8_t* data = protocol->data;
    string_printf(
        result,
        "FC: %u\r\n"
        "VС: %u\r\n"
        "Card: %u",
        data[0],
        data[1],
        (uint16_t)((data[2] << 8) | (data[3])));
}

void protocol_io_prox_xsf_render_brief_data(ProtocolIOProxXSF* protocol, string_t result) {
    uint8_t* data = protocol->data;
    string_printf(
        result,
        "FC: %u, VС: %u\r\n"
        "Card: %u",
        data[0],
        data[1],
        (uint16_t)((data[2] << 8) | (data[3])));
}

bool protocol_io_prox_xsf_write_data(ProtocolIOProxXSF* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_io_prox_xsf_encode(protocol->data, protocol->encoded_data);
    protocol_io_prox_xsf_decode(protocol->encoded_data, protocol->data);

    protocol_io_prox_xsf_encode(protocol->data, protocol->encoded_data);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_FSK2a | LFRFID_T5577_BITRATE_RF_64 |
                                  (2 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.blocks_to_write = 3;
        result = true;
    }
    return result;
};

const ProtocolBase protocol_io_prox_xsf = {
    .name = "IoProxXSF",
    .manufacturer = "Kantech",
    .data_size = IOPROXXSF_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_io_prox_xsf_alloc,
    .free = (ProtocolFree)protocol_io_prox_xsf_free,
    .get_data = (ProtocolGetData)protocol_io_prox_xsf_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_io_prox_xsf_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_io_prox_xsf_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_io_prox_xsf_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_io_prox_xsf_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_io_prox_xsf_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_io_prox_xsf_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_io_prox_xsf_write_data,
};