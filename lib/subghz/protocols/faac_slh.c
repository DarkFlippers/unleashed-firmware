#include "faac_slh.h"
#include "../subghz_keystore.h"
#include <m-array.h>
#include "keeloq_common.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolFaacSLH"

static const SubGhzBlockConst subghz_protocol_faac_slh_const = {
    .te_short = 255,
    .te_long = 595,
    .te_delta = 100,
    .min_count_bit_for_found = 64,
};

static uint32_t temp_fix_backup = 0;
static uint32_t temp_counter_backup = 0;
static bool faac_prog_mode = false;
static bool allow_zero_seed = false;

void faac_slh_reset_prog_mode() {
    temp_fix_backup = 0;
    temp_counter_backup = 0;
    faac_prog_mode = false;
    allow_zero_seed = false;
}

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
static void subghz_protocol_faac_slh_check_remote_controller(
    SubGhzBlockGeneric* instance,
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
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_faac_slh_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacSLH* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static bool subghz_protocol_faac_slh_gen_data(SubGhzProtocolEncoderFaacSLH* instance) {
    // TODO: Stupid bypass for custom button, remake later
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(0xF);
    }

    uint8_t custom_btn_id = subghz_custom_btn_get();

    // If we are using UP button - generate programming mode key and send it, otherwise - send regular key if possible
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) &&
       !(!allow_zero_seed && (instance->generic.seed == 0x0))) {
        uint8_t data_tmp = 0;
        uint8_t data_prg[8];

        data_prg[0] = 0x00;

        if(allow_zero_seed || (instance->generic.seed != 0x0)) {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            if(temp_counter_backup != 0x0) {
                temp_counter_backup += furi_hal_subghz_get_rolling_counter_mult();
            }
        }

        data_prg[1] = instance->generic.cnt & 0xFF;

        data_prg[2] = (uint8_t)(instance->generic.seed & 0xFF);
        data_prg[3] = (uint8_t)(instance->generic.seed >> 8 & 0xFF);
        data_prg[4] = (uint8_t)(instance->generic.seed >> 16 & 0xFF);
        data_prg[5] = (uint8_t)(instance->generic.seed >> 24);

        data_prg[2] ^= data_prg[1];
        data_prg[3] ^= data_prg[1];
        data_prg[4] ^= data_prg[1];
        data_prg[5] ^= data_prg[1];

        for(uint8_t i = data_prg[1] & 0x0F; i != 0; i--) {
            data_tmp = data_prg[5];

            data_prg[5] = ((data_prg[5] << 1) & 0xFF) | (data_prg[4] & 0x80) >> 7;
            data_prg[4] = ((data_prg[4] << 1) & 0xFF) | (data_prg[3] & 0x80) >> 7;
            data_prg[3] = ((data_prg[3] << 1) & 0xFF) | (data_prg[2] & 0x80) >> 7;
            data_prg[2] = ((data_prg[2] << 1) & 0xFF) | (data_tmp & 0x80) >> 7;
        }
        data_prg[6] = 0x0F;
        data_prg[7] = 0x52;

        uint32_t enc_prg_1 = data_prg[7] << 24 | data_prg[6] << 16 | data_prg[5] << 8 |
                             data_prg[4];
        uint32_t enc_prg_2 = data_prg[3] << 24 | data_prg[2] << 16 | data_prg[1] << 8 |
                             data_prg[0];
        instance->generic.data = (uint64_t)enc_prg_1 << 32 | enc_prg_2;
        //FURI_LOG_D(TAG, "New Prog Mode Key Generated: %016llX\r", instance->generic.data);

        return true;
    } else {
        if(!allow_zero_seed && (instance->generic.seed == 0x0)) {
            // Do not generate new data, send data from buffer
            return true;
        }
        // If we are in prog mode and regular Send button is used - Do not generate new data, send data from buffer
        if((faac_prog_mode == true) && (instance->generic.serial == 0x0) &&
           (instance->generic.btn == 0x0) && (temp_fix_backup == 0x0)) {
            return true;
        }
    }
    // Restore main remote data when we exit programming mode
    if((instance->generic.serial == 0x0) && (instance->generic.btn == 0x0) &&
       (temp_fix_backup != 0x0) && !faac_prog_mode) {
        instance->generic.serial = temp_fix_backup >> 4;
        instance->generic.btn = temp_fix_backup & 0xF;
        instance->generic.cnt = temp_counter_backup;
    }
    uint32_t fix = instance->generic.serial << 4 | instance->generic.btn;
    uint32_t hop = 0;
    uint32_t decrypt = 0;
    uint64_t man = 0;
    int res = 0;
    char fixx[8] = {};
    int shiftby = 32;
    for(int i = 0; i < 8; i++) {
        fixx[i] = (fix >> (shiftby -= 4)) & 0xF;
    }

    if(allow_zero_seed || (instance->generic.seed != 0x0)) {
        instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
    }

    if((instance->generic.cnt % 2) == 0) {
        decrypt = fixx[6] << 28 | fixx[7] << 24 | fixx[5] << 20 |
                  (instance->generic.cnt & 0xFFFFF);
    } else {
        decrypt = fixx[2] << 28 | fixx[3] << 24 | fixx[4] << 20 |
                  (instance->generic.cnt & 0xFFFFF);
    }
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(furi_string_get_cstr(manufacture_code->name), instance->manufacture_name);
            if(res == 0) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_FAAC:
                    //FAAC Learning
                    man = subghz_protocol_keeloq_common_faac_learning(
                        instance->generic.seed, manufacture_code->key);
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
    uint32_t cnt,
    uint32_t seed,
    const char* manufacture_name,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    // roguemaster don't steal!!!
    SubGhzProtocolEncoderFaacSLH* instance = context;
    instance->generic.serial = serial;
    instance->generic.btn = btn;
    instance->generic.cnt = (cnt & 0xFFFFF);
    instance->generic.seed = seed;
    instance->manufacture_name = manufacture_name;
    instance->generic.data_count_bit = 64;
    allow_zero_seed = true;
    bool res = subghz_protocol_faac_slh_gen_data(instance);
    if(res) {
        return SubGhzProtocolStatusOk ==
               subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @return true On success
 */
static bool subghz_protocol_encoder_faac_slh_get_upload(SubGhzProtocolEncoderFaacSLH* instance) {
    furi_assert(instance);

    subghz_protocol_faac_slh_gen_data(instance);
    size_t index = 0;
    size_t size_upload = 2 + (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_faac_slh_const.te_long * 2);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_faac_slh_const.te_long * 2);

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

SubGhzProtocolStatus
    subghz_protocol_encoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacSLH* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
        }
        if(!flipper_format_read_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
            FURI_LOG_E(TAG, "Missing Seed");
            break;
        }
        bool tmp_allow_zero_seed;
        if(flipper_format_read_bool(flipper_format, "AllowZeroSeed", &tmp_allow_zero_seed, 1)) {
            allow_zero_seed = true;
        } else {
            allow_zero_seed = false;
        }

        instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                                 seed_data[3];

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

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void subghz_protocol_encoder_faac_slh_stop(void* context) {
    SubGhzProtocolEncoderFaacSLH* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_faac_slh_yield(void* context) {
    SubGhzProtocolEncoderFaacSLH* instance = context;

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

void* subghz_protocol_decoder_faac_slh_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
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
            if(duration >= ((uint32_t)subghz_protocol_faac_slh_const.te_short * 3 +
                            subghz_protocol_faac_slh_const.te_delta)) {
                instance->decoder.parser_step = FaacSLHDecoderStepFoundPreambula;
                if(instance->decoder.decode_count_bit ==
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
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manifacture_name Manufacturer name
 */
static void subghz_protocol_faac_slh_check_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore,
    const char** manufacture_name) {
    uint32_t code_fix = instance->data >> 32;
    uint32_t code_hop = instance->data & 0xFFFFFFFF;
    uint32_t decrypt = 0;
    uint64_t man;

    // TODO: Stupid bypass for custom button, remake later
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(0xF);
    }

    subghz_custom_btn_set_max(1);

    uint8_t data_tmp = 0;
    uint8_t data_prg[8];
    data_prg[0] = (code_hop & 0xFF);
    data_prg[1] = ((code_hop >> 8) & 0xFF);
    data_prg[2] = ((code_hop >> 16) & 0xFF);
    data_prg[3] = (code_hop >> 24);
    data_prg[4] = (code_fix & 0xFF);
    data_prg[5] = ((code_fix >> 8) & 0xFF);
    data_prg[6] = ((code_fix >> 16) & 0xFF);
    data_prg[7] = (code_fix >> 24);

    if(((data_prg[7] == 0x52) && (data_prg[6] == 0x0F) && (data_prg[0] == 0x00))) {
        faac_prog_mode = true;
        // ProgMode ON
        for(uint8_t i = data_prg[1] & 0xF; i != 0; i--) {
            data_tmp = data_prg[2];

            data_prg[2] = data_prg[2] >> 1 | (data_prg[3] & 1) << 7;
            data_prg[3] = data_prg[3] >> 1 | (data_prg[4] & 1) << 7;
            data_prg[4] = data_prg[4] >> 1 | (data_prg[5] & 1) << 7;
            data_prg[5] = data_prg[5] >> 1 | (data_tmp & 1) << 7;
        }
        data_prg[2] ^= data_prg[1];
        data_prg[3] ^= data_prg[1];
        data_prg[4] ^= data_prg[1];
        data_prg[5] ^= data_prg[1];
        instance->seed = data_prg[5] << 24 | data_prg[4] << 16 | data_prg[3] << 8 | data_prg[2];
        uint32_t dec_prg_1 = data_prg[7] << 24 | data_prg[6] << 16 | data_prg[5] << 8 |
                             data_prg[4];
        uint32_t dec_prg_2 = data_prg[3] << 24 | data_prg[2] << 16 | data_prg[1] << 8 |
                             data_prg[0];
        instance->data_2 = (uint64_t)dec_prg_1 << 32 | dec_prg_2;
        instance->cnt = data_prg[1];

        *manufacture_name = "FAAC_SLH";
        return;
    } else {
        if(code_fix != 0x0) {
            temp_fix_backup = code_fix;
            instance->serial = code_fix >> 4;
            instance->btn = code_fix & 0xF;
        }

        faac_prog_mode = false;
    }

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            switch(manufacture_code->type) {
            case KEELOQ_LEARNING_FAAC:
                // FAAC Learning
                man = subghz_protocol_keeloq_common_faac_learning(
                    instance->seed, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(code_hop, man);
                *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                break;
            }
        }
    instance->cnt = decrypt & 0xFFFFF;
    // Backup counter in case when we need to use programming mode
    if(code_fix != 0x0) {
        temp_counter_backup = instance->cnt;
    }
}

uint8_t subghz_protocol_decoder_faac_slh_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_faac_slh_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;

    // Reset seed leftover from previous decoded signal
    instance->generic.seed = 0x0;
    temp_fix_backup = 0x0;

    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t seed_data[sizeof(uint32_t)] = {0};
    for(size_t i = 0; i < sizeof(uint32_t); i++) {
        seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
    }
    if((res == SubGhzProtocolStatusOk) &&
       !flipper_format_write_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
        FURI_LOG_E(TAG, "Unable to add Seed");
        res = SubGhzProtocolStatusError;
    }
    instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                             seed_data[3];

    subghz_protocol_faac_slh_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);

    return res;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_faac_slh_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }

        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
        }
        if(!flipper_format_read_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
            FURI_LOG_E(TAG, "Missing Seed");
            break;
        }
        bool tmp_allow_zero_seed;
        if(flipper_format_read_bool(flipper_format, "AllowZeroSeed", &tmp_allow_zero_seed, 1)) {
            allow_zero_seed = true;
        } else {
            allow_zero_seed = false;
        }
        instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                                 seed_data[3];

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void subghz_protocol_decoder_faac_slh_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    subghz_protocol_faac_slh_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);
    uint32_t code_fix = instance->generic.data >> 32;
    uint32_t code_hop = instance->generic.data & 0xFFFFFFFF;

    if(faac_prog_mode == true) {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Master Remote Prog Mode\r\n"
            "Ke:%lX%08lX\r\n"
            "Kd:%lX%08lX\r\n"
            "Seed:%08lX mCnt:%02X",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            (uint32_t)(instance->generic.data >> 32),
            (uint32_t)instance->generic.data,
            (uint32_t)(instance->generic.data_2 >> 32),
            (uint32_t)instance->generic.data_2,
            instance->generic.seed,
            (uint8_t)(instance->generic.cnt & 0xFF));
    } else if((allow_zero_seed == false) && (instance->generic.seed == 0x0)) {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:%lX%08lX\r\n"
            "Fix:%08lX\r\n"
            "Hop:%08lX    Btn:%X\r\n"
            "Sn:%07lX Sd:Unknown",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            (uint32_t)(instance->generic.data >> 32),
            (uint32_t)instance->generic.data,
            code_fix,
            code_hop,
            instance->generic.btn,
            instance->generic.serial);
    } else {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:%lX%08lX\r\n"
            "Fix:%08lX    Cnt:%05lX\r\n"
            "Hop:%08lX    Btn:%X\r\n"
            "Sn:%07lX Sd:%08lX",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            (uint32_t)(instance->generic.data >> 32),
            (uint32_t)instance->generic.data,
            code_fix,
            instance->generic.cnt,
            code_hop,
            instance->generic.btn,
            instance->generic.serial,
            instance->generic.seed);
    }
}
