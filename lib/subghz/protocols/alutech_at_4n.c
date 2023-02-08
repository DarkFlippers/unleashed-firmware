#include "alutech_at_4n.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocoAlutech_at_4n"

#define SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE 0xFFFFFFFF

static const SubGhzBlockConst subghz_protocol_alutech_at_4n_const = {
    .te_short = 400,
    .te_long = 800,
    .te_delta = 140,
    .min_count_bit_for_found = 72,
};

struct SubGhzProtocolDecoderAlutech_at_4n {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint64_t data;
    uint32_t crc;
    uint16_t header_count;

    const char* alutech_at_4n_rainbow_table_file_name;
};

struct SubGhzProtocolEncoderAlutech_at_4n {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Alutech_at_4nDecoderStepReset = 0,
    Alutech_at_4nDecoderStepCheckPreambula,
    Alutech_at_4nDecoderStepSaveDuration,
    Alutech_at_4nDecoderStepCheckDuration,
} Alutech_at_4nDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_alutech_at_4n_decoder = {
    .alloc = subghz_protocol_decoder_alutech_at_4n_alloc,
    .free = subghz_protocol_decoder_alutech_at_4n_free,

    .feed = subghz_protocol_decoder_alutech_at_4n_feed,
    .reset = subghz_protocol_decoder_alutech_at_4n_reset,

    .get_hash_data = subghz_protocol_decoder_alutech_at_4n_get_hash_data,
    .serialize = subghz_protocol_decoder_alutech_at_4n_serialize,
    .deserialize = subghz_protocol_decoder_alutech_at_4n_deserialize,
    .get_string = subghz_protocol_decoder_alutech_at_4n_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_alutech_at_4n_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_alutech_at_4n = {
    .name = SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_alutech_at_4n_decoder,
    .encoder = &subghz_protocol_alutech_at_4n_encoder,
};

/**
 * Read bytes from rainbow table
 * @param file_name Full path to rainbow table the file
 * @param number_alutech_at_4n_magic_data number in the array
 * @return alutech_at_4n_magic_data
 */
static uint32_t subghz_protocol_alutech_at_4n_get_magic_data_in_file(
    const char* file_name,
    uint8_t number_alutech_at_4n_magic_data) {
    if(!strcmp(file_name, "")) return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;

    uint8_t buffer[sizeof(uint32_t)] = {0};
    uint32_t address = number_alutech_at_4n_magic_data * sizeof(uint32_t);
    uint32_t alutech_at_4n_magic_data = 0;

    if(subghz_keystore_raw_get_data(file_name, address, buffer, sizeof(uint32_t))) {
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            alutech_at_4n_magic_data = (alutech_at_4n_magic_data << 8) | buffer[i];
        }
    } else {
        alutech_at_4n_magic_data = SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;
    }
    return alutech_at_4n_magic_data;
}

static uint8_t subghz_protocol_alutech_at_4n_crc(uint64_t data) {
    uint8_t* p = (uint8_t*)&data;
    uint8_t crc = 0xff;
    for(uint8_t y = 0; y < 8; y++) {
        crc = crc ^ p[y];
        for(uint8_t i = 0; i < 8; i++) {
            if((crc & 0x80) != 0) {
                crc <<= 1;
                crc ^= 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static uint8_t subghz_protocol_alutech_at_4n_decrypt_data_crc(uint8_t data) {
    uint8_t crc = data;
    for(uint8_t i = 0; i < 8; i++) {
        if((crc & 0x80) != 0) {
            crc <<= 1;
            crc ^= 0x31;
        } else {
            crc <<= 1;
        }
    }
    return ~crc;
}

static uint64_t subghz_protocol_alutech_at_4n_decrypt(uint64_t data, const char* file_name) {
    uint8_t* p = (uint8_t*)&data;
    uint32_t data1 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    uint32_t data2 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
    uint32_t data3 = 0;
    uint32_t magic_data[] = {
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 0),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 1),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 2),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 3),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 4),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 5)};

    uint32_t i = magic_data[0];
    do {
        data2 = data2 -
                ((magic_data[1] + (data1 << 4)) ^ ((magic_data[2] + (data1 >> 5)) ^ (data1 + i)));
        data3 = data2 + i;
        i += magic_data[3];
        data1 =
            data1 - ((magic_data[4] + (data2 << 4)) ^ ((magic_data[5] + (data2 >> 5)) ^ data3));
    } while(i != 0);

    p[0] = (uint8_t)(data1 >> 24);
    p[1] = (uint8_t)(data1 >> 16);
    p[3] = (uint8_t)data1;
    p[4] = (uint8_t)(data2 >> 24);
    p[5] = (uint8_t)(data2 >> 16);
    p[2] = (uint8_t)(data1 >> 8);
    p[6] = (uint8_t)(data2 >> 8);
    p[7] = (uint8_t)data2;

    return data;
}

// static uint64_t subghz_protocol_alutech_at_4n_encrypt(uint64_t data, const char* file_name) {
//     uint8_t* p = (uint8_t*)&data;
//     uint32_t data1 = 0;
//     uint32_t data2 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
//     uint32_t data3 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
//     uint32_t magic_data[] = {
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 6),
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 4),
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 5),
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 1),
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 2),
//         subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 0)};

//     do {
//         data1 = data1 + magic_data[0];
//         data2 = data2 + ((magic_data[1] + (data3 << 4)) ^
//                          ((magic_data[2] + (data3 >> 5)) ^ (data1 + data3)));
//         data3 = data3 + ((magic_data[3] + (data2 << 4)) ^
//                          ((magic_data[4] + (data2 >> 5)) ^ (data1 + data2)));
//     } while(data1 != magic_data[5]);
//     p[0] = (uint8_t)(data2 >> 24);
//     p[1] = (uint8_t)(data2 >> 16);
//     p[3] = (uint8_t)data2;
//     p[4] = (uint8_t)(data3 >> 24);
//     p[5] = (uint8_t)(data3 >> 16);
//     p[2] = (uint8_t)(data2 >> 8);
//     p[6] = (uint8_t)(data3 >> 8);
//     p[7] = (uint8_t)data3;

//     return data;
// }

void* subghz_protocol_decoder_alutech_at_4n_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderAlutech_at_4n* instance =
        malloc(sizeof(SubGhzProtocolDecoderAlutech_at_4n));
    instance->base.protocol = &subghz_protocol_alutech_at_4n;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->alutech_at_4n_rainbow_table_file_name =
        subghz_environment_get_alutech_at_4n_rainbow_table_file_name(environment);
    if(instance->alutech_at_4n_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->alutech_at_4n_rainbow_table_file_name);
    }
    return instance;
}

void subghz_protocol_decoder_alutech_at_4n_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    instance->alutech_at_4n_rainbow_table_file_name = NULL;
    free(instance);
}

void subghz_protocol_decoder_alutech_at_4n_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
}

void subghz_protocol_decoder_alutech_at_4n_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;

    switch(instance->decoder.parser_step) {
    case Alutech_at_4nDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                          subghz_protocol_alutech_at_4n_const.te_delta) {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepCheckPreambula;
            instance->header_count++;
        }
        break;
    case Alutech_at_4nDecoderStepCheckPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                        subghz_protocol_alutech_at_4n_const.te_delta)) {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            break;
        }
        if((instance->header_count > 2) &&
           (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short * 10) <
            subghz_protocol_alutech_at_4n_const.te_delta * 10)) {
            // Found header
            instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    case Alutech_at_4nDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Alutech_at_4nDecoderStepCheckDuration;
        }
        break;
    case Alutech_at_4nDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_alutech_at_4n_const.te_short * 2 +
                            subghz_protocol_alutech_at_4n_const.te_delta)) {
                //add last bit
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_short) <
                   subghz_protocol_alutech_at_4n_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_long) <
                    subghz_protocol_alutech_at_4n_const.te_delta * 2) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }

                // Found end TX
                instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_alutech_at_4n_const.min_count_bit_for_found) {
                    if(instance->generic.data != instance->data) {
                        instance->generic.data = instance->data;

                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        instance->crc = instance->decoder.decode_data;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->data = 0;
                    instance->decoder.decode_count_bit = 0;
                    instance->header_count = 0;
                }
                break;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_short) <
                 subghz_protocol_alutech_at_4n_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_long) <
                 subghz_protocol_alutech_at_4n_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                if(instance->decoder.decode_count_bit == 64) {
                    instance->data = instance->decoder.decode_data;
                    instance->decoder.decode_data = 0;
                }
                instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_long) <
                 subghz_protocol_alutech_at_4n_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                 subghz_protocol_alutech_at_4n_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                if(instance->decoder.decode_count_bit == 64) {
                    instance->data = instance->decoder.decode_data;
                    instance->decoder.decode_data = 0;
                }
                instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
                instance->header_count = 0;
            }
        } else {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file
 */
static void subghz_protocol_alutech_at_4n_remote_controller(
    SubGhzBlockGeneric* instance,
    uint8_t crc,
    const char* file_name) {
    /**
 *  Message format 72bit LSB first
 *           data        crc
 *      XXXXXXXXXXXXXXXX  CC
 *  
 *  For analysis, you need to turn the package MSB
 *  in decoded messages format
 * 
 *     crc1 serial  cnt  key
 *      cc SSSSSSSS XXxx BB 
 * 
 *  crc1 is calculated from the lower part of cnt
 *  key 1=0xff, 2=0x11, 3=0x22, 4=0x33, 5=0x44
 * 
 */

    bool status = false;
    uint64_t data = subghz_protocol_blocks_reverse_key(instance->data, 64);
    crc = subghz_protocol_blocks_reverse_key(crc, 8);

    if(crc == subghz_protocol_alutech_at_4n_crc(data)) {
        data = subghz_protocol_alutech_at_4n_decrypt(data, file_name);
        status = true;
    }

    if(status && ((uint8_t)(data >> 56) ==
                  subghz_protocol_alutech_at_4n_decrypt_data_crc((uint8_t)((data >> 8) & 0xFF)))) {
        instance->btn = (uint8_t)data & 0xFF;
        instance->cnt = (uint16_t)(data >> 8) & 0xFFFF;
        instance->serial = (uint32_t)(data >> 24) & 0xFFFFFFFF;
    }

    if(!status) {
        instance->btn = 0;
        instance->cnt = 0;
        instance->serial = 0;
    }
}

uint8_t subghz_protocol_decoder_alutech_at_4n_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    return (uint8_t)instance->crc;
}

bool subghz_protocol_decoder_alutech_at_4n_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    bool res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if(res && !flipper_format_write_uint32(flipper_format, "CRC", &instance->crc, 1)) {
        FURI_LOG_E(TAG, "Unable to add CRC");
        res = false;
    }
    return res;

    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_alutech_at_4n_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_alutech_at_4n_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "CRC", (uint32_t*)&instance->crc, 1)) {
            FURI_LOG_E(TAG, "Missing CRC");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_alutech_at_4n_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    subghz_protocol_alutech_at_4n_remote_controller(
        &instance->generic, instance->crc, instance->alutech_at_4n_rainbow_table_file_name);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %d\r\n"
        "Key:0x%08lX%08lX%02X\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%03lX\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        (uint8_t)instance->crc,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
