#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include <lfrfid/tools/bit_lib.h>
#include "lfrfid_protocols.h"

#define GALLAGHER_CLOCK_PER_BIT (32)

#define GALLAGHER_ENCODED_BIT_SIZE (96)
#define GALLAGHER_ENCODED_BYTE_SIZE ((GALLAGHER_ENCODED_BIT_SIZE) / 8)
#define GALLAGHER_PREAMBLE_BIT_SIZE (16)
#define GALLAGHER_PREAMBLE_BYTE_SIZE ((GALLAGHER_PREAMBLE_BIT_SIZE) / 8)
#define GALLAGHER_ENCODED_BYTE_FULL_SIZE \
    (GALLAGHER_ENCODED_BYTE_SIZE + GALLAGHER_PREAMBLE_BYTE_SIZE)
#define GALLAGHER_DECODED_DATA_SIZE 8

#define GALLAGHER_READ_SHORT_TIME (128)
#define GALLAGHER_READ_LONG_TIME (256)
#define GALLAGHER_READ_JITTER_TIME (60)

#define GALLAGHER_READ_SHORT_TIME_LOW (GALLAGHER_READ_SHORT_TIME - GALLAGHER_READ_JITTER_TIME)
#define GALLAGHER_READ_SHORT_TIME_HIGH (GALLAGHER_READ_SHORT_TIME + GALLAGHER_READ_JITTER_TIME)
#define GALLAGHER_READ_LONG_TIME_LOW (GALLAGHER_READ_LONG_TIME - GALLAGHER_READ_JITTER_TIME)
#define GALLAGHER_READ_LONG_TIME_HIGH (GALLAGHER_READ_LONG_TIME + GALLAGHER_READ_JITTER_TIME)

typedef struct {
    uint8_t data[GALLAGHER_DECODED_DATA_SIZE];
    uint8_t encoded_data[GALLAGHER_ENCODED_BYTE_FULL_SIZE];

    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolGallagher;

ProtocolGallagher* protocol_gallagher_alloc(void) {
    ProtocolGallagher* proto = malloc(sizeof(ProtocolGallagher));
    return (void*)proto;
};

void protocol_gallagher_free(ProtocolGallagher* protocol) {
    free(protocol);
};

uint8_t* protocol_gallagher_get_data(ProtocolGallagher* protocol) {
    return protocol->data;
};

static void protocol_gallagher_scramble(uint8_t* data, size_t length) {
    const uint8_t lut[] = {
        0xa3, 0xb0, 0x80, 0xc6, 0xb2, 0xf4, 0x5c, 0x6c, 0x81, 0xf1, 0xbb, 0xeb, 0x55, 0x67, 0x3c,
        0x05, 0x1a, 0x0e, 0x61, 0xf6, 0x22, 0xce, 0xaa, 0x8f, 0xbd, 0x3b, 0x1f, 0x5e, 0x44, 0x04,
        0x51, 0x2e, 0x4d, 0x9a, 0x84, 0xea, 0xf8, 0x66, 0x74, 0x29, 0x7f, 0x70, 0xd8, 0x31, 0x7a,
        0x6d, 0xa4, 0x00, 0x82, 0xb9, 0x5f, 0xb4, 0x16, 0xab, 0xff, 0xc2, 0x39, 0xdc, 0x19, 0x65,
        0x57, 0x7c, 0x20, 0xfa, 0x5a, 0x49, 0x13, 0xd0, 0xfb, 0xa8, 0x91, 0x73, 0xb1, 0x33, 0x18,
        0xbe, 0x21, 0x72, 0x48, 0xb6, 0xdb, 0xa0, 0x5d, 0xcc, 0xe6, 0x17, 0x27, 0xe5, 0xd4, 0x53,
        0x42, 0xf3, 0xdd, 0x7b, 0x24, 0xac, 0x2b, 0x58, 0x1e, 0xa7, 0xe7, 0x86, 0x40, 0xd3, 0x98,
        0x97, 0x71, 0xcb, 0x3a, 0x0f, 0x01, 0x9b, 0x6e, 0x1b, 0xfc, 0x34, 0xa6, 0xda, 0x07, 0x0c,
        0xae, 0x37, 0xca, 0x54, 0xfd, 0x26, 0xfe, 0x0a, 0x45, 0xa2, 0x2a, 0xc4, 0x12, 0x0d, 0xf5,
        0x4f, 0x69, 0xe0, 0x8a, 0x77, 0x60, 0x3f, 0x99, 0x95, 0xd2, 0x38, 0x36, 0x62, 0xb7, 0x32,
        0x7e, 0x79, 0xc0, 0x46, 0x93, 0x2f, 0xa5, 0xba, 0x5b, 0xaf, 0x52, 0x1d, 0xc3, 0x75, 0xcf,
        0xd6, 0x4c, 0x83, 0xe8, 0x3d, 0x30, 0x4e, 0xbc, 0x08, 0x2d, 0x09, 0x06, 0xd9, 0x25, 0x9e,
        0x89, 0xf2, 0x96, 0x88, 0xc1, 0x8c, 0x94, 0x0b, 0x28, 0xf0, 0x47, 0x63, 0xd5, 0xb3, 0x68,
        0x56, 0x9c, 0xf9, 0x6f, 0x41, 0x50, 0x85, 0x8b, 0x9d, 0x59, 0xbf, 0x9f, 0xe2, 0x8e, 0x6a,
        0x11, 0x23, 0xa1, 0xcd, 0xb5, 0x7d, 0xc7, 0xa9, 0xc8, 0xef, 0xdf, 0x02, 0xb8, 0x03, 0x6b,
        0x35, 0x3e, 0x2c, 0x76, 0xc9, 0xde, 0x1c, 0x4b, 0xd1, 0xed, 0x14, 0xc5, 0xad, 0xe9, 0x64,
        0x4a, 0xec, 0x8d, 0xf7, 0x10, 0x43, 0x78, 0x15, 0x87, 0xe4, 0xd7, 0x92, 0xe1, 0xee, 0xe3,
        0x90};
    for(size_t i = 0; i < length; i++) {
        data[i] = lut[data[i]];
    }
}

static void protocol_gallagher_descramble(uint8_t* data, size_t length) {
    const uint8_t lut[] = {
        0x2f, 0x6e, 0xdd, 0xdf, 0x1d, 0x0f, 0xb0, 0x76, 0xad, 0xaf, 0x7f, 0xbb, 0x77, 0x85, 0x11,
        0x6d, 0xf4, 0xd2, 0x84, 0x42, 0xeb, 0xf7, 0x34, 0x55, 0x4a, 0x3a, 0x10, 0x71, 0xe7, 0xa1,
        0x62, 0x1a, 0x3e, 0x4c, 0x14, 0xd3, 0x5e, 0xb2, 0x7d, 0x56, 0xbc, 0x27, 0x82, 0x60, 0xe3,
        0xae, 0x1f, 0x9b, 0xaa, 0x2b, 0x95, 0x49, 0x73, 0xe1, 0x92, 0x79, 0x91, 0x38, 0x6c, 0x19,
        0x0e, 0xa9, 0xe2, 0x8d, 0x66, 0xc7, 0x5a, 0xf5, 0x1c, 0x80, 0x99, 0xbe, 0x4e, 0x41, 0xf0,
        0xe8, 0xa6, 0x20, 0xab, 0x87, 0xc8, 0x1e, 0xa0, 0x59, 0x7b, 0x0c, 0xc3, 0x3c, 0x61, 0xcc,
        0x40, 0x9e, 0x06, 0x52, 0x1b, 0x32, 0x8c, 0x12, 0x93, 0xbf, 0xef, 0x3b, 0x25, 0x0d, 0xc2,
        0x88, 0xd1, 0xe0, 0x07, 0x2d, 0x70, 0xc6, 0x29, 0x6a, 0x4d, 0x47, 0x26, 0xa3, 0xe4, 0x8b,
        0xf6, 0x97, 0x2c, 0x5d, 0x3d, 0xd7, 0x96, 0x28, 0x02, 0x08, 0x30, 0xa7, 0x22, 0xc9, 0x65,
        0xf8, 0xb7, 0xb4, 0x8a, 0xca, 0xb9, 0xf2, 0xd0, 0x17, 0xff, 0x46, 0xfb, 0x9a, 0xba, 0x8f,
        0xb6, 0x69, 0x68, 0x8e, 0x21, 0x6f, 0xc4, 0xcb, 0xb3, 0xce, 0x51, 0xd4, 0x81, 0x00, 0x2e,
        0x9c, 0x74, 0x63, 0x45, 0xd9, 0x16, 0x35, 0x5f, 0xed, 0x78, 0x9f, 0x01, 0x48, 0x04, 0xc1,
        0x33, 0xd6, 0x4f, 0x94, 0xde, 0x31, 0x9d, 0x0a, 0xac, 0x18, 0x4b, 0xcd, 0x98, 0xb8, 0x37,
        0xa2, 0x83, 0xec, 0x03, 0xd8, 0xda, 0xe5, 0x7a, 0x6b, 0x53, 0xd5, 0x15, 0xa4, 0x43, 0xe9,
        0x90, 0x67, 0x58, 0xc0, 0xa5, 0xfa, 0x2a, 0xb1, 0x75, 0x50, 0x39, 0x5c, 0xe6, 0xdc, 0x89,
        0xfc, 0xcf, 0xfe, 0xf9, 0x57, 0x54, 0x64, 0xa8, 0xee, 0x23, 0x0b, 0xf1, 0xea, 0xfd, 0xdb,
        0xbd, 0x09, 0xb5, 0x5b, 0x05, 0x86, 0x13, 0xf3, 0x24, 0xc5, 0x3f, 0x44, 0x72, 0x7c, 0x7e,
        0x36};

    for(size_t i = 0; i < length; i++) {
        data[i] = lut[data[i]];
    }
}

static void protocol_gallagher_decode(ProtocolGallagher* protocol) {
    bit_lib_remove_bit_every_nth(protocol->encoded_data, 16, 9 * 8, 9);
    protocol_gallagher_descramble(protocol->encoded_data + 2, 8);

    // Region code
    bit_lib_set_bits(protocol->data, 0, (protocol->encoded_data[5] & 0x1E) >> 1, 4);

    // Issue Level
    bit_lib_set_bits(protocol->data, 4, (protocol->encoded_data[9] & 0x0F), 4);

    // Facility Code
    uint32_t fc = (protocol->encoded_data[7] & 0x0F) << 12 | protocol->encoded_data[3] << 4 |
                  ((protocol->encoded_data[9] >> 4) & 0x0F);
    protocol->data[3] = (uint8_t)fc;
    protocol->data[2] = (uint8_t)(fc >>= 8);
    protocol->data[1] = (uint8_t)(fc >>= 8);

    // Card Number
    uint32_t card = protocol->encoded_data[2] << 16 | (protocol->encoded_data[6] & 0x1F) << 11 |
                    protocol->encoded_data[4] << 3 | (protocol->encoded_data[5] & 0xE0) >> 5;
    protocol->data[7] = (uint8_t)card;
    protocol->data[6] = (uint8_t)(card >>= 8);
    protocol->data[5] = (uint8_t)(card >>= 8);
    protocol->data[4] = (uint8_t)(card >>= 8);
}

static bool protocol_gallagher_can_be_decoded(ProtocolGallagher* protocol) {
    // check 16 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 0, 16) != 0b0111111111101010) return false;

    // check next 16 bits preamble
    if(bit_lib_get_bits_16(protocol->encoded_data, 96, 16) != 0b0111111111101010) return false;

    uint8_t checksum_arr[8] = {0};
    for(int i = 0, pos = 0; i < 8; i++) {
        // Following the preamble, every 9th bit is a checksum-bit for the preceding byte
        pos = 16 + (9 * i);
        checksum_arr[i] = bit_lib_get_bits(protocol->encoded_data, pos, 8);
    }
    uint8_t crc = bit_lib_get_bits(protocol->encoded_data, 16 + (9 * 8), 8);
    uint8_t calc_crc = bit_lib_crc8(checksum_arr, 8, 0x7, 0x2c, false, false, 0x00);

    // crc
    if(crc != calc_crc) return false;

    return true;
}

void protocol_gallagher_decoder_start(ProtocolGallagher* protocol) {
    memset(protocol->encoded_data, 0, GALLAGHER_ENCODED_BYTE_FULL_SIZE);
    manchester_advance(
        protocol->decoder_manchester_state,
        ManchesterEventReset,
        &protocol->decoder_manchester_state,
        NULL);
};

bool protocol_gallagher_decoder_feed(ProtocolGallagher* protocol, bool level, uint32_t duration) {
    bool result = false;

    ManchesterEvent event = ManchesterEventReset;

    if(duration > GALLAGHER_READ_SHORT_TIME_LOW && duration < GALLAGHER_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > GALLAGHER_READ_LONG_TIME_LOW && duration < GALLAGHER_READ_LONG_TIME_HIGH) {
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
            bit_lib_push_bit(protocol->encoded_data, GALLAGHER_ENCODED_BYTE_FULL_SIZE, data);

            if(protocol_gallagher_can_be_decoded(protocol)) {
                protocol_gallagher_decode(protocol);
                result = true;
            }
        }
    }

    return result;
};

bool protocol_gallagher_encoder_start(ProtocolGallagher* protocol) {
    // Preamble
    bit_lib_set_bits(protocol->encoded_data, 0, 0b01111111, 8);
    bit_lib_set_bits(protocol->encoded_data, 8, 0b11101010, 8);

    uint8_t rc = bit_lib_get_bits(protocol->data, 0, 4);
    uint8_t il = bit_lib_get_bits(protocol->data, 4, 4);
    uint32_t fc = bit_lib_get_bits_32(protocol->data, 8, 24);
    uint32_t cn = bit_lib_get_bits_32(protocol->data, 32, 32);

    uint8_t payload[8] = {0};
    payload[0] = (cn & 0xffffff) >> 16;
    payload[1] = (fc & 0xfff) >> 4;
    payload[2] = (cn & 0x7ff) >> 3;
    payload[3] = (cn & 0x7) << 5 | (rc & 0xf) << 1;
    payload[4] = (cn & 0xffff) >> 11;
    payload[5] = (fc & 0xffff) >> 12;
    payload[6] = 0;
    payload[7] = (fc & 0xf) << 4 | (il & 0xf);

    // Gallagher scramble
    protocol_gallagher_scramble(payload, 8);

    for(int i = 0; i < 8; i++) {
        // data byte
        bit_lib_set_bits(protocol->encoded_data, 16 + (i * 9), payload[i], 8);

        // every byte is followed by a bit which is the inverse of the last bit
        bit_lib_set_bit(protocol->encoded_data, 16 + (i * 9) + 8, !(payload[i] & 0x1));
    }

    // checksum
    uint8_t crc = bit_lib_crc8(payload, 8, 0x7, 0x2c, false, false, 0x00);
    bit_lib_set_bits(protocol->encoded_data, 16 + (9 * 8), crc, 8);

    return true;
};

LevelDuration protocol_gallagher_encoder_yield(ProtocolGallagher* protocol) {
    bool level = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_data_index);
    uint32_t duration = GALLAGHER_CLOCK_PER_BIT / 2;

    if(protocol->encoded_polarity) {
        protocol->encoded_polarity = false;
    } else {
        level = !level;

        protocol->encoded_polarity = true;
        bit_lib_increment_index(protocol->encoded_data_index, GALLAGHER_ENCODED_BIT_SIZE);
    }

    return level_duration_make(level, duration);
};

bool protocol_gallagher_write_data(ProtocolGallagher* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_gallagher_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_32 |
             (3 << LFRFID_T5577_MAXBLOCK_SHIFT));
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
};

void protocol_gallagher_render_data(ProtocolGallagher* protocol, string_t result) {
    UNUSED(protocol);
    uint8_t rc = bit_lib_get_bits(protocol->data, 0, 4);
    uint8_t il = bit_lib_get_bits(protocol->data, 4, 4);
    uint32_t fc = bit_lib_get_bits_32(protocol->data, 8, 24);
    uint32_t card_id = bit_lib_get_bits_32(protocol->data, 32, 32);

    string_cat_printf(result, "Region: %u, Issue Level: %u\r\n", rc, il);
    string_cat_printf(result, "FC: %u, C: %lu\r\n", fc, card_id);
};

const ProtocolBase protocol_gallagher = {
    .name = "Gallagher",
    .manufacturer = "Gallagher",
    .data_size = GALLAGHER_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_gallagher_alloc,
    .free = (ProtocolFree)protocol_gallagher_free,
    .get_data = (ProtocolGetData)protocol_gallagher_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_gallagher_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_gallagher_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_gallagher_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_gallagher_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_gallagher_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_gallagher_render_data,
    .write_data = (ProtocolWriteData)protocol_gallagher_write_data,
};