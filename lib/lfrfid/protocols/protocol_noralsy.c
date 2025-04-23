#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define NORALSY_CLOCK_PER_BIT (32)

#define NORALSY_ENCODED_BIT_SIZE       (96)
#define NORALSY_ENCODED_BYTE_SIZE      ((NORALSY_ENCODED_BIT_SIZE) / 8)
#define NORALSY_PREAMBLE_BIT_SIZE      (32)
#define NORALSY_PREAMBLE_BYTE_SIZE     ((NORALSY_PREAMBLE_BIT_SIZE) / 8)
#define NORALSY_ENCODED_BYTE_FULL_SIZE ((NORALSY_ENCODED_BIT_SIZE) / 8)
#define NORALSY_DECODED_DATA_SIZE      ((NORALSY_ENCODED_BIT_SIZE) / 8)

#define NORALSY_READ_SHORT_TIME  (128)
#define NORALSY_READ_LONG_TIME   (256)
#define NORALSY_READ_JITTER_TIME (60)

#define NORALSY_READ_SHORT_TIME_LOW  (NORALSY_READ_SHORT_TIME - NORALSY_READ_JITTER_TIME)
#define NORALSY_READ_SHORT_TIME_HIGH (NORALSY_READ_SHORT_TIME + NORALSY_READ_JITTER_TIME)
#define NORALSY_READ_LONG_TIME_LOW   (NORALSY_READ_LONG_TIME - NORALSY_READ_JITTER_TIME)
#define NORALSY_READ_LONG_TIME_HIGH  (NORALSY_READ_LONG_TIME + NORALSY_READ_JITTER_TIME)

#define TAG "NORALSY"

typedef struct {
    uint8_t data[NORALSY_ENCODED_BYTE_SIZE];
    uint8_t encoded_data[NORALSY_ENCODED_BYTE_SIZE];

    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolNoralsy;

ProtocolNoralsy* protocol_noralsy_alloc(void) {
    ProtocolNoralsy* protocol = malloc(sizeof(ProtocolNoralsy));
    return (void*)protocol;
}

void protocol_noralsy_free(ProtocolNoralsy* protocol) {
    free(protocol);
}

static uint8_t noralsy_chksum(uint8_t* bits, uint8_t len) {
    uint8_t sum = 0;
    for(uint8_t i = 0; i < len; i += 4)
        sum ^= bit_lib_get_bits(bits, i, 4);
    return sum & 0x0F;
}

uint8_t* protocol_noralsy_get_data(ProtocolNoralsy* protocol) {
    return protocol->data;
}

static void protocol_noralsy_decode(ProtocolNoralsy* protocol) {
    bit_lib_copy_bits(protocol->data, 0, NORALSY_ENCODED_BIT_SIZE, protocol->encoded_data, 0);
}

static bool protocol_noralsy_can_be_decoded(ProtocolNoralsy* protocol) {
    // check 12 bits preamble
    // If necessary, use 0xBB0214FF for 32 bit preamble check
    // However, it is not confirmed the 13-16 bit are static.
    if(bit_lib_get_bits_16(protocol->encoded_data, 0, 12) != 0b101110110000) return false;
    uint8_t calc1 = noralsy_chksum(&protocol->encoded_data[4], 40);
    uint8_t calc2 = noralsy_chksum(&protocol->encoded_data[0], 76);
    uint8_t chk1 = bit_lib_get_bits(protocol->encoded_data, 72, 4);
    uint8_t chk2 = bit_lib_get_bits(protocol->encoded_data, 76, 4);
    if(calc1 != chk1 || calc2 != chk2) return false;

    return true;
}

void protocol_noralsy_decoder_start(ProtocolNoralsy* protocol) {
    memset(protocol->encoded_data, 0, NORALSY_ENCODED_BYTE_FULL_SIZE);
    manchester_advance(
        protocol->decoder_manchester_state,
        ManchesterEventReset,
        &protocol->decoder_manchester_state,
        NULL);
}

bool protocol_noralsy_decoder_feed(ProtocolNoralsy* protocol, bool level, uint32_t duration) {
    bool result = false;

    ManchesterEvent event = ManchesterEventReset;

    if(duration > NORALSY_READ_SHORT_TIME_LOW && duration < NORALSY_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > NORALSY_READ_LONG_TIME_LOW && duration < NORALSY_READ_LONG_TIME_HIGH) {
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
            bit_lib_push_bit(protocol->encoded_data, NORALSY_ENCODED_BYTE_FULL_SIZE, data);

            if(protocol_noralsy_can_be_decoded(protocol)) {
                protocol_noralsy_decode(protocol);
                result = true;
            }
        }
    }

    return result;
}

bool protocol_noralsy_encoder_start(ProtocolNoralsy* protocol) {
    bit_lib_copy_bits(protocol->encoded_data, 0, NORALSY_ENCODED_BIT_SIZE, protocol->data, 0);

    return true;
}

LevelDuration protocol_noralsy_encoder_yield(ProtocolNoralsy* protocol) {
    bool level = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_data_index);
    uint32_t duration = NORALSY_CLOCK_PER_BIT / 2;

    if(protocol->encoded_polarity) {
        protocol->encoded_polarity = false;
    } else {
        level = !level;

        protocol->encoded_polarity = true;
        bit_lib_increment_index(protocol->encoded_data_index, NORALSY_ENCODED_BIT_SIZE);
    }

    return level_duration_make(level, duration);
}

bool protocol_noralsy_write_data(ProtocolNoralsy* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_noralsy_encoder_start(protocol);
    protocol_noralsy_decode(protocol);

    protocol_noralsy_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_32 |
             (3 << LFRFID_T5577_MAXBLOCK_SHIFT) | LFRFID_T5577_ST_TERMINATOR);
        // In fact, base on the current two dump samples from Iceman server,
        // Noralsy are usually T5577s with config = 0x00088C6A
        // But the `C` and `A` are not explainable by the ATA5577C datasheet
        // and they don't affect reading whatsoever.
        // So we are mimicing Proxmark's solution here. Leave those nibbles as zero.
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.blocks_to_write = 4;
        result = true;
    }
    return result;
}

static void protocol_noralsy_render_data_internal(ProtocolNoralsy* protocol, FuriString* result) {
    UNUSED(protocol);
    uint32_t raw2 = bit_lib_get_bits_32(protocol->data, 32, 32);
    uint32_t raw3 = bit_lib_get_bits_32(protocol->data, 64, 32);
    uint32_t cardid = ((raw2 & 0xFFF00000) >> 20) << 16;
    cardid |= (raw2 & 0xFF) << 8;
    cardid |= ((raw3 & 0xFF000000) >> 24);

    uint8_t year = (raw2 & 0x000ff000) >> 12;
    bool tag_is_gen_z = (year > 0x60);
    furi_string_printf(
        result,
        "Card ID: %07lx\n"
        "Year: %s%02x",
        cardid,
        tag_is_gen_z ? "19" : "20",
        year);
}

void protocol_noralsy_render_data(ProtocolNoralsy* protocol, FuriString* result) {
    protocol_noralsy_render_data_internal(protocol, result);
}

void protocol_noralsy_render_brief_data(ProtocolNoralsy* protocol, FuriString* result) {
    protocol_noralsy_render_data_internal(protocol, result);
}

const ProtocolBase protocol_noralsy = {
    .name = "Noralsy",
    .manufacturer = "Noralsy",
    .data_size = NORALSY_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_noralsy_alloc,
    .free = (ProtocolFree)protocol_noralsy_free,
    .get_data = (ProtocolGetData)protocol_noralsy_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_noralsy_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_noralsy_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_noralsy_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_noralsy_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_noralsy_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_noralsy_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_noralsy_write_data,
};
