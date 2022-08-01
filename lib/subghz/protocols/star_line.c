#include "star_line.h"
#include "keeloq_common.h"

#include "../subghz_keystore.h"
#include <m-string.h>
#include <m-array.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

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
};

struct SubGhzProtocolEncoderStarLine {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
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
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_star_line = {
    .name = SUBGHZ_PROTOCOL_STAR_LINE_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_star_line_decoder,
    .encoder = &subghz_protocol_star_line_encoder,
};

void* subghz_protocol_decoder_star_line_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderStarLine* instance = malloc(sizeof(SubGhzProtocolDecoderStarLine));
    instance->base.protocol = &subghz_protocol_star_line;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    return instance;
}

void subghz_protocol_decoder_star_line_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;

    free(instance);
}

void subghz_protocol_decoder_star_line_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    instance->decoder.parser_step = StarLineDecoderStepReset;
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
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_star_line_const.min_count_bit_for_found) {
                    if(instance->generic.data != instance->decoder.decode_data) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
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
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = StarLineDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_star_line_const.te_long) <
                 subghz_protocol_star_line_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_star_line_const.te_long) <
                 subghz_protocol_star_line_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
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

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            switch(manufacture_code->type) {
            case KEELOQ_LEARNING_SIMPLE:
                //Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_NORMAL:
                // Normal_Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man_normal_learning =
                    subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
                    return 1;
                }
                break;
            case KEELOQ_LEARNING_UNKNOWN:
                // Simple Learning
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
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
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
                    return 1;
                }
                //###########################
                // Normal_Learning
                // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                man_normal_learning =
                    subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
                    return 1;
                }
                man_normal_learning = subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                decrypt = subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                if(subghz_protocol_star_line_check_decrypt(instance, decrypt, btn, end_serial)) {
                    *manufacture_name = string_get_cstr(manufacture_code->name);
                    return 1;
                }
                break;
            }
        }

    *manufacture_name = "Unknown";
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

bool subghz_protocol_decoder_star_line_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    subghz_protocol_star_line_check_remote_controller(
        &instance->generic, instance->keystore, &instance->manufacture_name);
    bool res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    if(res && !flipper_format_write_string_cstr(
                  flipper_format, "Manufacture", instance->manufacture_name)) {
        FURI_LOG_E(TAG, "Unable to add manufacture name");
        res = false;
    }
    if(res && instance->generic.data_count_bit !=
                  subghz_protocol_star_line_const.min_count_bit_for_found) {
        FURI_LOG_E(TAG, "Wrong number of bits in key");
        res = false;
    }
    return res;
}

bool subghz_protocol_decoder_star_line_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderStarLine* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        res = true;
    } while(false);

    return res;
}

void subghz_protocol_decoder_star_line_get_string(void* context, string_t output) {
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

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Fix:0x%08lX    Cnt:%04X\r\n"
        "Hop:0x%08lX    Btn:%02lX\r\n"
        "MF:%s\r\n"
        "Sn:0x%07lX \r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        instance->generic.cnt,
        code_found_reverse_lo,
        instance->generic.btn,
        instance->manufacture_name,
        instance->generic.serial);
}
