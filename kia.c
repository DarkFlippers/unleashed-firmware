#include "kia.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocoKia"

static const SubGhzBlockConst subghz_protocol_kia_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 100,
    .min_count_bit_for_found = 61,
};

struct SubGhzProtocolDecoderKIA {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
};

struct SubGhzProtocolEncoderKIA {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    KIADecoderStepReset = 0,
    KIADecoderStepCheckPreambula,
    KIADecoderStepSaveDuration,
    KIADecoderStepCheckDuration,
} KIADecoderStep;

const SubGhzProtocolDecoder subghz_protocol_kia_decoder = {
    .alloc = subghz_protocol_decoder_kia_alloc,
    .free = subghz_protocol_decoder_kia_free,

    .feed = subghz_protocol_decoder_kia_feed,
    .reset = subghz_protocol_decoder_kia_reset,

    .get_hash_data = NULL,
    .get_hash_data_long = subghz_protocol_decoder_kia_get_hash_data,
    .serialize = subghz_protocol_decoder_kia_serialize,
    .deserialize = subghz_protocol_decoder_kia_deserialize,
    .get_string = subghz_protocol_decoder_kia_get_string,
    .get_string_brief = NULL,
};

const SubGhzProtocolEncoder subghz_protocol_kia_encoder = {
    .alloc = subghz_protocol_encoder_kia_alloc,
    .free = subghz_protocol_encoder_kia_free,

    .deserialize = subghz_protocol_encoder_kia_deserialize,
    .stop = subghz_protocol_encoder_kia_stop,
    .yield = subghz_protocol_encoder_kia_yield,
};

void* subghz_protocol_encoder_kia_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderKIA* instance = malloc(sizeof(SubGhzProtocolDecoderKIA));
    instance->base.protocol = &subghz_protocol_kia;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->encoder.size_upload = 848;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.repeat = 1;
    instance->encoder.is_running = false;

    return instance;
}

/**
 * Free SubGhzProtocolEncoderKIA.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 */
void subghz_protocol_encoder_kia_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderKIA* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderKIA instance
 */
void subghz_protocol_encoder_kia_stop(void* context) {
    SubGhzProtocolEncoderKIA* instance = context;
    instance->encoder.is_running = false;
}

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderKIA instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_kia_yield(void* context) {
    SubGhzProtocolEncoderKIA* instance = context;

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
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_kia_check_remote_controller(SubGhzBlockGeneric* instance);

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderKIA instance
 * @return true On success
 */
static bool subghz_protocol_encoder_kia_get_upload(SubGhzProtocolEncoderKIA* instance) {
    furi_assert(instance);

    // Save original button
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->generic.btn);
    }

    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2 + 32) * 2 + 540;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(
            TAG,
            "Size upload exceeds allocated encoder buffer. %i",
            instance->generic.data_count_bit);
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }

    uint8_t btn = subghz_custom_btn_get() == SUBGHZ_CUSTOM_BTN_OK ?
                      subghz_custom_btn_get_original() :
                      subghz_custom_btn_get();

    uint64_t reversed = 0, value = ((uint64_t)(0x0F) << 56) |
                                   ((uint64_t)(instance->generic.cnt & 0xFFFF) << 40) |
                                   ((uint64_t)(instance->generic.serial & 0x0FFFFFFF) << 12) |
                                   ((uint64_t)(btn & 0x0F) << 8);
    for(uint8_t i = 0; i < sizeof(uint64_t); ++i) {
        reversed = (reversed << 8) | ((value >> (i * 8)) & 0xFF);
    }

    instance->generic.data =
        value | (uint64_t)subghz_protocol_blocks_crc8((uint8_t*)&reversed, 7, 0x7F, 0x0F);

    //Send header
    for(uint16_t i = 270; i > 0; i--) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_kia_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_kia_const.te_short);
    }

    for(uint8_t h = 2; h > 0; h--) {
        for(uint8_t i = 15; i > 0; i--) {
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_kia_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_kia_const.te_short);
        }

        for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
            if(bit_read(instance->generic.data, i - 1)) {
                //send bit 1
                instance->encoder.upload[index++] =
                    level_duration_make(true, (uint32_t)subghz_protocol_kia_const.te_long);
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_kia_const.te_long);
            } else {
                //send bit 0
                instance->encoder.upload[index++] =
                    level_duration_make(true, (uint32_t)subghz_protocol_kia_const.te_short);
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_kia_const.te_short);
            }
        }

        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_kia_const.te_long * 3);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_kia_const.te_long * 3);
    }

    return true;
}

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKIA instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_kia_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderKIA* instance = context;

    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic, flipper_format, subghz_protocol_kia_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }

        subghz_protocol_kia_check_remote_controller(&instance->generic);
        if(!subghz_protocol_encoder_kia_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            ret = SubGhzProtocolStatusErrorParserKey;
            break;
        }

        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

const SubGhzProtocol subghz_protocol_kia = {
    .name = SUBGHZ_PROTOCOL_KIA_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_kia_decoder,
    .encoder = &subghz_protocol_kia_encoder,

    .filter = SubGhzProtocolFilter_Cars,
};

void* subghz_protocol_decoder_kia_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderKIA* instance = malloc(sizeof(SubGhzProtocolDecoderKIA));
    instance->base.protocol = &subghz_protocol_kia;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_decoder_kia_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;
    free(instance);
}

void subghz_protocol_decoder_kia_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;
    instance->decoder.parser_step = KIADecoderStepReset;
}

void subghz_protocol_decoder_kia_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;

    switch(instance->decoder.parser_step) {
    case KIADecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_kia_const.te_short) <
                       subghz_protocol_kia_const.te_delta)) {
            instance->decoder.parser_step = KIADecoderStepCheckPreambula;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;
    case KIADecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, subghz_protocol_kia_const.te_short) <
                subghz_protocol_kia_const.te_delta) ||
               (DURATION_DIFF(duration, subghz_protocol_kia_const.te_long) <
                subghz_protocol_kia_const.te_delta)) {
                instance->decoder.te_last = duration;
            } else {
                instance->decoder.parser_step = KIADecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, subghz_protocol_kia_const.te_short) <
             subghz_protocol_kia_const.te_delta) &&
            (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_kia_const.te_short) <
             subghz_protocol_kia_const.te_delta)) {
            // Found header
            instance->header_count++;
            break;
        } else if(
            (DURATION_DIFF(duration, subghz_protocol_kia_const.te_long) <
             subghz_protocol_kia_const.te_delta) &&
            (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_kia_const.te_long) <
             subghz_protocol_kia_const.te_delta)) {
            // Found start bit
            if(instance->header_count > 15) {
                instance->decoder.parser_step = KIADecoderStepSaveDuration;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 1;
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            } else {
                instance->decoder.parser_step = KIADecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepSaveDuration:
        if(level) {
            if(duration >=
               (subghz_protocol_kia_const.te_long + subghz_protocol_kia_const.te_delta * 2UL)) {
                //Found stop bit
                instance->decoder.parser_step = KIADecoderStepReset;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_kia_const.min_count_bit_for_found) {
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
                instance->decoder.parser_step = KIADecoderStepCheckDuration;
            }

        } else {
            instance->decoder.parser_step = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_kia_const.te_short) <
                subghz_protocol_kia_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_kia_const.te_short) <
                subghz_protocol_kia_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = KIADecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_kia_const.te_long) <
                 subghz_protocol_kia_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_kia_const.te_long) <
                 subghz_protocol_kia_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = KIADecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = KIADecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = KIADecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_kia_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
    *   0x0F 0112 43B04EC 1 7D
    *   0x0F 0113 43B04EC 1 DF
    *   0x0F 0114 43B04EC 1 30
    *   0x0F 0115 43B04EC 2 13
    *   0x0F 0116 43B04EC 3 F5
    *         CNT  Serial K CRC8 Kia (CRC8, poly 0x7f, start_crc 0x0f)
    */

    instance->serial = (uint32_t)((instance->data >> 12) & 0x0FFFFFFF);
    instance->btn = (instance->data >> 8) & 0x0F;
    instance->cnt = (instance->data >> 40) & 0xFFFF;

    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }

    subghz_custom_btn_set_max(4);
}

uint32_t subghz_protocol_decoder_kia_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;
    return subghz_protocol_blocks_get_hash_data_long(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_kia_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_kia_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_kia_const.min_count_bit_for_found);
}

static const char* subghz_protocol_kia_get_name_button(uint8_t btn) {
    const char* name_btn[5] = {"Unknown", "Lock", "Unlock", "Trunk", "Horn"};
    return name_btn[btn < 5 ? btn : 0];
}

void subghz_protocol_decoder_kia_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderKIA* instance = context;

    subghz_protocol_kia_check_remote_controller(&instance->generic);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Sn:%07lX  Cnt:%04lX\r\n"
        "Btn:%s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        instance->generic.serial,
        instance->generic.cnt,
        subghz_protocol_kia_get_name_button(instance->generic.btn));
}
