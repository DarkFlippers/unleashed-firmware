#include "faac_slh.h"
#include "../subghz_keystore.h"
#include <m-string.h>
#include <m-array.h>
#include "keeloq_common.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolFaacSHL"

static const SubGhzBlockConst subghz_protocol_faac_slh_const = {
    .te_short = 255,
    .te_long = 595,
    .te_delta = 100,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderFaacSLH {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
    const char* manufacture_name;
};

struct SubGhzProtocolEncoderFaacSLH {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
    const char* manufacture_name;
};

typedef enum {
    FaacSLHDecoderStepReset = 0,
    FaacSLHDecoderStepFoundPreambula,
    FaacSLHDecoderStepSaveDuration,
    FaacSLHDecoderStepCheckDuration,
} FaacSLHDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_faac_slh_decoder = {
    .alloc = subghz_protocol_decoder_faac_slh_alloc,
    .free = subghz_protocol_decoder_faac_slh_free,

    .feed = subghz_protocol_decoder_faac_slh_feed,
    .reset = subghz_protocol_decoder_faac_slh_reset,

    .get_hash_data = subghz_protocol_decoder_faac_slh_get_hash_data,
    .serialize = subghz_protocol_decoder_faac_slh_serialize,
    .deserialize = subghz_protocol_decoder_faac_slh_deserialize,
    .get_string = subghz_protocol_decoder_faac_slh_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_faac_slh_encoder = {
    .alloc = subghz_protocol_encoder_faac_slh_alloc,
    .free = subghz_protocol_encoder_faac_slh_free,

    .deserialize = subghz_protocol_encoder_faac_slh_deserialize,
    .stop = subghz_protocol_encoder_faac_slh_stop,
    .yield = subghz_protocol_encoder_faac_slh_yield,
};

const SubGhzProtocol subghz_protocol_faac_slh = {
    .name = SUBGHZ_PROTOCOL_FAAC_SLH_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_faac_slh_decoder,
    .encoder = &subghz_protocol_faac_slh_encoder,
};

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manufacture_name
 */
static void subghz_protocol_faac_slh_check_remote_controller
    (SubGhzBlockGeneric* instance,
     SubGhzKeystore* keystore,
     const char** manufacture_name);

void* subghz_protocol_encoder_faac_slh_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderFaacSLH* instance = malloc(sizeof(SubGhzProtocolEncoderFaacSLH));

    instance->base.protocol = &subghz_protocol_faac_slh;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_faac_slh_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacSLH* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static bool subghz_protocol_faac_slh_gen_data(SubGhzProtocolEncoderFaacSLH* instance) {
    instance->generic.cnt++;
    uint32_t fix = instance->generic.data >> 32;
    uint32_t hop = 0;
    uint32_t decrypt = 0;
    uint64_t man = 0;
    uint32_t seed = 0x77ED7698;
    int res = 0;
    char fixx[8] = {};
    int shiftby = 32;
    for (int i = 0; i < 8; i++) {
        fixx[i] = (fix >> (shiftby -= 4)) & 0xF;
    }
    if ((instance->generic.cnt % 2) == 0) {
        decrypt = fixx[6] << 28 | fixx[7] << 24 | fixx[5] << 20 | 
        fixx[1] << 16 | instance->generic.cnt; 
    }
    else {
        decrypt = fixx[2] << 28 | fixx[3] << 24 | fixx[4] << 20 | 
        fixx[1] << 16 | instance->generic.cnt;
    }
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(string_get_cstr(manufacture_code->name), instance->manufacture_name);
            if(res == 0) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_FAAC:
                    //FAAC Learning
                    man =
                        subghz_protocol_keeloq_common_faac_learning(seed, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    break;
                }
                break;
            }
        }
    if(hop) {
        instance->generic.data = (uint64_t)fix << 32 | hop;
    }
    return true;
}

bool subghz_protocol_faac_slh_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacSLH* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->manufacture_name = manufacture_name;
    instance->generic.data_count_bit = 64;
    bool res = subghz_protocol_faac_slh_gen_data(instance);
    if(res) {
        res =
            subghz_block_generic_serialize(&instance->generic, flipper_format, frequency, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_faac_slh_get_upload(SubGhzProtocolEncoderFaacSLH* instance) {
    furi_assert(instance);

    //gen new key
    if(subghz_protocol_faac_slh_gen_data(instance)) {
        //ToDo if you need to add a callback to automatically update the data on the display
    } else {
        return false;
    }

    size_t index = 0;
    size_t size_upload = 2 + (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 11; i > 0; i--) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_faac_slh_const.te_long * 2);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_faac_slh_const.te_short * 2);
    }

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_faac_slh_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_faac_slh_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_faac_slh_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_faac_slh_const.te_long);
        }
    }
    return true;
}

bool subghz_protocol_encoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacSLH* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        subghz_protocol_faac_slh_check_remote_controller(
            &instance->generic, instance->keystore, &instance->manufacture_name);

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_faac_slh_get_upload(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_runing = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_faac_slh_stop(void* context) {
    SubGhzProtocolEncoderFaacSLH* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_faac_slh_yield(void* context) {
    SubGhzProtocolEncoderFaacSLH* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_runing) {
        instance->encoder.is_runing = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_faac_slh_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderFaacSLH* instance = malloc(sizeof(SubGhzProtocolDecoderFaacSLH));
    instance->base.protocol = &subghz_protocol_faac_slh;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);
    return instance;
}

void subghz_protocol_decoder_faac_slh_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    free(instance);
}

void subghz_protocol_decoder_faac_slh_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    instance->decoder.parser_step = FaacSLHDecoderStepReset;
}

void subghz_protocol_decoder_faac_slh_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;

    switch(instance->decoder.parser_step) {
    case FaacSLHDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long * 2) <
                       subghz_protocol_faac_slh_const.te_delta * 3)) {
            instance->decoder.parser_step = FaacSLHDecoderStepFoundPreambula;
        }
        break;
    case FaacSLHDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long * 2) <
                        subghz_protocol_faac_slh_const.te_delta * 3)) {
            //Found Preambula
            instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepSaveDuration:
        if(level) {
            if(duration >= (subghz_protocol_faac_slh_const.te_short * 3 +
                            subghz_protocol_faac_slh_const.te_delta)) {
                instance->decoder.parser_step = FaacSLHDecoderStepFoundPreambula;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_faac_slh_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = FaacSLHDecoderStepCheckDuration;
            }

        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_slh_const.te_short) <
                subghz_protocol_faac_slh_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long) <
                subghz_protocol_faac_slh_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_slh_const.te_long) <
                 subghz_protocol_faac_slh_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_short) <
                 subghz_protocol_faac_slh_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = FaacSLHDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_faac_slh_check_remote_controller
    (SubGhzBlockGeneric* instance,
     SubGhzKeystore* keystore,
     const char** manufacture_name) {
    //uint64_t code_found_reverse =
        //subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit);
    uint32_t code_fix = instance->data >> 32;
    uint32_t code_hop = instance->data & 0xFFFFFFFF;
    instance->serial = code_fix >> 4;
    instance->btn = code_fix & 0xF;
    uint32_t decrypt = 0;
    uint64_t man;
    uint32_t seed = 0x77ED7698;

    for
    M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
        uint32_t hi = manufacture_code->key >> 32;
        uint32_t lo = manufacture_code->key & 0xFFFFFFFF;
        switch(manufacture_code->type) {
        case KEELOQ_LEARNING_FAAC:
        // FAAC Learning
        man = subghz_protocol_keeloq_common_faac_learning(seed, manufacture_code->key);
        FURI_LOG_I(TAG, "mfkey: %08lX%08lX mf: %s", hi, lo, manufacture_code->name);
        decrypt = subghz_protocol_keeloq_common_decrypt(code_hop, man);
        *manufacture_name = string_get_cstr(manufacture_code->name);
        break;
        }
    } instance->cnt = (decrypt & 0xFFFF);
}

uint8_t subghz_protocol_decoder_faac_slh_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_faac_slh_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, frequency, preset);
}

bool subghz_protocol_decoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_block_generic_deserialize(&instance->generic, flipper_format);
}

void subghz_protocol_decoder_faac_slh_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    subghz_protocol_faac_slh_check_remote_controller(&instance->generic, instance->keystore, &instance->manufacture_name);
    //uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        //instance->generic.data, instance->generic.data_count_bit);
    uint32_t code_fix = instance->generic.data >> 32;
    uint32_t code_hop = instance->generic.data & 0xFFFFFFFF;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%lX%08lX\r\n"
        "Fix:%08lX    Cnt:%04X\r\n"
        "Hop:%08lX    Btn:%lX\r\n"
        "Sn:%07lX \r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        code_fix,
        instance->generic.cnt,
        code_hop,
        instance->generic.btn,
        instance->generic.serial);
}
