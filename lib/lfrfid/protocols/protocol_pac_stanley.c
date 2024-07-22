#include <furi.h>
#include <math.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/hex.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"

#define PAC_STANLEY_ENCODED_BIT_SIZE   (128)
#define PAC_STANLEY_ENCODED_BYTE_SIZE  (((PAC_STANLEY_ENCODED_BIT_SIZE) / 8))
#define PAC_STANLEY_PREAMBLE_BIT_SIZE  (8)
#define PAC_STANLEY_PREAMBLE_BYTE_SIZE (1)
#define PAC_STANLEY_ENCODED_BYTE_FULL_SIZE \
    (PAC_STANLEY_ENCODED_BYTE_SIZE + PAC_STANLEY_PREAMBLE_BYTE_SIZE)
#define PAC_STANLEY_BYTE_LENGTH      (10) // start bit, 7 data bits, parity bit, stop bit
#define PAC_STANLEY_DATA_START_INDEX (8 + (3 * PAC_STANLEY_BYTE_LENGTH) + 1)

#define PAC_STANLEY_DECODED_DATA_SIZE (4)
#define PAC_STANLEY_ENCODED_DATA_SIZE (sizeof(ProtocolPACStanley))

#define PAC_STANLEY_CLOCKS_IN_US (32)
#define PAC_STANLEY_CYCLE_LENGTH (256)
#define PAC_STANLEY_MIN_TIME     (60)
#define PAC_STANLEY_MAX_TIME     (4000)

typedef struct {
    bool inverted;
    bool got_preamble;
    size_t encoded_index;
    uint8_t encoded_data[PAC_STANLEY_ENCODED_BYTE_FULL_SIZE];
    uint8_t data[PAC_STANLEY_DECODED_DATA_SIZE];
} ProtocolPACStanley;

ProtocolPACStanley* protocol_pac_stanley_alloc(void) {
    ProtocolPACStanley* protocol = malloc(sizeof(ProtocolPACStanley));
    return (void*)protocol;
}

void protocol_pac_stanley_free(ProtocolPACStanley* protocol) {
    free(protocol);
}

uint8_t* protocol_pac_stanley_get_data(ProtocolPACStanley* protocol) {
    return protocol->data;
}

static void protocol_pac_stanley_decode(ProtocolPACStanley* protocol) {
    uint8_t asciiCardId[8];
    for(size_t idx = 0; idx < 8; idx++) {
        uint8_t byte = bit_lib_reverse_8_fast(bit_lib_get_bits(
            protocol->encoded_data,
            PAC_STANLEY_DATA_START_INDEX + (PAC_STANLEY_BYTE_LENGTH * idx),
            8));
        asciiCardId[idx] = byte & 0x7F; // discard the parity bit
    }

    hex_chars_to_uint8((char*)asciiCardId, protocol->data);
}

static bool protocol_pac_stanley_can_be_decoded(ProtocolPACStanley* protocol) {
    // Check preamble
    if(bit_lib_get_bits(protocol->encoded_data, 0, 8) != 0b11111111) return false;
    if(bit_lib_get_bit(protocol->encoded_data, 8) != 0) return false;
    if(bit_lib_get_bit(protocol->encoded_data, 9) != 0) return false;
    if(bit_lib_get_bit(protocol->encoded_data, 10) != 1) return false;
    if(bit_lib_get_bits(protocol->encoded_data, 11, 8) != 0b00000010) return false;

    // Check next preamble
    if(bit_lib_get_bits(protocol->encoded_data, 128, 8) != 0b11111111) return false;

    // Checksum
    uint8_t checksum = 0;
    uint8_t stripped_byte;
    for(size_t idx = 0; idx < 9; idx++) {
        uint8_t byte = bit_lib_reverse_8_fast(bit_lib_get_bits(
            protocol->encoded_data,
            PAC_STANLEY_DATA_START_INDEX + (PAC_STANLEY_BYTE_LENGTH * idx),
            8));
        stripped_byte = byte & 0x7F; // discard the parity bit
        if(bit_lib_test_parity_32(stripped_byte, BitLibParityOdd) != (byte & 0x80) >> 7) {
            return false;
        }
        if(idx < 8) checksum ^= stripped_byte;
    }
    if(stripped_byte != checksum) return false;
    return true;
}

void protocol_pac_stanley_decoder_start(ProtocolPACStanley* protocol) {
    memset(protocol->data, 0, PAC_STANLEY_DECODED_DATA_SIZE);
    protocol->inverted = false;
    protocol->got_preamble = false;
}

bool protocol_pac_stanley_decoder_feed(ProtocolPACStanley* protocol, bool level, uint32_t duration) {
    bool pushed = false;

    if(duration > PAC_STANLEY_MAX_TIME) return false;

    uint8_t pulses = (uint8_t)roundf((float)duration / PAC_STANLEY_CYCLE_LENGTH);

    // Handle last stopbit & preamble (1 sb, 8 bit preamble)
    if(pulses >= 9 && !protocol->got_preamble) {
        pulses = 8;
        protocol->got_preamble = true;
        protocol->inverted = !level;
    } else if(pulses >= 9 && protocol->got_preamble) {
        protocol->got_preamble = false;
    } else if(pulses == 0 && duration > PAC_STANLEY_MIN_TIME) {
        pulses = 1;
    }

    if(pulses) {
        for(uint8_t i = 0; i < pulses; i++) {
            bit_lib_push_bit(
                protocol->encoded_data,
                PAC_STANLEY_ENCODED_BYTE_FULL_SIZE,
                level ^ protocol->inverted);
        }
        pushed = true;
    }

    if(pushed && protocol_pac_stanley_can_be_decoded(protocol)) {
        protocol_pac_stanley_decode(protocol);
        return true;
    }

    return false;
}

bool protocol_pac_stanley_encoder_start(ProtocolPACStanley* protocol) {
    memset(protocol->encoded_data, 0, sizeof(protocol->encoded_data));

    uint8_t idbytes[10];
    idbytes[0] = '2';
    idbytes[1] = '0';

    uint8_to_hex_chars(protocol->data, &idbytes[2], 8);

    // insert start and stop bits
    for(size_t i = 0; i < 16; i++)
        protocol->encoded_data[i] = 0x40 >> ((i + 3) % 5 * 2);

    protocol->encoded_data[0] = 0xFF; // mark + stop
    protocol->encoded_data[1] = 0x20; // start + reflect8(STX)

    uint8_t checksum = 0;
    for(size_t i = 2; i < 13; i++) {
        uint8_t shift = 7 - (i + 3) % 4 * 2;
        uint8_t index = i + (i - 1) / 4;

        uint16_t pattern;
        if(i < 12) {
            pattern = bit_lib_reverse_8_fast(idbytes[i - 2]);
            pattern |= bit_lib_test_parity_32(pattern, BitLibParityOdd);
            if(i > 3) checksum ^= idbytes[i - 2];
        } else {
            pattern = (bit_lib_reverse_8_fast(checksum) & 0xFE) |
                      (bit_lib_test_parity_32(checksum, BitLibParityOdd));
        }
        pattern <<= shift;

        protocol->encoded_data[index] |= pattern >> 8 & 0xFF;
        protocol->encoded_data[index + 1] |= pattern & 0xFF;
    }

    protocol->encoded_index = 0;
    return true;
}

LevelDuration protocol_pac_stanley_encoder_yield(ProtocolPACStanley* protocol) {
    uint16_t length = PAC_STANLEY_CLOCKS_IN_US;
    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_index);
    bit_lib_increment_index(protocol->encoded_index, PAC_STANLEY_ENCODED_BIT_SIZE);
    while(bit_lib_get_bit(protocol->encoded_data, protocol->encoded_index) == bit) {
        length += PAC_STANLEY_CLOCKS_IN_US;
        bit_lib_increment_index(protocol->encoded_index, PAC_STANLEY_ENCODED_BIT_SIZE);
    }

    return level_duration_make(bit, length);
}

bool protocol_pac_stanley_write_data(ProtocolPACStanley* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_pac_stanley_encoder_start(protocol);
    protocol_pac_stanley_decode(protocol);

    protocol_pac_stanley_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_DIRECT | LFRFID_T5577_BITRATE_RF_32 |
                                  (4 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.block[4] = bit_lib_get_bits_32(protocol->encoded_data, 96, 32);
        request->t5577.blocks_to_write = 5;
        result = true;
    }
    return result;
}

void protocol_pac_stanley_render_data(ProtocolPACStanley* protocol, FuriString* result) {
    furi_string_printf(result, "CIN: %08lX", bit_lib_get_bits_32(protocol->data, 0, 32));
}

const ProtocolBase protocol_pac_stanley = {
    .name = "PAC/Stanley",
    .manufacturer = "N/A",
    .data_size = PAC_STANLEY_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_pac_stanley_alloc,
    .free = (ProtocolFree)protocol_pac_stanley_free,
    .get_data = (ProtocolGetData)protocol_pac_stanley_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_pac_stanley_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_pac_stanley_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_pac_stanley_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_pac_stanley_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_pac_stanley_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_pac_stanley_render_data,
    .write_data = (ProtocolWriteData)protocol_pac_stanley_write_data,
};
