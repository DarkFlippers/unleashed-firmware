#include "keeloq.h"
#include "keeloq_common.h"

#include "../subghz_keystore.h"
#include <m-array.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolKeeloq"

static const SubGhzBlockConst subghz_protocol_keeloq_const = {
    .te_short = 400,
    .te_long = 800,
    .te_delta = 140,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderKeeloq {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    SubGhzKeystore* keystore;
    const char* manufacture_name;

    FuriString* manufacture_from_file;
};

struct SubGhzProtocolEncoderKeeloq {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
    const char* manufacture_name;

    FuriString* manufacture_from_file;
};

typedef enum {
    KeeloqDecoderStepReset = 0,
    KeeloqDecoderStepCheckPreambula,
    KeeloqDecoderStepSaveDuration,
    KeeloqDecoderStepCheckDuration,
} KeeloqDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_keeloq_decoder = {
    .alloc = subghz_protocol_decoder_keeloq_alloc,
    .free = subghz_protocol_decoder_keeloq_free,

    .feed = subghz_protocol_decoder_keeloq_feed,
    .reset = subghz_protocol_decoder_keeloq_reset,

    .get_hash_data = subghz_protocol_decoder_keeloq_get_hash_data,
    .serialize = subghz_protocol_decoder_keeloq_serialize,
    .deserialize = subghz_protocol_decoder_keeloq_deserialize,
    .get_string = subghz_protocol_decoder_keeloq_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_keeloq_encoder = {
    .alloc = subghz_protocol_encoder_keeloq_alloc,
    .free = subghz_protocol_encoder_keeloq_free,

    .deserialize = subghz_protocol_encoder_keeloq_deserialize,
    .stop = subghz_protocol_encoder_keeloq_stop,
    .yield = subghz_protocol_encoder_keeloq_yield,
};

const SubGhzProtocol subghz_protocol_keeloq = {
    .name = SUBGHZ_PROTOCOL_KEELOQ_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_keeloq_decoder,
    .encoder = &subghz_protocol_keeloq_encoder,
};

static const char* mfname;
static uint8_t kl_type;
static uint8_t btn_temp_id;
static uint8_t btn_temp_id_original;

void keeloq_set_btn(uint8_t b) {
    btn_temp_id = b;
}

uint8_t keeloq_get_original_btn() {
    return btn_temp_id_original;
}

uint8_t keeloq_get_custom_btn() {
    return btn_temp_id;
}

void keeloq_reset_original_btn() {
    btn_temp_id_original = 0;
}

void keeloq_reset_mfname() {
    mfname = "";
}

void keeloq_reset_kl_type() {
    kl_type = 0;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param keystore Pointer to a SubGhzKeystore* instance
 * @param manufacture_name
 */
static void subghz_protocol_keeloq_check_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore,
    const char** manufacture_name);

void* subghz_protocol_encoder_keeloq_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderKeeloq* instance = malloc(sizeof(SubGhzProtocolEncoderKeeloq));

    instance->base.protocol = &subghz_protocol_keeloq;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    instance->encoder.repeat = 100;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    instance->manufacture_from_file = furi_string_alloc();

    return instance;
}

void subghz_protocol_encoder_keeloq_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderKeeloq* instance = context;
    furi_string_free(instance->manufacture_from_file);
    free(instance->encoder.upload);
    free(instance);
}

/** 
 * Key generation from simple data
 * @param instance Pointer to a SubGhzProtocolEncoderKeeloq* instance
 * @param btn Button number, 4 bit
 */
static bool subghz_protocol_keeloq_gen_data(
    SubGhzProtocolEncoderKeeloq* instance,
    uint8_t btn,
    bool counter_up) {
    if(counter_up) {
        if(instance->generic.cnt < 0xFFFF) {
            if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) >= 0xFFFF) {
                instance->generic.cnt = 0;
            } else {
                instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        } else if(instance->generic.cnt >= 0xFFFF) {
            instance->generic.cnt = 0;
        }
    }
    uint32_t fix = (uint32_t)btn << 28 | instance->generic.serial;
    uint32_t decrypt = (uint32_t)btn << 28 |
                       (instance->generic.serial & 0x3FF)
                           << 16 | //ToDo in some protocols the discriminator is 0
                       instance->generic.cnt;
    uint32_t hop = 0;
    uint64_t man = 0;
    uint64_t code_found_reverse;
    int res = 0;
    if(instance->manufacture_name == 0x0) {
        instance->manufacture_name = "";
    }

    // DTM Neo uses 12bit -> simple learning -- FAAC_RC,XT , Mutanco_Mutancode -> 12bit normal learning
    if((strcmp(instance->manufacture_name, "DTM_Neo") == 0) ||
       (strcmp(instance->manufacture_name, "FAAC_RC,XT") == 0) ||
       (strcmp(instance->manufacture_name, "Mutanco_Mutancode") == 0)) {
        decrypt = btn << 28 | (instance->generic.serial & 0xFFF) << 16 | instance->generic.cnt;
    }

    // Nice Smilo, MHouse, JCM, Normstahl -> 8bit serial - simple learning
    if((strcmp(instance->manufacture_name, "NICE_Smilo") == 0) ||
       (strcmp(instance->manufacture_name, "NICE_MHOUSE") == 0) ||
       (strcmp(instance->manufacture_name, "JCM_Tech") == 0) ||
       (strcmp(instance->manufacture_name, "Normstahl") == 0)) {
        decrypt = btn << 28 | (instance->generic.serial & 0xFF) << 16 | instance->generic.cnt;
    }

    // Beninca -> 4bit serial - simple XOR
    if(strcmp(instance->manufacture_name, "Beninca") == 0) {
        decrypt = btn << 28 | (instance->generic.serial & 0xF) << 16 | instance->generic.cnt;
    }

    if(strcmp(instance->manufacture_name, "Unknown") == 0) {
        code_found_reverse = subghz_protocol_blocks_reverse_key(
            instance->generic.data, instance->generic.data_count_bit);
        hop = code_found_reverse & 0x00000000ffffffff;
    } else if(strcmp(instance->manufacture_name, "AN-Motors") == 0) {
        hop = (instance->generic.cnt & 0xFF) << 24 | (instance->generic.cnt & 0xFF) << 16 |
              (btn & 0xF) << 12 | 0x404;
    } else if(strcmp(instance->manufacture_name, "HCS101") == 0) {
        hop = instance->generic.cnt << 16 | (btn & 0xF) << 12 | 0x000;
    } else {
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            res = strcmp(furi_string_get_cstr(manufacture_code->name), instance->manufacture_name);
            if(res == 0) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_SIMPLE:
                    //Simple Learning
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, manufacture_code->key);
                    break;
                case KEELOQ_LEARNING_NORMAL:
                    //Simple Learning
                    man =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    break;
                case KEELOQ_LEARNING_SECURE:
                    //Secure Learning
                    man = subghz_protocol_keeloq_common_secure_learning(
                        fix, instance->generic.seed, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    break;
                case KEELOQ_LEARNING_MAGIC_XOR_TYPE_1:
                    //Magic XOR type-1 Learning
                    man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                        instance->generic.serial, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    break;
                case KEELOQ_LEARNING_MAGIC_SERIAL_TYPE_1:
                    //Magic Serial Type 1 learning
                    man = subghz_protocol_keeloq_common_magic_serial_type1_learning(
                        fix, manufacture_code->key);
                    hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    break;
                case KEELOQ_LEARNING_UNKNOWN:
                    if(kl_type == 1) {
                        hop =
                            subghz_protocol_keeloq_common_encrypt(decrypt, manufacture_code->key);
                    }
                    if(kl_type == 2) {
                        man = subghz_protocol_keeloq_common_normal_learning(
                            fix, manufacture_code->key);
                        hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    }
                    if(kl_type == 3) {
                        man = subghz_protocol_keeloq_common_secure_learning(
                            fix, instance->generic.seed, manufacture_code->key);
                        hop = subghz_protocol_keeloq_common_encrypt(decrypt, man);
                    }
                    if(kl_type == 4) {
                        man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                            instance->generic.serial, manufacture_code->key);
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
    }
    return true;
}

bool subghz_protocol_keeloq_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderKeeloq* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->manufacture_name = manufacture_name;
    instance->generic.data_count_bit = 64;
    bool res = subghz_protocol_keeloq_gen_data(instance, btn, false);
    if(res) {
        res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

bool subghz_protocol_keeloq_bft_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderKeeloq* instance = context;
    instance->generic.serial = serial;
    instance->generic.btn = btn;
    instance->generic.cnt = cnt;
    instance->generic.seed = seed;
    instance->manufacture_name = manufacture_name;
    instance->generic.data_count_bit = 64;
    // roguuemaster don't steal.!!!!
    bool res = subghz_protocol_keeloq_gen_data(instance, btn, false);
    if(res) {
        res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_keeloq_get_upload(SubGhzProtocolEncoderKeeloq* instance, uint8_t btn) {
    furi_assert(instance);

    // Save original button
    if(btn_temp_id_original == 0) {
        btn_temp_id_original = btn;
    }

    // Set custom button
    if(btn_temp_id == 1) {
        switch(btn_temp_id_original) {
        case 0x1:
            btn = 0x2;
            break;
        case 0x2:
            btn = 0x1;
            break;
        case 0xA:
            btn = 0x1;
            break;
        case 0x4:
            btn = 0x1;
            break;
        case 0x8:
            btn = 0x1;
            break;

        default:
            break;
        }
    }
    if(btn_temp_id == 2) {
        switch(btn_temp_id_original) {
        case 0x1:
            btn = 0x4;
            break;
        case 0x2:
            btn = 0x4;
            break;
        case 0xA:
            btn = 0x4;
            break;
        case 0x4:
            btn = 0xA;
            break;
        case 0x8:
            btn = 0x4;
            break;

        default:
            break;
        }
    }
    if(btn_temp_id == 3) {
        switch(btn_temp_id_original) {
        case 0x1:
            btn = 0x8;
            break;
        case 0x2:
            btn = 0x8;
            break;
        case 0xA:
            btn = 0x8;
            break;
        case 0x4:
            btn = 0x8;
            break;
        case 0x8:
            btn = 0x2;
            break;

        default:
            break;
        }
    }
    if(btn_temp_id == 4) {
        switch(btn_temp_id_original) {
        case 0x1:
            btn = 0xA;
            break;
        case 0x2:
            btn = 0xA;
            break;
        case 0xA:
            btn = 0x2;
            break;
        case 0x4:
            btn = 0x2;
            break;
        case 0x8:
            btn = 0xA;
            break;

        default:
            break;
        }
    }

    if((btn_temp_id == 0) && (btn_temp_id_original != 0)) {
        btn = btn_temp_id_original;
    }

    //gen new key
    if(subghz_protocol_keeloq_gen_data(instance, btn, true)) {
        //ToDo if you need to add a callback to automatically update the data on the display
    } else {
        return false;
    }

    size_t index = 0;
    size_t size_upload = 11 * 2 + 2 + (instance->generic.data_count_bit * 2) + 4;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 11; i > 0; i--) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_short);
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_short * 10);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_short);
        }
    }
    // +send 2 status bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_long);
    // send end
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_keeloq_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_keeloq_const.te_short * 40);

    return true;
}

bool subghz_protocol_encoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderKeeloq* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_keeloq_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }

        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
        }
        if(!flipper_format_read_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
            FURI_LOG_D(TAG, "ENCODER: Missing Seed");
        }
        instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                                 seed_data[3];

        // Read manufacturer from file
        if(flipper_format_read_string(
               flipper_format, "Manufacture", instance->manufacture_from_file)) {
            instance->manufacture_name = furi_string_get_cstr(instance->manufacture_from_file);
            mfname = furi_string_get_cstr(instance->manufacture_from_file);
        } else {
            FURI_LOG_D(TAG, "ENCODER: Missing Manufacture");
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        subghz_protocol_keeloq_check_remote_controller(
            &instance->generic, instance->keystore, &instance->manufacture_name);

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_keeloq_get_upload(instance, instance->generic.btn)) break;

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_keeloq_stop(void* context) {
    SubGhzProtocolEncoderKeeloq* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_keeloq_yield(void* context) {
    SubGhzProtocolEncoderKeeloq* instance = context;

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

void* subghz_protocol_decoder_keeloq_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderKeeloq* instance = malloc(sizeof(SubGhzProtocolDecoderKeeloq));
    instance->base.protocol = &subghz_protocol_keeloq;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);
    instance->manufacture_from_file = furi_string_alloc();

    return instance;
}

void subghz_protocol_decoder_keeloq_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;
    furi_string_free(instance->manufacture_from_file);

    free(instance);
}

void subghz_protocol_decoder_keeloq_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;
    instance->decoder.parser_step = KeeloqDecoderStepReset;
    mfname = "";
    kl_type = 0;
}

void subghz_protocol_decoder_keeloq_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;

    switch(instance->decoder.parser_step) {
    case KeeloqDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_keeloq_const.te_short) <
                          subghz_protocol_keeloq_const.te_delta) {
            instance->decoder.parser_step = KeeloqDecoderStepCheckPreambula;
            instance->header_count++;
        }
        break;
    case KeeloqDecoderStepCheckPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_keeloq_const.te_short) <
                        subghz_protocol_keeloq_const.te_delta)) {
            instance->decoder.parser_step = KeeloqDecoderStepReset;
            break;
        }
        if((instance->header_count > 2) &&
           (DURATION_DIFF(duration, subghz_protocol_keeloq_const.te_short * 10) <
            subghz_protocol_keeloq_const.te_delta * 10)) {
            // Found header
            instance->decoder.parser_step = KeeloqDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = KeeloqDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    case KeeloqDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = KeeloqDecoderStepCheckDuration;
        }
        break;
    case KeeloqDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_keeloq_const.te_short * 2 +
                            subghz_protocol_keeloq_const.te_delta)) {
                // Found end TX
                instance->decoder.parser_step = KeeloqDecoderStepReset;
                if((instance->decoder.decode_count_bit >=
                    subghz_protocol_keeloq_const.min_count_bit_for_found) &&
                   (instance->decoder.decode_count_bit <=
                    subghz_protocol_keeloq_const.min_count_bit_for_found + 2)) {
                    if(instance->generic.data != instance->decoder.decode_data) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit =
                            subghz_protocol_keeloq_const.min_count_bit_for_found;
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                    instance->header_count = 0;
                }
                break;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_keeloq_const.te_short) <
                 subghz_protocol_keeloq_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_keeloq_const.te_long) <
                 subghz_protocol_keeloq_const.te_delta * 2)) {
                if(instance->decoder.decode_count_bit <
                   subghz_protocol_keeloq_const.min_count_bit_for_found) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else {
                    instance->decoder.decode_count_bit++;
                }
                instance->decoder.parser_step = KeeloqDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_keeloq_const.te_long) <
                 subghz_protocol_keeloq_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_keeloq_const.te_short) <
                 subghz_protocol_keeloq_const.te_delta)) {
                if(instance->decoder.decode_count_bit <
                   subghz_protocol_keeloq_const.min_count_bit_for_found) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else {
                    instance->decoder.decode_count_bit++;
                }
                instance->decoder.parser_step = KeeloqDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = KeeloqDecoderStepReset;
                instance->header_count = 0;
            }
        } else {
            instance->decoder.parser_step = KeeloqDecoderStepReset;
            instance->header_count = 0;
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
static inline bool subghz_protocol_keeloq_check_decrypt(
    SubGhzBlockGeneric* instance,
    uint32_t decrypt,
    uint8_t btn,
    uint32_t end_serial) {
    furi_assert(instance);
    if((decrypt >> 28 == btn) && (((((uint16_t)(decrypt >> 16)) & 0xFF) == end_serial) ||
                                  ((((uint16_t)(decrypt >> 16)) & 0xFF) == 0))) {
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
static uint8_t subghz_protocol_keeloq_check_remote_controller_selector(
    SubGhzBlockGeneric* instance,
    uint32_t fix,
    uint32_t hop,
    SubGhzKeystore* keystore,
    const char** manufacture_name) {
    // protocol HCS300 uses 10 bits in discriminator, HCS200 uses 8 bits, for backward compatibility, we are looking for the 8-bit pattern
    // HCS300 -> uint16_t end_serial = (uint16_t)(fix & 0x3FF);
    // HCS200 -> uint16_t end_serial = (uint16_t)(fix & 0xFF);

    uint16_t end_serial = (uint16_t)(fix & 0xFF);
    uint8_t btn = (uint8_t)(fix >> 28);
    uint32_t decrypt = 0;
    uint64_t man;
    int res = 0;
    if(mfname == 0x0) {
        mfname = "";
    }

    if(strcmp(mfname, "") == 0) {
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            switch(manufacture_code->type) {
            case KEELOQ_LEARNING_SIMPLE:
                // Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_NORMAL:
                // Normal Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man = subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_SECURE:
                man = subghz_protocol_keeloq_common_secure_learning(
                    fix, instance->seed, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_MAGIC_XOR_TYPE_1:
                man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                    fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_MAGIC_SERIAL_TYPE_1:
                man = subghz_protocol_keeloq_common_magic_serial_type1_learning(
                    fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_MAGIC_SERIAL_TYPE_2:
                man = subghz_protocol_keeloq_common_magic_serial_type2_learning(
                    fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_MAGIC_SERIAL_TYPE_3:
                man = subghz_protocol_keeloq_common_magic_serial_type3_learning(
                    fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_UNKNOWN:
                // Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 1;
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
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 1;
                    return 1;
                }

                //###########################
                // Normal Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man = subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 2;
                    return 1;
                }

                // Check for mirrored man
                man = subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 2;
                    return 1;
                }

                // Secure Learning
                man = subghz_protocol_keeloq_common_secure_learning(
                    fix, instance->seed, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 3;
                    return 1;
                }

                // Check for mirrored man
                man = subghz_protocol_keeloq_common_secure_learning(fix, instance->seed, man_rev);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 3;
                    return 1;
                }

                // Magic xor type1 learning
                man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                    fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 4;
                    return 1;
                }

                // Check for mirrored man
                man = subghz_protocol_keeloq_common_magic_xor_type1_learning(fix, man_rev);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                    mfname = *manufacture_name;
                    kl_type = 4;
                    return 1;
                }

                break;
            }
        }
    } else if(strcmp(mfname, "Unknown") == 0) {
        return 1;
    } else {
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            res = strcmp(furi_string_get_cstr(manufacture_code->name), mfname);
            if(res == 0) {
                switch(manufacture_code->type) {
                case KEELOQ_LEARNING_SIMPLE:
                    // Simple Learning
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_NORMAL:
                    // Normal Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_SECURE:
                    man = subghz_protocol_keeloq_common_secure_learning(
                        fix, instance->seed, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_MAGIC_XOR_TYPE_1:
                    man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                        fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_MAGIC_SERIAL_TYPE_1:
                    man = subghz_protocol_keeloq_common_magic_serial_type1_learning(
                        fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        return 1;
                    }
                    break;
                case KEELOQ_LEARNING_UNKNOWN:
                    // Simple Learning
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 1;
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
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 1;
                        return 1;
                    }
                    //###########################
                    // Normal Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man =
                        subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 2;
                        return 1;
                    }

                    // Check for mirrored man
                    man = subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 2;
                        return 1;
                    }

                    // Secure Learning
                    man = subghz_protocol_keeloq_common_secure_learning(
                        fix, instance->seed, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 3;
                        return 1;
                    }

                    // Check for mirrored man
                    man = subghz_protocol_keeloq_common_secure_learning(
                        fix, instance->seed, man_rev);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 3;
                        return 1;
                    }

                    // Magic xor type1 learning
                    man = subghz_protocol_keeloq_common_magic_xor_type1_learning(
                        fix, manufacture_code->key);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 4;
                        return 1;
                    }

                    // Check for mirrored man
                    man = subghz_protocol_keeloq_common_magic_xor_type1_learning(fix, man_rev);
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, man);
                    if(subghz_protocol_keeloq_check_decrypt(instance, decrypt, btn, end_serial)) {
                        *manufacture_name = furi_string_get_cstr(manufacture_code->name);
                        mfname = *manufacture_name;
                        kl_type = 4;
                        return 1;
                    }

                    break;
                }
            }
        }
    }

    *manufacture_name = "Unknown";
    mfname = "Unknown";
    instance->cnt = 0;

    return 0;
}

static void subghz_protocol_keeloq_check_remote_controller(
    SubGhzBlockGeneric* instance,
    SubGhzKeystore* keystore,
    const char** manufacture_name) {
    uint64_t key = subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit);
    uint32_t key_fix = key >> 32;
    uint32_t key_hop = key & 0x00000000ffffffff;
    // Check key AN-Motors
    if((key_hop >> 24) == ((key_hop >> 16) & 0x00ff) &&
       (key_fix >> 28) == ((key_hop >> 12) & 0x0f) && (key_hop & 0xFFF) == 0x404) {
        *manufacture_name = "AN-Motors";
        mfname = *manufacture_name;
        instance->cnt = key_hop >> 16;
    } else if((key_hop & 0xFFF) == (0x000) && (key_fix >> 28) == ((key_hop >> 12) & 0x0f)) {
        *manufacture_name = "HCS101";
        mfname = *manufacture_name;
        instance->cnt = key_hop >> 16;
    } else {
        subghz_protocol_keeloq_check_remote_controller_selector(
            instance, key_fix, key_hop, keystore, manufacture_name);
    }

    instance->serial = key_fix & 0x0FFFFFFF;
    instance->btn = key_fix >> 28;

    // Save original button for later use
    if(btn_temp_id_original == 0) {
        btn_temp_id_original = instance->btn;
    }
}

uint8_t subghz_protocol_decoder_keeloq_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_keeloq_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;

    bool res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    subghz_protocol_keeloq_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);

    if(strcmp(instance->manufacture_name, "BFT") == 0) {
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
        }
        if(res && !flipper_format_write_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
            FURI_LOG_E(TAG, "DECODER Serialize: Unable to add Seed");
            res = false;
        }
        instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                                 seed_data[3];
    }

    if(res && !flipper_format_write_string_cstr(
                  flipper_format, "Manufacture", instance->manufacture_name)) {
        FURI_LOG_E(TAG, "DECODER Serialize: Unable to add manufacture name");
        res = false;
    }
    return res;
}

bool subghz_protocol_decoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_keeloq_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }

        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (instance->generic.seed >> i * 8) & 0xFF;
        }
        if(!flipper_format_read_hex(flipper_format, "Seed", seed_data, sizeof(uint32_t))) {
            FURI_LOG_D(TAG, "DECODER: Missing Seed");
        }
        instance->generic.seed = seed_data[0] << 24 | seed_data[1] << 16 | seed_data[2] << 8 |
                                 seed_data[3];

        // Read manufacturer from file
        if(flipper_format_read_string(
               flipper_format, "Manufacture", instance->manufacture_from_file)) {
            instance->manufacture_name = furi_string_get_cstr(instance->manufacture_from_file);
            mfname = furi_string_get_cstr(instance->manufacture_from_file);
        } else {
            FURI_LOG_D(TAG, "DECODER: Missing Manufacture");
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_decoder_keeloq_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderKeeloq* instance = context;

    subghz_protocol_keeloq_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);
    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    if(strcmp(instance->manufacture_name, "BFT") == 0) {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:%08lX%08lX\r\n"
            "Fix:0x%08lX    Cnt:%04lX\r\n"
            "Hop:0x%08lX    Btn:%01X\r\n"
            "MF:%s Sd:%08lX",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            code_found_hi,
            code_found_lo,
            code_found_reverse_hi,
            instance->generic.cnt,
            code_found_reverse_lo,
            instance->generic.btn,
            instance->manufacture_name,
            instance->generic.seed);
    } else {
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:%08lX%08lX\r\n"
            "Fix:0x%08lX    Cnt:%04lX\r\n"
            "Hop:0x%08lX    Btn:%01X\r\n"
            "MF:%s",
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
}
