#include "star_line.h"
#include "keeloq_common.h"

#include "../subghz_keystore.h"
#include <m-array.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../subghz_keystore_i.h"

#define TAG "SubGhzProtocolStarLine"

static const SubGhzBlockConst subghz_protocol_star_line_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 120,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderStarLine {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    SubGhzKeystore* keystore;
    const char* manufacture_name;

    FuriString* manufacture_from_file;
};

struct SubGhzProtocolEncoderStarLine {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
    const char* manufacture_name;

    FuriString* manufacture_from_file;
};

typedef enum {
    StarLineDecoderStepReset = 0,
    StarLineDecoderStepCheckPreambula,
    StarLineDecoderStepSaveDuration,
    StarLineDecoderStepCheckDuration,
} StarLineDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_star_line_decoder = {
    .alloc = subghz_protocol_decoder_star_line_alloc,
    .free = subghz_protocol_decoder_star_line_free,

    .feed = subghz_protocol_decoder_star_line_feed,
    .reset = subghz_protocol_decoder_star_line_reset,

    .get_hash_data = subghz_protocol_decoder_star_line_get_hash_data,
    .serialize = subghz_protocol_decoder_star_line_serialize,
    .deserialize = subghz_protocol_decoder_star_line_deserialize,
    .get_string = subghz_protocol_decoder_star_line_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_star_line_encoder = {
    .alloc = subghz_protocol_encoder_star_line_alloc,
    .free = subghz_protocol_encoder_star_line_free,

    .deserialize = subghz_protocol_encoder_star_line_deserialize,
    .stop = subghz_protocol_encoder_star_line_stop,
    .yield = subghz_protocol_encoder_star_line_yield,
};

const SubGhzProtocol subghz_protocol_star_line = {
    .name = SUBGHZ_PROTOCOL_STAR_LINE_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send |
            SubGhzProtocolFlag_StarLine,

    .decoder = &subghz_protocol_star_line_decoder,
    .encoder = &subghz_protocol_star_line_encoder,
};

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manufacture_name
 */
static void subghz_protocol_star_line_check_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore,
    const char** manufacture_name);

void* subghz_protocol_encoder_star_line_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderStarLine* instance = malloc(sizeof(SubGhzProtocolEncoderStarLine));

    instance->base.protocol = &subghz_protocol_star_line;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    instance->manufacture_from_file = furi_string_alloc();

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    return instance;
}

void subghz_protocol_encoder_star_line_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderStarLine* instance = context;
    furi_string_free(instance->manufacture_from_file);
    free(instance->encoder.upload);
    free(instance);
}

/** 
 * Key generation from simple data
 * @param instance Pointer to a SubGhzProtocolEncoderKeeloq* instance
 * @param btn Button number, 4 bit
 */
static bool
    subghz_protocol_star_line_gen_data(SubGhzProtocolEncoderStarLine* instance, uint8_t btn) {
    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) >= 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }
    uint32_t fix = btn << 24 | instance->generic.serial;
    uint32_t decrypt = btn << 24 | (instance->generic.serial & 0xFF) << 16 | instance->generic.cnt;
    uint32_t hop = 0;
    uint64_t man = 0;
    uint64_t code_found_reverse;
    int res = 0;

    if(instance->manufacture_name == 0x0) {
        instance->manufacture_name = "";
    }

    if(strcmp(instance->manufacture_name, "Unknown") == 0) {
        code_found_reverse = subghz_protocol_blocks_reverse_key(
            instance->generic.data, instance->generic.data_count_bit);
        hop = code_found_reverse & 0x00000000ffffffff;
    } else {
        uint8_t kl_type_en = instance->keystore->kl_type;
        for
            M_EACH(
                manufacture_code,
                *subghz_keystore_get_data(instance->keystore),
                SubGhzKeyArray_t) {
                res = strcmp(
                    furi_string_get_cstr(manufacture_code->name), instance->manufacture_name);
                if(res == 0) {
                    switch(manufacture_code->type) {
                    case KEELOQ_LEARNING_SIMPLE:
                        //Simple Learning
                        hop =
                            subghz_protocol_keeloq_common_encrypt(decrypt, manufacture_code->key);
                        break;
                    case KEELOQ_LEARNING_NORMAL:
                        //Normal Learning
                        man = subghz_protocol_keeloq_common_normal_learning(
                            fix, manufacture_code->key);
                        hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                        break;
                    case KEELOQ_LEARNING_UNKNOWN:
                        if(kl_type_en == 1) {
                            hop = subghz_protocol_keeloq_common_encrypt(
                                decrypt, manufacture_code->key);
                        }
                        if(kl_type_en == 2) {
                            man = subghz_protocol_keeloq_common_normal_learning(
                                fix, manufacture_code->key);
                            hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                        }
                        break;
                    }
                    break;
                }
            }
    }
    if(hop) {
        uint64_t yek = (uint64_t)fix << 32 | hop;
        instance->generic.data =
            subghz_protocol_blocks_reverse_key(yek, instance->generic.data_count_bit);
        return true;
    } else {
        instance->manufacture_name = "Unknown";
        return false;
    }
}

bool subghz_protocol_star_line_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderStarLine* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->manufacture_name = manufacture_name;
    instance->generic.data_count_bit = 64;
    bool res = subghz_protocol_star_line_gen_data(instance, btn);
    if(res) {
        return SubGhzProtocolStatusOk ==
               subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @return true On success
 */
static bool subghz_protocol_encoder_star_line_get_upload(
    SubGhzProtocolEncoderStarLine* instance,
    uint8_t btn) {
    furi_assert(instance);

    // Gen new key
    if(!subghz_protocol_star_line_gen_data(instance, btn)) {
        return false;
    }

    size_t index = 0;
    size_t size_upload = 6 * 2 + (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 6; i > 0; i--) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_star_line_const.te_long * 2);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_star_line_const.te_long * 2);
    }

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_star_line_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_star_line_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_star_line_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_star_line_const.te_short);
        }
    }

    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_star_line_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderStarLine* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        // Read manufacturer from file
        if(flipper_format_read_string(
               flipper_format, "Manufacture", instance->manufacture_from_file)) {
            instance->manufacture_name = furi_string_get_cstr(instance->manufacture_from_file);
            instance->keystore->mfname = instance->manufacture_name;
        } else {
            FURI_LOG_D(TAG, "ENCODER: Missing Manufacture");
        }

        subghz_protocol_star_line_check_remote_controller(
            &instance->generic, instance->keystore, &instance->manufacture_name);

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_star_line_get_upload(instance, instance->generic.btn);

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

void subghz_protocol_encoder_star_line_stop(void* context) {
    SubGhzProtocolEncoderStarLine* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_star_line_yield(void* context) {
    SubGhzProtocolEncoderStarLine* instance = context;

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

void* subghz_protocol_decoder_star_line_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderStarLine* instance = malloc(sizeof(SubGhzProtocolDecoderStarLine));
    instance->base.protocol = &subghz_protocol_star_line;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->manufacture_from_file = furi_string_alloc();

    instance->keystore = subghz_environment_get_keystore(environment);

    return instance;
}

void subghz_protocol_decoder_star_line_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    furi_string_free(instance->manufacture_from_file);

    free(instance);
}

void subghz_protocol_decoder_star_line_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    instance->decoder.parser_step = StarLineDecoderStepReset;
    // TODO
    instance->keystore->mfname = "";
    instance->keystore->kl_type = 0;
}

void subghz_protocol_decoder_star_line_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;

    switch(instance->decoder.parser_step) {
    case StarLineDecoderStepReset:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_star_line_const.te_long * 2) <
               subghz_protocol_star_line_const.te_delta * 2) {
                instance->decoder.parser_step = StarLineDecoderStepCheckPreambula;
                instance->header_count++;
            } else if(instance->header_count > 4) {
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = StarLineDecoderStepCheckDuration;
            }
        } else {
            instance->header_count = 0;
        }
        break;
    case StarLineDecoderStepCheckPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_star_line_const.te_long * 2) <
                        subghz_protocol_star_line_const.te_delta * 2)) {
            //Found Preambula
            instance->decoder.parser_step = StarLineDecoderStepReset;
        } else {
            instance->header_count = 0;
            instance->decoder.parser_step = StarLineDecoderStepReset;
        }
        break;
    case StarLineDecoderStepSaveDuration:
        if(level) {
            if(duration >= (subghz_protocol_star_line_const.te_long +
                            subghz_protocol_star_line_const.te_delta)) {
                instance->decoder.parser_step = StarLineDecoderStepReset;
                if((instance->decoder.decode_count_bit >=
                    subghz_protocol_star_line_const.min_count_bit_for_found) &&
                   (instance->decoder.decode_count_bit <=
                    subghz_protocol_star_line_const.min_count_bit_for_found + 2)) {
                    if(instance->generic.data != instance->decoder.decode_data) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit =
                            subghz_protocol_star_line_const.min_count_bit_for_found;
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->header_count = 0;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = StarLineDecoderStepCheckDuration;
            }

        } else {
            instance->decoder.parser_step = StarLineDecoderStepReset;
        }
        break;
    case StarLineDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_star_line_const.te_short) <
                subghz_protocol_star_line_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_star_line_const.te_short) <
                subghz_protocol_star_line_const.te_delta)) {
                if(instance->decoder.decode_count_bit <
                   subghz_protocol_star_line_const.min_count_bit_for_found) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else {
                    instance->decoder.decode_count_bit++;
                }
                instance->decoder.parser_step = StarLineDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_star_line_const.te_long) <
                 subghz_protocol_star_line_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_star_line_const.te_long) <
                 subghz_protocol_star_line_const.te_delta)) {
                if(instance->decoder.decode_count_bit <
                   subghz_protocol_star_line_const.min_count_bit_for_found) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else {
                    instance->decoder.decode_count_bit++;
                }
                instance->decoder.parser_step = StarLineDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = StarLineDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = StarLineDecoderStepReset;
        }
        break;
    }
}

/**
 * Validation of decrypt data.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param decrypt Decrypd data
 * @param btn Button number, 4 bit
 * @param end_serial decrement the last 10 bits of the serial number
 * @return true On success
 */
static inline bool subghz_protocol_star_line_check_decrypt(
    SubGhzBlockGeneric* instance,
    uint32_t decrypt,
    uint8_t btn,
    uint32_t end_serial) {
    furi_assert(instance);
    if((decrypt >> 24 == btn) && ((((uint16_t)(decrypt >> 16)) & 0x00FF) == end_serial)) {
        instance->cnt = decrypt & 0x0000FFFF;
        return true;
    }
    return false;
}

/** 
 * Checking the accepted code against the database manafacture key
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param fix Fix part of the parcel
 * @param hop Hop encrypted part of the parcel
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manufacture_name 
 * @return true on successful search
 */
static uint8_t subghz_protocol_star_line_check_remote_controller_selector(
    SubGhzBlockGeneric* instance,
    uint32_t fix,
    uint32_t hop,
    SubGhzKeystore* keystore,
    const char** manufacture_name) {
    uint16_t end_serial = (uint16_t)(fix & 0xFF);
    uint8_t btn = (uint8_t)(fix >> 24);
    uint32_t decrypt = 0;
    uint64_t man_normal_learning;
    bool mf_not_set = false;
    // TODO:
    // if(mfname == 0x0) {
    //     mfname = "";
    // }

    const char* mfname = keystore->mfname;

    if(strcmp(mfname, "Unknown") == 0) {
        return 1;
    } else if(strcmp(mfname, "") == 0) {
        mf_not_set = true;
    }
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            if(mf_not_set || (strcmp(furi_string_get_cstr(manufacture_code->name), mfname) == 0)) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_SIMPLE:
                    // Simple Learning
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_NORMAL:
                    // Normal Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_UNKNOWN:
                    // Simple Learning
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        keystore->kl_type = 1;
                        return 1;
                    }
                    // Check for mirrored man
                    uint64_t man_rev = 0;
                    uint64_t man_rev_byte = 0;
                    for(uint8_t i = 0; i < 64; i += 8) {
                        man_rev_byte = (uint8_t)(manufacture_code->key >> i);
                        man_rev = man_rev | man_rev_byte << (56 - i);
                    }
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_rev);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        keystore->kl_type = 1;
                        return 1;
                    }
                    //###########################
                    // Normal Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        keystore->kl_type = 2;
                        return 1;
                    }
                    // Check for mirrored man
                    man_normal_learning =
                        subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if(subghz_protocol_star_line_check_decrypt(
                           instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        keystore->mfname = *manufacture_name;
                        keystore->kl_type = 2;
                        return 1;
                    }
                    break;
                }
            }
        }

    *manufacture_name = "Unknown";
    keystore->mfname = "Unknown";
    instance->cnt = 0;

    return 0;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manufacture_name
 */
static void subghz_protocol_star_line_check_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore,
    const char** manufacture_name) {
    uint64_t key = subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit);
    uint32_t key_fix = key >> 32;
    uint32_t key_hop = key & 0x00000000ffffffff;

    subghz_protocol_star_line_check_remote_controller_selector(
        instance, key_fix, key_hop, keystore, manufacture_name);

    instance->serial = key_fix & 0x00FFFFFF;
    instance->btn = key_fix >> 24;
}

uint8_t subghz_protocol_decoder_star_line_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_star_line_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    subghz_protocol_star_line_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_string_cstr(
           flipper_format, "Manufacture", instance->manufacture_name)) {
        FURI_LOG_E(TAG, "Unable to add manufacture name");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    if((ret == SubGhzProtocolStatusOk) &&
       instance->generic.data_count_bit !=
           subghz_protocol_star_line_const.min_count_bit_for_found) {
        FURI_LOG_E(TAG, "Wrong number of bits in key");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_star_line_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        // Read manufacturer from file
        if(flipper_format_read_string(
               flipper_format, "Manufacture", instance->manufacture_from_file)) {
            instance->manufacture_name = furi_string_get_cstr(instance->manufacture_from_file);
            instance->keystore->mfname = instance->manufacture_name;
        } else {
            FURI_LOG_D(TAG, "DECODER: Missing Manufacture");
        }

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void subghz_protocol_decoder_star_line_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;

    subghz_protocol_star_line_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);
    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Fix:0x%08lX    Cnt:%04lX\r\n"
        "Hop:0x%08lX    Btn:%02X\r\n"
        "MF:%s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        instance->generic.cnt,
        code_found_reverse_lo,
        instance->generic.btn,
        instance->manufacture_name);
}
