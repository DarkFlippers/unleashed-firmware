// The timing parameters and data structure used in this file
// are based on the knowledge found in Proxmark3's firmware:
// https://github.com/RfidResearchGroup/proxmark3/blob/1c52152d30f7744c0336633317ea6640dbcdc796/client/src/cmdlfsecurakey.c
// PM3's repo has mentioned the existence of non-26-or-32-bit formats.
// Those are not supported here for preventing false positives.
#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/hex.h>
#include <bit_lib/bit_lib.h>
#include "lfrfid_protocols.h"
#include <toolbox/manchester_decoder.h>

#define TAG "SECURAKEY"

#define SECURAKEY_RKKT_ENCODED_FULL_SIZE_BITS (96)
#define SECURAKEY_RKKT_ENCODED_FULL_SIZE_BYTE (12)

#define SECURAKEY_RKKTH_ENCODED_FULL_SIZE_BITS (64)
#define SECURAKEY_RKKTH_ENCODED_FULL_SIZE_BYTE (8)

#define SECURAKEY_DECODED_DATA_SIZE_BITS  (48)
// RKKT: 16-bit for facility code/number, 16-bit for card number, 16-bit for two checksum
// RKKTH: 16-bit zero padding, 32-bit card number
#define SECURAKEY_DECODED_DATA_SIZE_BYTES (SECURAKEY_DECODED_DATA_SIZE_BITS / 8)
#define LFRFID_FREQUENCY                  (125000)
#define SECURAKEY_CLOCK_PER_BIT           (40) // RF/40
#define SECURAKEY_READ_LONG_TIME \
    (1000000 / (LFRFID_FREQUENCY / SECURAKEY_CLOCK_PER_BIT)) // 1000000 micro sec / sec
#define SECURAKEY_READ_SHORT_TIME  (SECURAKEY_READ_LONG_TIME / 2)
#define SECURAKEY_READ_JITTER_TIME (SECURAKEY_READ_SHORT_TIME * 40 / 100) // 40% jitter tolerance
#define SECURAKEY_READ_SHORT_TIME_LOW \
    (SECURAKEY_READ_SHORT_TIME -      \
     SECURAKEY_READ_JITTER_TIME) // these are used for manchester decoding
#define SECURAKEY_READ_SHORT_TIME_HIGH (SECURAKEY_READ_SHORT_TIME + SECURAKEY_READ_JITTER_TIME)
#define SECURAKEY_READ_LONG_TIME_LOW   (SECURAKEY_READ_LONG_TIME - SECURAKEY_READ_JITTER_TIME)
#define SECURAKEY_READ_LONG_TIME_HIGH  (SECURAKEY_READ_LONG_TIME + SECURAKEY_READ_JITTER_TIME)

typedef struct {
    uint8_t data[SECURAKEY_DECODED_DATA_SIZE_BYTES];
    uint8_t RKKT_encoded_data[SECURAKEY_RKKT_ENCODED_FULL_SIZE_BYTE];
    uint8_t RKKTH_encoded_data[SECURAKEY_RKKTH_ENCODED_FULL_SIZE_BYTE];
    uint8_t encoded_data_index;
    bool encoded_polarity;
    ManchesterState decoder_manchester_state;
    uint8_t bit_format;
} ProtocolSecurakey;

ProtocolSecurakey* protocol_securakey_alloc(void) {
    ProtocolSecurakey* protocol = malloc(sizeof(ProtocolSecurakey));
    return (void*)protocol;
}

void protocol_securakey_free(ProtocolSecurakey* protocol) {
    free(protocol);
}

uint8_t* protocol_securakey_get_data(ProtocolSecurakey* protocol) {
    return protocol->data;
}

static bool protocol_securakey_can_be_decoded(ProtocolSecurakey* protocol) {
    // check 19 bits preamble + format flag
    if(bit_lib_get_bits_32(protocol->RKKT_encoded_data, 0, 19) == 0b0111111111000000000) {
        protocol->bit_format = 0;
        return true;
    } else if(bit_lib_get_bits_32(protocol->RKKT_encoded_data, 0, 19) == 0b0111111111001011010) {
        protocol->bit_format = 26;
        return true;
    } else if(bit_lib_get_bits_32(protocol->RKKT_encoded_data, 0, 19) == 0b0111111111001100000) {
        protocol->bit_format = 32;
        return true;
    } else {
        return false;
    }
}

static void protocol_securakey_decode(ProtocolSecurakey* protocol) {
    memset(protocol->data, 0, SECURAKEY_DECODED_DATA_SIZE_BYTES);
    // RKKT_encoded_data looks like this (citation: pm3 repo):
    // 26-bit format (1-bit even parity bit,  8-bit facility number, 16-bit card number, 1-bit odd parity bit)
    // preamble     ??bitlen   reserved        EPf   fffffffc   cccccccc   cccccccOP  CS?        CS2?
    // 0111111111 0 01011010 0 00000000 0 00000010 0 00110110 0 00111110 0 01100010 0 00001111 0 01100000 0 00000000 0 0000

    // 32-bit format (1-bit even parity bit, 14-bit facility number, 16-bit card number, 1-bit odd parity bit)
    // preamble     ??bitlen   reserved  EPfffffff   fffffffc   cccccccc   cccccccOP  CS?        CS2?
    // 0111111111 0 01100000 0 00000000 0 10000100 0 11001010 0 01011011 0 01010110 0 00010110 0 11100000 0 00000000 0 0000

    // RKKTH-02 encoded data sometimes look like this
    // plaintext format (preamble and 32-bit? card number)
    // preamble     unknown    unknown    cccccccc   cccccccc   cccccccc   cccccccc
    // 0          1            2           3           4           5           6
    // 0123456789 0 12345678 9 01234567 8 90123456 7 89012345 6 78901234 5 67890123
    // 0111111111 0 00000000 0 00000000 0 00000000 0 00011101 0 00000100 0 01001010

    if(bit_lib_get_bits(protocol->RKKT_encoded_data, 13, 6) == 0) {
        FURI_LOG_D(TAG, "Plaintext RKKTH detected");
        protocol->bit_format = 0;
        // get card number (c)
        bit_lib_copy_bits(protocol->data, 16, 8, protocol->RKKT_encoded_data, 29);
        // skip spacers (0s)
        bit_lib_copy_bits(protocol->data, 24, 8, protocol->RKKT_encoded_data, 38);
        bit_lib_copy_bits(protocol->data, 32, 8, protocol->RKKT_encoded_data, 47);
        bit_lib_copy_bits(protocol->data, 40, 8, protocol->RKKT_encoded_data, 56);
    } else {
        if(bit_lib_get_bits(protocol->RKKT_encoded_data, 13, 6) == 26) {
            FURI_LOG_D(TAG, "26-bit RKKT detected");
            protocol->bit_format = 26;
            // left two 0 paddings in the beginning for easier parsing (00011010 = 011010)
            // get facility number (f)
            bit_lib_copy_bits(protocol->data, 8, 1, protocol->RKKT_encoded_data, 36);
            // have to skip one spacer
            bit_lib_copy_bits(protocol->data, 9, 7, protocol->RKKT_encoded_data, 38);
        } else if(bit_lib_get_bits(protocol->RKKT_encoded_data, 13, 6) == 32) {
            FURI_LOG_D(TAG, "32-bit RKKT detected");
            protocol->bit_format = 32;
            // same two 0 paddings here, otherwise should be bit_lib_copy_bits(protocol->data, 8, 7, protocol->RKKT_encoded_data, 30);
            bit_lib_copy_bits(protocol->data, 2, 7, protocol->RKKT_encoded_data, 30);
            // have to skip one spacer
            bit_lib_copy_bits(protocol->data, 9, 7, protocol->RKKT_encoded_data, 38);
        }
        // get card number (c)
        bit_lib_copy_bits(protocol->data, 16, 1, protocol->RKKT_encoded_data, 45);
        // same skips here
        bit_lib_copy_bits(protocol->data, 17, 8, protocol->RKKT_encoded_data, 47);
        bit_lib_copy_bits(protocol->data, 25, 7, protocol->RKKT_encoded_data, 56);

        // unsure about CS yet, might as well just save it
        // CS1
        bit_lib_copy_bits(protocol->data, 32, 8, protocol->RKKT_encoded_data, 65);
        // CS2
        bit_lib_copy_bits(protocol->data, 40, 8, protocol->RKKT_encoded_data, 74);
    }

    // (decoded) data looks like this (pp are zero paddings):
    // 26-bit format (1-bit EP,  8-bit facility number, 16-bit card number, 1-bit OP)
    // pppppppp ffffffff cccccccc cccccccc CS1      CS2
    // 00000000 00011011 00011111 00110001 00001111 01100000

    // 32-bit format (1-bit EP, 14-bit facility number, 16-bit card number, 1-bit OP)
    // ppffffff ffffffff cccccccc cccccccc CS1      CS2
    // 00000010 01100101 00101101 10101011 00010110 11100000

    // plaintext format (preamble and 32-bit? card number)
    // pppppppp pppppppp cccccccc cccccccc cccccccc cccccccc
    // 00000000 00000000 00101011 00011101 00000100 01001010
}

void protocol_securakey_decoder_start(ProtocolSecurakey* protocol) {
    // always takes in encoded data as RKKT for simplicity
    // this part is feeding decoder which will delineate the format anyway
    memset(protocol->RKKT_encoded_data, 0, SECURAKEY_RKKT_ENCODED_FULL_SIZE_BYTE);
    manchester_advance(
        protocol->decoder_manchester_state,
        ManchesterEventReset,
        &protocol->decoder_manchester_state,
        NULL);
}

bool protocol_securakey_decoder_feed(ProtocolSecurakey* protocol, bool level, uint32_t duration) {
    bool result = false;
    // this is where we do manchester demodulation on already ASK-demoded data
    ManchesterEvent event = ManchesterEventReset;
    if(duration > SECURAKEY_READ_SHORT_TIME_LOW && duration < SECURAKEY_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > SECURAKEY_READ_LONG_TIME_LOW && duration < SECURAKEY_READ_LONG_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventLongHigh;
        } else {
            event = ManchesterEventLongLow;
        }
    }
    // append a new bit to the encoded bit stream
    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok = manchester_advance(
            protocol->decoder_manchester_state, event, &protocol->decoder_manchester_state, &data);
        if(data_ok) {
            bit_lib_push_bit(
                protocol->RKKT_encoded_data, SECURAKEY_RKKT_ENCODED_FULL_SIZE_BYTE, data);
            if(protocol_securakey_can_be_decoded(protocol)) {
                protocol_securakey_decode(protocol);
                result = true;
            }
        }
    }
    return result;
}

void protocol_securakey_render_data(ProtocolSecurakey* protocol, FuriString* result) {
    if(bit_lib_get_bits_16(protocol->data, 0, 16) == 0) {
        protocol->bit_format = 0;
        furi_string_printf(
            result,
            "RKKTH Plaintext format\nCard number: %llu",
            bit_lib_get_bits_64(protocol->data, 0, 48));
    } else {
        if(bit_lib_get_bits(protocol->data, 0, 8) == 0) {
            protocol->bit_format = 26;
        } else {
            protocol->bit_format = 32;
        }
        furi_string_printf(
            result,
            "RKKT %u-bit format\nFacility code: %u\nCard number: %u",
            protocol->bit_format,
            bit_lib_get_bits_16(protocol->data, 0, 16),
            bit_lib_get_bits_16(protocol->data, 16, 16));
    }
}

bool protocol_securakey_encoder_start(ProtocolSecurakey* protocol) {
    // set all of our encoded_data bits to zeros.
    memset(protocol->RKKTH_encoded_data, 0, SECURAKEY_RKKTH_ENCODED_FULL_SIZE_BYTE);
    memset(protocol->RKKT_encoded_data, 0, SECURAKEY_RKKT_ENCODED_FULL_SIZE_BYTE);
    if(bit_lib_get_bits_16(protocol->data, 0, 16) == 0) {
        // write the preamble to the beginning of the RKKT_encoded_data
        bit_lib_set_bits(protocol->RKKTH_encoded_data, 0, 0b01111111, 8);
        bit_lib_set_bits(protocol->RKKTH_encoded_data, 8, 0b110, 3); //preamble cont.
        // write card number (c)
        bit_lib_copy_bits(protocol->RKKTH_encoded_data, 29, 8, protocol->data, 16);
        // skip spacers (they are zero already by memset)
        bit_lib_copy_bits(protocol->RKKTH_encoded_data, 38, 8, protocol->data, 24);
        bit_lib_copy_bits(protocol->RKKTH_encoded_data, 47, 8, protocol->data, 32);
        bit_lib_copy_bits(protocol->RKKTH_encoded_data, 56, 8, protocol->data, 40);
    } else {
        // write the preamble to the beginning of the RKKT_encoded_data
        bit_lib_set_bits(protocol->RKKT_encoded_data, 0, 0b01111111, 8);
        bit_lib_set_bits(protocol->RKKT_encoded_data, 8, 0b11001, 5); //preamble cont.
        if(bit_lib_get_bits(protocol->data, 0, 8) == 0) {
            protocol->bit_format = 26;
            // set bit length
            bit_lib_set_bits(protocol->RKKT_encoded_data, 13, protocol->bit_format, 6);
            // set even parity & odd parity
            if(!bit_lib_test_parity(protocol->data, 8, 12, BitLibParityOdd, 12)) {
                bit_lib_set_bit(protocol->RKKT_encoded_data, 35, 1);
            }
            if(bit_lib_test_parity(protocol->data, 20, 12, BitLibParityOdd, 12)) {
                bit_lib_set_bit(protocol->RKKT_encoded_data, 63, 1);
            }
            // write facility number (f)
            bit_lib_copy_bits(protocol->RKKT_encoded_data, 36, 1, protocol->data, 8);
            // have to skip one spacer
            bit_lib_copy_bits(protocol->RKKT_encoded_data, 38, 7, protocol->data, 9);
        } else {
            protocol->bit_format = 32;
            // set bit length
            bit_lib_set_bits(protocol->RKKT_encoded_data, 13, protocol->bit_format, 6);
            // set EP & OP
            if(!bit_lib_test_parity(protocol->data, 2, 15, BitLibParityOdd, 15)) {
                bit_lib_set_bit(protocol->RKKT_encoded_data, 29, 1);
            }
            if(bit_lib_test_parity(protocol->data, 17, 15, BitLibParityOdd, 15)) {
                bit_lib_set_bit(protocol->RKKT_encoded_data, 63, 1);
            }
            // write facility number (f)
            bit_lib_copy_bits(protocol->RKKT_encoded_data, 30, 7, protocol->data, 2);
            // have to skip one spacer
            bit_lib_copy_bits(protocol->RKKT_encoded_data, 38, 7, protocol->data, 3);
        }

        // write card number (c)
        bit_lib_copy_bits(protocol->RKKT_encoded_data, 45, 1, protocol->data, 16);
        // same skips here
        bit_lib_copy_bits(protocol->RKKT_encoded_data, 47, 8, protocol->data, 17);
        bit_lib_copy_bits(protocol->RKKT_encoded_data, 56, 7, protocol->data, 25);

        // unsure about CS yet might as well just copy it from saved
        // CS1
        bit_lib_copy_bits(protocol->RKKT_encoded_data, 65, 8, protocol->data, 32);
        // CS2
        bit_lib_copy_bits(protocol->RKKT_encoded_data, 74, 8, protocol->data, 40);
    }
    // for sending we start at bit 0.
    protocol->encoded_data_index = 0;
    protocol->encoded_polarity = true;
    return true;
}

LevelDuration protocol_securakey_encoder_yield(ProtocolSecurakey* protocol) {
    if(bit_lib_get_bits_16(protocol->data, 0, 16) == 0) {
        bool level = bit_lib_get_bit(protocol->RKKTH_encoded_data, protocol->encoded_data_index);
        uint32_t duration = SECURAKEY_CLOCK_PER_BIT / 2;
        if(protocol->encoded_polarity) {
            protocol->encoded_polarity = false;
        } else {
            level = !level;
            protocol->encoded_polarity = true;
            bit_lib_increment_index(
                protocol->encoded_data_index, SECURAKEY_RKKTH_ENCODED_FULL_SIZE_BITS);
        }
        return level_duration_make(level, duration);
    } else {
        bool level = bit_lib_get_bit(protocol->RKKT_encoded_data, protocol->encoded_data_index);
        uint32_t duration = SECURAKEY_CLOCK_PER_BIT / 2;
        if(protocol->encoded_polarity) {
            protocol->encoded_polarity = false;
        } else {
            level = !level;
            protocol->encoded_polarity = true;
            bit_lib_increment_index(
                protocol->encoded_data_index, SECURAKEY_RKKT_ENCODED_FULL_SIZE_BITS);
        }
        return level_duration_make(level, duration);
    }
}

bool protocol_securakey_write_data(ProtocolSecurakey* protocol, void* data) {
    protocol_securakey_encoder_start(protocol);
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;
    // Write T5577
    if(bit_lib_get_bits_16(protocol->data, 0, 16) == 0) {
        if(request->write_type == LFRFIDWriteTypeT5577) {
            request->t5577.block[0] =
                (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_40 |
                 (2
                  << LFRFID_T5577_MAXBLOCK_SHIFT)); // we only need 2 32-bit blocks for our 64-bit encoded data
            request->t5577.block[1] = bit_lib_get_bits_32(protocol->RKKTH_encoded_data, 0, 32);
            request->t5577.block[2] = bit_lib_get_bits_32(protocol->RKKTH_encoded_data, 32, 32);
            request->t5577.blocks_to_write = 3;
            result = true;
        }
    } else {
        if(request->write_type == LFRFIDWriteTypeT5577) {
            request->t5577.block[0] =
                (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_40 |
                 (3
                  << LFRFID_T5577_MAXBLOCK_SHIFT)); // we only need 3 32-bit blocks for our 96-bit encoded data
            request->t5577.block[1] = bit_lib_get_bits_32(protocol->RKKT_encoded_data, 0, 32);
            request->t5577.block[2] = bit_lib_get_bits_32(protocol->RKKT_encoded_data, 32, 32);
            request->t5577.block[3] = bit_lib_get_bits_32(protocol->RKKT_encoded_data, 64, 32);
            request->t5577.blocks_to_write = 4;
            result = true;
        }
    }
    return result;
}

const ProtocolBase protocol_securakey = {
    .name = "Radio Key",
    .manufacturer = "Securakey",
    .data_size = SECURAKEY_DECODED_DATA_SIZE_BYTES,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_securakey_alloc,
    .free = (ProtocolFree)protocol_securakey_free,
    .get_data = (ProtocolGetData)protocol_securakey_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_securakey_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_securakey_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_securakey_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_securakey_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_securakey_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_securakey_render_data,
    .write_data = (ProtocolWriteData)protocol_securakey_write_data,
};
