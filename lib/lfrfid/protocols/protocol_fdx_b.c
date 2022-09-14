#include <furi.h>
#include "toolbox/level_duration.h"
#include "protocol_fdx_b.h"
#include <toolbox/manchester_decoder.h>
#include <lfrfid/tools/bit_lib.h>
#include "lfrfid_protocols.h"

#define FDX_B_ENCODED_BIT_SIZE (128)
#define FDX_B_ENCODED_BYTE_SIZE (((FDX_B_ENCODED_BIT_SIZE) / 8))
#define FDX_B_PREAMBLE_BIT_SIZE (11)
#define FDX_B_PREAMBLE_BYTE_SIZE (2)
#define FDX_B_ENCODED_BYTE_FULL_SIZE (FDX_B_ENCODED_BYTE_SIZE + FDX_B_PREAMBLE_BYTE_SIZE)

#define FDXB_DECODED_DATA_SIZE (11)

#define FDX_B_SHORT_TIME (128)
#define FDX_B_LONG_TIME (256)
#define FDX_B_JITTER_TIME (60)

#define FDX_B_SHORT_TIME_LOW (FDX_B_SHORT_TIME - FDX_B_JITTER_TIME)
#define FDX_B_SHORT_TIME_HIGH (FDX_B_SHORT_TIME + FDX_B_JITTER_TIME)
#define FDX_B_LONG_TIME_LOW (FDX_B_LONG_TIME - FDX_B_JITTER_TIME)
#define FDX_B_LONG_TIME_HIGH (FDX_B_LONG_TIME + FDX_B_JITTER_TIME)

typedef struct {
    bool last_short;
    bool last_level;
    size_t encoded_index;
    uint8_t encoded_data[FDX_B_ENCODED_BYTE_FULL_SIZE];
    uint8_t data[FDXB_DECODED_DATA_SIZE];
} ProtocolFDXB;

ProtocolFDXB* protocol_fdx_b_alloc(void) {
    ProtocolFDXB* protocol = malloc(sizeof(ProtocolFDXB));
    return protocol;
};

void protocol_fdx_b_free(ProtocolFDXB* protocol) {
    free(protocol);
};

uint8_t* protocol_fdx_b_get_data(ProtocolFDXB* proto) {
    return proto->data;
};

void protocol_fdx_b_decoder_start(ProtocolFDXB* protocol) {
    memset(protocol->encoded_data, 0, FDX_B_ENCODED_BYTE_FULL_SIZE);
    protocol->last_short = false;
};

static bool protocol_fdx_b_can_be_decoded(ProtocolFDXB* protocol) {
    bool result = false;

    /*
    msb		lsb
    0   10000000000	  Header pattern. 11 bits.
    11    1nnnnnnnn	
    20    1nnnnnnnn	  38 bit (12 digit) National code.
    29    1nnnnnnnn	  eg. 000000001008 (decimal).
    38    1nnnnnnnn	
    47    1nnnnnncc	  10 bit (3 digit) Country code.
    56    1cccccccc	  eg. 999 (decimal).
    65    1s-------	  1 bit data block status flag.
    74    1-------a	  1 bit animal application indicator.
    83    1xxxxxxxx	  16 bit checksum.
    92    1xxxxxxxx	
    101   1eeeeeeee	  24 bits of extra data if present.
    110   1eeeeeeee	  eg. $123456.
    119   1eeeeeeee	
    */

    do {
        // check 11 bits preamble
        if(bit_lib_get_bits_16(protocol->encoded_data, 0, 11) != 0b10000000000) break;
        // check next 11 bits preamble
        if(bit_lib_get_bits_16(protocol->encoded_data, 128, 11) != 0b10000000000) break;
        // check control bits
        if(!bit_lib_test_parity(protocol->encoded_data, 3, 13 * 9, BitLibParityAlways1, 9)) break;

        // compute checksum
        uint8_t crc_data[8];
        for(size_t i = 0; i < 8; i++) {
            bit_lib_copy_bits(crc_data, i * 8, 8, protocol->encoded_data, 12 + 9 * i);
        }
        uint16_t crc_res = bit_lib_crc16(crc_data, 8, 0x1021, 0x0000, false, false, 0x0000);

        // read checksum
        uint16_t crc_ex = 0;
        bit_lib_copy_bits((uint8_t*)&crc_ex, 8, 8, protocol->encoded_data, 84);
        bit_lib_copy_bits((uint8_t*)&crc_ex, 0, 8, protocol->encoded_data, 93);

        // compare checksum
        if(crc_res != crc_ex) break;

        result = true;
    } while(false);

    return result;
}

void protocol_fdx_b_decode(ProtocolFDXB* protocol) {
    // remove parity
    bit_lib_remove_bit_every_nth(protocol->encoded_data, 3, 13 * 9, 9);

    // remove header pattern
    for(size_t i = 0; i < 11; i++)
        bit_lib_push_bit(protocol->encoded_data, FDX_B_ENCODED_BYTE_FULL_SIZE, 0);

    // 0  nnnnnnnn
    // 8  nnnnnnnn	  38 bit (12 digit) National code.
    // 16 nnnnnnnn	  eg. 000000001008 (decimal).
    // 24 nnnnnnnn
    // 32 nnnnnncc	  10 bit (3 digit) Country code.
    // 40 cccccccc	  eg. 999 (decimal).
    // 48 s-------	  1 bit data block status flag.
    // 56 -------a	  1 bit animal application indicator.
    // 64 xxxxxxxx	  16 bit checksum.
    // 72 xxxxxxxx
    // 80 eeeeeeee	  24 bits of extra data if present.
    // 88 eeeeeeee	  eg. $123456.
    // 92 eeeeeeee

    // copy data without checksum
    bit_lib_copy_bits(protocol->data, 0, 64, protocol->encoded_data, 0);
    bit_lib_copy_bits(protocol->data, 64, 24, protocol->encoded_data, 80);

    // const BitLibRegion regions_encoded[] = {
    //     {'n', 0, 38},
    //     {'c', 38, 10},
    //     {'b', 48, 16},
    //     {'x', 64, 16},
    //     {'e', 80, 24},
    // };

    // bit_lib_print_regions(regions_encoded, 5, protocol->encoded_data, FDX_B_ENCODED_BIT_SIZE);

    // const BitLibRegion regions_decoded[] = {
    //     {'n', 0, 38},
    //     {'c', 38, 10},
    //     {'b', 48, 16},
    //     {'e', 64, 24},
    // };

    // bit_lib_print_regions(regions_decoded, 4, protocol->data, FDXB_DECODED_DATA_SIZE * 8);
}

bool protocol_fdx_b_decoder_feed(ProtocolFDXB* protocol, bool level, uint32_t duration) {
    bool result = false;
    UNUSED(level);

    bool pushed = false;

    // Bi-Phase Manchester decoding
    if(duration >= FDX_B_SHORT_TIME_LOW && duration <= FDX_B_SHORT_TIME_HIGH) {
        if(protocol->last_short == false) {
            protocol->last_short = true;
        } else {
            pushed = true;
            bit_lib_push_bit(protocol->encoded_data, FDX_B_ENCODED_BYTE_FULL_SIZE, false);
            protocol->last_short = false;
        }
    } else if(duration >= FDX_B_LONG_TIME_LOW && duration <= FDX_B_LONG_TIME_HIGH) {
        if(protocol->last_short == false) {
            pushed = true;
            bit_lib_push_bit(protocol->encoded_data, FDX_B_ENCODED_BYTE_FULL_SIZE, true);
        } else {
            // reset
            protocol->last_short = false;
        }
    } else {
        // reset
        protocol->last_short = false;
    }

    if(pushed && protocol_fdx_b_can_be_decoded(protocol)) {
        protocol_fdx_b_decode(protocol);
        result = true;
    }

    return result;
};

bool protocol_fdx_b_encoder_start(ProtocolFDXB* protocol) {
    memset(protocol->encoded_data, 0, FDX_B_ENCODED_BYTE_FULL_SIZE);
    bit_lib_set_bit(protocol->encoded_data, 0, 1);
    for(size_t i = 0; i < 13; i++) {
        bit_lib_set_bit(protocol->encoded_data, 11 + 9 * i, 1);
        if(i == 8 || i == 9) continue;

        if(i < 8) {
            bit_lib_copy_bits(protocol->encoded_data, 12 + 9 * i, 8, protocol->data, i * 8);
        } else {
            bit_lib_copy_bits(protocol->encoded_data, 12 + 9 * i, 8, protocol->data, (i - 2) * 8);
        }
    }

    uint16_t crc_res = bit_lib_crc16(protocol->data, 8, 0x1021, 0x0000, false, false, 0x0000);
    bit_lib_copy_bits(protocol->encoded_data, 84, 8, (uint8_t*)&crc_res, 8);
    bit_lib_copy_bits(protocol->encoded_data, 93, 8, (uint8_t*)&crc_res, 0);

    protocol->encoded_index = 0;
    protocol->last_short = false;
    protocol->last_level = false;
    return true;
};

LevelDuration protocol_fdx_b_encoder_yield(ProtocolFDXB* protocol) {
    uint32_t duration;
    protocol->last_level = !protocol->last_level;

    bool bit = bit_lib_get_bit(protocol->encoded_data, protocol->encoded_index);

    // Bi-Phase Manchester encoder
    if(bit) {
        // one long pulse for 1
        duration = FDX_B_LONG_TIME / 8;
        bit_lib_increment_index(protocol->encoded_index, FDX_B_ENCODED_BIT_SIZE);
    } else {
        // two short pulses for 0
        duration = FDX_B_SHORT_TIME / 8;
        if(protocol->last_short) {
            bit_lib_increment_index(protocol->encoded_index, FDX_B_ENCODED_BIT_SIZE);
            protocol->last_short = false;
        } else {
            protocol->last_short = true;
        }
    }

    return level_duration_make(protocol->last_level, duration);
};

// 0  nnnnnnnn
// 8  nnnnnnnn	  38 bit (12 digit) National code.
// 16 nnnnnnnn	  eg. 000000001008 (decimal).
// 24 nnnnnnnn
// 32 nnnnnnnn	  10 bit (3 digit) Country code.
// 40 cccccccc	  eg. 999 (decimal).
// 48 s-------	  1 bit data block status flag.
// 56 -------a	  1 bit animal application indicator.
// 64 eeeeeeee	  24 bits of extra data if present.
// 72 eeeeeeee	  eg. $123456.
// 80 eeeeeeee

static uint64_t protocol_fdx_b_get_national_code(const uint8_t* data) {
    uint64_t national_code = bit_lib_get_bits_32(data, 0, 32);
    national_code = national_code << 32;
    national_code |= bit_lib_get_bits_32(data, 32, 6) << (32 - 6);
    bit_lib_reverse_bits((uint8_t*)&national_code, 0, 64);
    return national_code;
}

static uint16_t protocol_fdx_b_get_country_code(const uint8_t* data) {
    uint16_t country_code = bit_lib_get_bits_16(data, 38, 10) << 6;
    bit_lib_reverse_bits((uint8_t*)&country_code, 0, 16);
    return country_code;
}

static bool protocol_fdx_b_get_temp(const uint8_t* data, float* temp) {
    uint32_t extended = bit_lib_get_bits_32(data, 64, 24) << 8;
    bit_lib_reverse_bits((uint8_t*)&extended, 0, 32);

    uint8_t ex_parity = (extended & 0x100) >> 8;
    uint8_t ex_temperature = extended & 0xff;
    uint8_t ex_calc_parity = bit_lib_test_parity_32(ex_temperature, BitLibParityOdd);
    bool ex_temperature_present = (ex_calc_parity == ex_parity) && !(extended & 0xe00);

    if(ex_temperature_present) {
        float temperature_f = 74 + ex_temperature * 0.2;
        *temp = temperature_f;
        return true;
    } else {
        return false;
    }
}

void protocol_fdx_b_render_data(ProtocolFDXB* protocol, string_t result) {
    // 38 bits of national code
    uint64_t national_code = protocol_fdx_b_get_national_code(protocol->data);

    // 10 bit of country code
    uint16_t country_code = protocol_fdx_b_get_country_code(protocol->data);

    bool block_status = bit_lib_get_bit(protocol->data, 48);
    bool rudi_bit = bit_lib_get_bit(protocol->data, 49);
    uint8_t reserved = bit_lib_get_bits(protocol->data, 50, 5);
    uint8_t user_info = bit_lib_get_bits(protocol->data, 55, 5);
    uint8_t replacement_number = bit_lib_get_bits(protocol->data, 60, 3);
    bool animal_flag = bit_lib_get_bit(protocol->data, 63);

    string_printf(result, "ID: %03u-%012llu\r\n", country_code, national_code);
    string_cat_printf(result, "Animal: %s, ", animal_flag ? "Yes" : "No");

    float temperature;
    if(protocol_fdx_b_get_temp(protocol->data, &temperature)) {
        float temperature_c = (temperature - 32) / 1.8;
        string_cat_printf(
            result, "T: %.2fF, %.2fC\r\n", (double)temperature, (double)temperature_c);
    } else {
        string_cat_printf(result, "T: ---\r\n");
    }

    string_cat_printf(
        result,
        "Bits: %X-%X-%X-%X-%X",
        block_status,
        rudi_bit,
        reserved,
        user_info,
        replacement_number);
};

void protocol_fdx_b_render_brief_data(ProtocolFDXB* protocol, string_t result) {
    // 38 bits of national code
    uint64_t national_code = protocol_fdx_b_get_national_code(protocol->data);

    // 10 bit of country code
    uint16_t country_code = protocol_fdx_b_get_country_code(protocol->data);

    bool animal_flag = bit_lib_get_bit(protocol->data, 63);

    string_printf(result, "ID: %03u-%012llu\r\n", country_code, national_code);
    string_cat_printf(result, "Animal: %s, ", animal_flag ? "Yes" : "No");

    float temperature;
    if(protocol_fdx_b_get_temp(protocol->data, &temperature)) {
        float temperature_c = (temperature - 32) / 1.8;
        string_cat_printf(result, "T: %.2fC", (double)temperature_c);
    } else {
        string_cat_printf(result, "T: ---");
    }
};

bool protocol_fdx_b_write_data(ProtocolFDXB* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_fdx_b_encoder_start(protocol);
    protocol_fdx_b_decode(protocol);

    protocol_fdx_b_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] = LFRFID_T5577_MODULATION_DIPHASE | LFRFID_T5577_BITRATE_RF_32 |
                                  (4 << LFRFID_T5577_MAXBLOCK_SHIFT);
        request->t5577.block[1] = bit_lib_get_bits_32(protocol->encoded_data, 0, 32);
        request->t5577.block[2] = bit_lib_get_bits_32(protocol->encoded_data, 32, 32);
        request->t5577.block[3] = bit_lib_get_bits_32(protocol->encoded_data, 64, 32);
        request->t5577.block[4] = bit_lib_get_bits_32(protocol->encoded_data, 96, 32);
        request->t5577.blocks_to_write = 5;
        result = true;
    }
    return result;
};

const ProtocolBase protocol_fdx_b = {
    .name = "FDX-B",
    .manufacturer = "ISO",
    .data_size = FDXB_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_fdx_b_alloc,
    .free = (ProtocolFree)protocol_fdx_b_free,
    .get_data = (ProtocolGetData)protocol_fdx_b_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_fdx_b_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_fdx_b_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_fdx_b_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_fdx_b_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_fdx_b_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_fdx_b_render_brief_data,
    .write_data = (ProtocolWriteData)protocol_fdx_b_write_data,
};