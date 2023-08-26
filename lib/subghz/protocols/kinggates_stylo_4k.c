#include "kinggates_stylo_4k.h"
#include "keeloq_common.h"

#include "../subghz_keystore.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocoKingGates_stylo_4k"

static const SubGhzBlockConst subghz_protocol_kinggates_stylo_4k_const = {
    .te_short = 400,
    .te_long = 1100,
    .te_delta = 140,
    .min_count_bit_for_found = 89,
};

struct SubGhzProtocolDecoderKingGates_stylo_4k {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    SubGhzKeystore* keystore;
};

struct SubGhzProtocolEncoderKingGates_stylo_4k {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
    SubGhzKeystore* keystore;
};

typedef enum {
    KingGates_stylo_4kDecoderStepReset = 0,
    KingGates_stylo_4kDecoderStepCheckPreambula,
    KingGates_stylo_4kDecoderStepCheckStartBit,
    KingGates_stylo_4kDecoderStepSaveDuration,
    KingGates_stylo_4kDecoderStepCheckDuration,
} KingGates_stylo_4kDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_kinggates_stylo_4k_decoder = {
    .alloc = subghz_protocol_decoder_kinggates_stylo_4k_alloc,
    .free = subghz_protocol_decoder_kinggates_stylo_4k_free,

    .feed = subghz_protocol_decoder_kinggates_stylo_4k_feed,
    .reset = subghz_protocol_decoder_kinggates_stylo_4k_reset,

    .get_hash_data = subghz_protocol_decoder_kinggates_stylo_4k_get_hash_data,
    .serialize = subghz_protocol_decoder_kinggates_stylo_4k_serialize,
    .deserialize = subghz_protocol_decoder_kinggates_stylo_4k_deserialize,
    .get_string = subghz_protocol_decoder_kinggates_stylo_4k_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_kinggates_stylo_4k_encoder = {
    .alloc = subghz_protocol_encoder_kinggates_stylo_4k_alloc,
    .free = subghz_protocol_encoder_kinggates_stylo_4k_free,

    .deserialize = subghz_protocol_encoder_kinggates_stylo_4k_deserialize,
    .stop = subghz_protocol_encoder_kinggates_stylo_4k_stop,
    .yield = subghz_protocol_encoder_kinggates_stylo_4k_yield,
};

const SubGhzProtocol subghz_protocol_kinggates_stylo_4k = {
    .name = SUBGHZ_PROTOCOL_KINGGATES_STYLO_4K_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_kinggates_stylo_4k_decoder,
    .encoder = &subghz_protocol_kinggates_stylo_4k_encoder,
};

//
// Encoder
//

// Pre define function
static void subghz_protocol_kinggates_stylo_4k_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore);

void* subghz_protocol_encoder_kinggates_stylo_4k_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderKingGates_stylo_4k* instance =
        malloc(sizeof(SubGhzProtocolEncoderKingGates_stylo_4k));

    instance->base.protocol = &subghz_protocol_kinggates_stylo_4k;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 512;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    return instance;
}

void subghz_protocol_encoder_kinggates_stylo_4k_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderKingGates_stylo_4k* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_encoder_kinggates_stylo_4k_stop(void* context) {
    SubGhzProtocolEncoderKingGates_stylo_4k* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_kinggates_stylo_4k_yield(void* context) {
    SubGhzProtocolEncoderKingGates_stylo_4k* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

/** 
 * Key generation from simple data
 * @param instance Pointer to a SubGhzProtocolEncoderKingGates_stylo_4k* instance
 * @param btn Button number, 4 bit
 */
static bool subghz_protocol_kinggates_stylo_4k_gen_data(
    SubGhzProtocolEncoderKingGates_stylo_4k* instance,
    uint8_t btn) {
    UNUSED(btn);
    uint32_t hop = subghz_protocol_blocks_reverse_key(instance->generic.data_2 >> 4, 32);
    uint64_t fix = subghz_protocol_blocks_reverse_key(instance->generic.data, 53);
    int res = 0;
    uint32_t decrypt = 0;

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(furi_string_get_cstr(manufacture_code->name), "Kingates_Stylo4k");
            if(res == 0) {
                //Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                break;
            }
        }
    instance->generic.cnt = decrypt & 0xFFFF;

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }

    instance->generic.btn = (fix >> 17) & 0x0F;
    instance->generic.serial = ((fix >> 5) & 0xFFFF0000) | (fix & 0xFFFF);

    uint32_t data = (decrypt & 0xFFFF0000) | instance->generic.cnt;

    uint64_t encrypt = 0;
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(furi_string_get_cstr(manufacture_code->name), "Kingates_Stylo4k");
            if(res == 0) {
                //Simple Learning
                encrypt = subghz_protocol_keeloq_common_encrypt(data, manufacture_code->key);
                encrypt = subghz_protocol_blocks_reverse_key(encrypt, 32);
                instance->generic.data_2 = encrypt << 4;
                return true;
            }
        }

    return false;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderKingGates_stylo_4k instance
 * @return true On success
 */
static bool subghz_protocol_encoder_kinggates_stylo_4k_get_upload(
    SubGhzProtocolEncoderKingGates_stylo_4k* instance,
    uint8_t btn) {
    furi_assert(instance);

    // Gen new key
    if(!subghz_protocol_kinggates_stylo_4k_gen_data(instance, btn)) {
        return false;
    }

    size_t index = 0;

    // Start
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)9500);

    // Send header
    for(uint8_t i = 12; i > 0; i--) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
        instance->encoder.upload[index++] = level_duration_make(
            false, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
    }

    // After header
    instance->encoder.upload[index - 1].duration =
        (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long * 2;
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short * 2);

    // Send key fix
    for(uint8_t i = 53; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long);
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
        }
    }

    // Send key hop
    for(uint8_t i = 36; i > 0; i--) {
        if(bit_read(instance->generic.data_2, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long);
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_short);
        }
    }

    // Set upload size after generating upload, fix it later

    instance->encoder.size_upload = index;

    return true;
}

SubGhzProtocolStatus subghz_protocol_encoder_kinggates_stylo_4k_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderKingGates_stylo_4k* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        subghz_protocol_kinggates_stylo_4k_remote_controller(
            &instance->generic, instance->keystore);

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            break;
        }

        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }

        subghz_protocol_encoder_kinggates_stylo_4k_get_upload(instance, instance->generic.btn);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

//
// Decoder
//
void* subghz_protocol_decoder_kinggates_stylo_4k_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderKingGates_stylo_4k* instance =
        malloc(sizeof(SubGhzProtocolDecoderKingGates_stylo_4k));
    instance->base.protocol = &subghz_protocol_kinggates_stylo_4k;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);
    return instance;
}

void subghz_protocol_decoder_kinggates_stylo_4k_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    free(instance);
}

void subghz_protocol_decoder_kinggates_stylo_4k_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
}

void subghz_protocol_decoder_kinggates_stylo_4k_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;

    switch(instance->decoder.parser_step) {
    case KingGates_stylo_4kDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_short) <
                          subghz_protocol_kinggates_stylo_4k_const.te_delta) {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepCheckPreambula;
            instance->header_count++;
        }
        break;
    case KingGates_stylo_4kDecoderStepCheckPreambula:
        if((!level) &&
           (DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_short) <
            subghz_protocol_kinggates_stylo_4k_const.te_delta)) {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
            break;
        }
        if((instance->header_count > 2) &&
           (DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_long * 2) <
            subghz_protocol_kinggates_stylo_4k_const.te_delta * 2)) {
            // Found header
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepCheckStartBit;
        } else {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    case KingGates_stylo_4kDecoderStepCheckStartBit:
        if((level) &&
           DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_short * 2) <
               subghz_protocol_kinggates_stylo_4k_const.te_delta * 2) {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->generic.data_2 = 0;
            instance->decoder.decode_count_bit = 0;
            instance->header_count = 0;
        }
        break;
    case KingGates_stylo_4kDecoderStepSaveDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_kinggates_stylo_4k_const.te_long * 3)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_kinggates_stylo_4k_const.min_count_bit_for_found) {
                    instance->generic.data = instance->generic.data_2;
                    instance->generic.data_2 = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }

                instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
                instance->decoder.decode_data = 0;
                instance->generic.data_2 = 0;
                instance->decoder.decode_count_bit = 0;
                instance->header_count = 0;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = KingGates_stylo_4kDecoderStepCheckDuration;
            }
        } else {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    case KingGates_stylo_4kDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_kinggates_stylo_4k_const.te_short) <
                subghz_protocol_kinggates_stylo_4k_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_long) <
                subghz_protocol_kinggates_stylo_4k_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = KingGates_stylo_4kDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_kinggates_stylo_4k_const.te_long) <
                 subghz_protocol_kinggates_stylo_4k_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_kinggates_stylo_4k_const.te_short) <
                 subghz_protocol_kinggates_stylo_4k_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = KingGates_stylo_4kDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
                instance->header_count = 0;
            }
            if(instance->decoder.decode_count_bit == 53) {
                instance->generic.data_2 = instance->decoder.decode_data;
                instance->decoder.decode_data = 0;
            }
        } else {
            instance->decoder.parser_step = KingGates_stylo_4kDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param data Input encrypted data
 * @param keystore Pointer to a SubGhzKeystore* instance
 */
static void subghz_protocol_kinggates_stylo_4k_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore) {
    /**
 *  9500us   12*(400/400)  2200/800|1-bit|0-bit|
 *           _   _       _       __   ___     _ 
 *  ________| |_| |_..._| |_____|  |_|   |___| |.....
 * 
 *  1-bit 400/1100 us
 *  0-bit 1100/400 us
 *  
 *  The package consists of 89 bits of data, LSB first  
 *  Data        - 1C9037F0C80000 CE280BA00
 *    S[3]     S[2]   1 key    S[1]     S[0]    2 byte always 0   Hop[3]   Hop[2]  Hop[1]   Hop[0]    0
 *  11100100 10000001 1 0111 11110000 11001000 00000000 00000000 11001110 00101000 00001011 10100000 0000
 * 
 *  Encryption  - keeloq Simple Learning
 *                                         key C  S[3]  CNT 
 *  Decrypt     - 0xEC270B9C        =>  0x  E  C   27   0B9C
 * 
 * 
 * 
*/

    uint32_t hop = subghz_protocol_blocks_reverse_key(instance->data_2 >> 4, 32);
    uint64_t fix = subghz_protocol_blocks_reverse_key(instance->data, 53);
    bool ret = false;
    uint32_t decrypt = 0;
    instance->btn = (fix >> 17) & 0x0F;
    instance->serial = ((fix >> 5) & 0xFFFF0000) | (fix & 0xFFFF);

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            if(manufacture_code->type == KEELOQ_LEARNING_SIMPLE) {
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if(((decrypt >> 28) == instance->btn) && (((decrypt >> 24) & 0x0F) == 0x0C) &&
                   (((decrypt >> 16) & 0xFF) == (instance->serial & 0xFF))) {
                    ret = true;
                    break;
                }
            }
        }
    if(ret) {
        instance->cnt = decrypt & 0xFFFF;
    } else {
        instance->btn = 0;
        instance->serial = 0;
        instance->cnt = 0;
    }
}

uint8_t subghz_protocol_decoder_kinggates_stylo_4k_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_kinggates_stylo_4k_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> (i * 8)) & 0xFF;
    }

    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Data");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    return ret;
}

SubGhzProtocolStatus subghz_protocol_decoder_kinggates_stylo_4k_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_kinggates_stylo_4k_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }

        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }
    } while(false);
    return ret;
}

void subghz_protocol_decoder_kinggates_stylo_4k_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderKingGates_stylo_4k* instance = context;
    subghz_protocol_kinggates_stylo_4k_remote_controller(&instance->generic, instance->keystore);

    furi_string_cat_printf(
        output,
        "%s\r\n"
        "Key:0x%llX%07llX  %dbit\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%04lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data,
        instance->generic.data_2,
        instance->generic.data_count_bit,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}