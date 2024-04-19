#include "linear_delta3.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolLinearDelta3"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define DATA_TO_DIP(dip)                                                                    \
    (dip & 0x0080 ? '0' : '1'), (dip & 0x0040 ? '0' : '1'), (dip & 0x0020 ? '0' : '1'),     \
        (dip & 0x0010 ? '0' : '1'), (dip & 0x0008 ? '0' : '1'), (dip & 0x0004 ? '0' : '1'), \
        (dip & 0x0002 ? '0' : '1'), (dip & 0x0001 ? '0' : '1')

static const SubGhzBlockConst subghz_protocol_linear_delta3_const = {
    .te_short = 500,
    .te_long = 2000,
    .te_delta = 150,
    .min_count_bit_for_found = 8,
};

struct SubGhzProtocolDecoderLinearDelta3 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t last_data;
};

struct SubGhzProtocolEncoderLinearDelta3 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    LinearDecoderStepReset = 0,
    LinearDecoderStepSaveDuration,
    LinearDecoderStepCheckDuration,
} LinearDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_linear_delta3_decoder = {
    .alloc = subghz_protocol_decoder_linear_delta3_alloc,
    .free = subghz_protocol_decoder_linear_delta3_free,

    .feed = subghz_protocol_decoder_linear_delta3_feed,
    .reset = subghz_protocol_decoder_linear_delta3_reset,

    .get_hash_data = subghz_protocol_decoder_linear_delta3_get_hash_data,
    .serialize = subghz_protocol_decoder_linear_delta3_serialize,
    .deserialize = subghz_protocol_decoder_linear_delta3_deserialize,
    .get_string = subghz_protocol_decoder_linear_delta3_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_linear_delta3_encoder = {
    .alloc = subghz_protocol_encoder_linear_delta3_alloc,
    .free = subghz_protocol_encoder_linear_delta3_free,

    .deserialize = subghz_protocol_encoder_linear_delta3_deserialize,
    .stop = subghz_protocol_encoder_linear_delta3_stop,
    .yield = subghz_protocol_encoder_linear_delta3_yield,
};

const SubGhzProtocol subghz_protocol_linear_delta3 = {
    .name = SUBGHZ_PROTOCOL_LINEAR_DELTA3_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_linear_delta3_decoder,
    .encoder = &subghz_protocol_linear_delta3_encoder,
};

void* subghz_protocol_encoder_linear_delta3_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderLinearDelta3* instance =
        malloc(sizeof(SubGhzProtocolEncoderLinearDelta3));

    instance->base.protocol = &subghz_protocol_linear_delta3;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 16;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_linear_delta3_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderLinearDelta3* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderLinearDelta3 instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_linear_delta3_get_upload(SubGhzProtocolEncoderLinearDelta3* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 1; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_linear_delta3_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_linear_delta3_const.te_short * 7);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_linear_delta3_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_linear_delta3_const.te_long);
        }
    }
    //Send end bit
    if(bit_read(instance->generic.data, 0)) {
        //send bit 1
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_linear_delta3_const.te_short);
        //Send PT_GUARD
        instance->encoder.upload[index] = level_duration_make(
            false, (uint32_t)subghz_protocol_linear_delta3_const.te_short * 73);
    } else {
        //send bit 0
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_linear_delta3_const.te_long);
        //Send PT_GUARD
        instance->encoder.upload[index] = level_duration_make(
            false, (uint32_t)subghz_protocol_linear_delta3_const.te_short * 70);
    }

    return true;
}

SubGhzProtocolStatus subghz_protocol_encoder_linear_delta3_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderLinearDelta3* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_linear_delta3_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_linear_delta3_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_linear_delta3_stop(void* context) {
    SubGhzProtocolEncoderLinearDelta3* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_linear_delta3_yield(void* context) {
    SubGhzProtocolEncoderLinearDelta3* instance = context;

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

void* subghz_protocol_decoder_linear_delta3_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderLinearDelta3* instance =
        malloc(sizeof(SubGhzProtocolDecoderLinearDelta3));
    instance->base.protocol = &subghz_protocol_linear_delta3;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_linear_delta3_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    free(instance);
}

void subghz_protocol_decoder_linear_delta3_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    instance->decoder.parser_step = LinearDecoderStepReset;
    instance->last_data = 0;
}

void subghz_protocol_decoder_linear_delta3_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    switch(instance->decoder.parser_step) {
    case LinearDecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, subghz_protocol_linear_delta3_const.te_short * 70) <
            subghz_protocol_linear_delta3_const.te_delta * 24)) {
            //Found header Linear
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = LinearDecoderStepSaveDuration;
        }
        break;
    case LinearDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = LinearDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = LinearDecoderStepReset;
        }
        break;
    case LinearDecoderStepCheckDuration:
        if(!level) {
            if(duration >= (subghz_protocol_linear_delta3_const.te_short * 10)) {
                instance->decoder.parser_step = LinearDecoderStepReset;
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_linear_delta3_const.te_short) <
                   subghz_protocol_linear_delta3_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_linear_delta3_const.te_long) <
                    subghz_protocol_linear_delta3_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_linear_delta3_const.min_count_bit_for_found) {
                    if((instance->last_data == instance->decoder.decode_data) &&
                       instance->last_data) {
                        instance->generic.serial = 0x0;
                        instance->generic.btn = 0x0;

                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.parser_step = LinearDecoderStepSaveDuration;
                    instance->last_data = instance->decoder.decode_data;
                }
                break;
            }

            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_linear_delta3_const.te_short) <
                subghz_protocol_linear_delta3_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_linear_delta3_const.te_short * 7) <
                subghz_protocol_linear_delta3_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = LinearDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_linear_delta3_const.te_long) <
                 subghz_protocol_linear_delta3_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_linear_delta3_const.te_long) <
                 subghz_protocol_linear_delta3_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = LinearDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = LinearDecoderStepReset;
            }

        } else {
            instance->decoder.parser_step = LinearDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_linear_delta3_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8));
}

SubGhzProtocolStatus subghz_protocol_decoder_linear_delta3_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus subghz_protocol_decoder_linear_delta3_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_linear_delta3_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_linear_delta3_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderLinearDelta3* instance = context;

    uint32_t data = instance->generic.data & 0xFF;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        data,
        DATA_TO_DIP(data));
}
