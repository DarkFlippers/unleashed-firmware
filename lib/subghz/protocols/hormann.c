#include "hormann.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolHormannHsm"

#define HORMANN_HSM_PATTERN 0xFF000000003

static const SubGhzBlockConst subghz_protocol_hormann_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 200,
    .min_count_bit_for_found = 44,
};

struct SubGhzProtocolDecoderHormann {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderHormann {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    HormannDecoderStepReset = 0,
    HormannDecoderStepFoundStartHeader,
    HormannDecoderStepFoundHeader,
    HormannDecoderStepFoundStartBit,
    HormannDecoderStepSaveDuration,
    HormannDecoderStepCheckDuration,
} HormannDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_hormann_decoder = {
    .alloc = subghz_protocol_decoder_hormann_alloc,
    .free = subghz_protocol_decoder_hormann_free,

    .feed = subghz_protocol_decoder_hormann_feed,
    .reset = subghz_protocol_decoder_hormann_reset,

    .get_hash_data = subghz_protocol_decoder_hormann_get_hash_data,
    .serialize = subghz_protocol_decoder_hormann_serialize,
    .deserialize = subghz_protocol_decoder_hormann_deserialize,
    .get_string = subghz_protocol_decoder_hormann_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_hormann_encoder = {
    .alloc = subghz_protocol_encoder_hormann_alloc,
    .free = subghz_protocol_encoder_hormann_free,

    .deserialize = subghz_protocol_encoder_hormann_deserialize,
    .stop = subghz_protocol_encoder_hormann_stop,
    .yield = subghz_protocol_encoder_hormann_yield,
};

const SubGhzProtocol subghz_protocol_hormann = {
    .name = SUBGHZ_PROTOCOL_HORMANN_HSM_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_hormann_decoder,
    .encoder = &subghz_protocol_hormann_encoder,
};

void* subghz_protocol_encoder_hormann_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHormann* instance = malloc(sizeof(SubGhzProtocolEncoderHormann));

    instance->base.protocol = &subghz_protocol_hormann;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 2048;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_hormann_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHormann* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHormann instance
 * @return true On success
 */
static bool subghz_protocol_encoder_hormann_get_upload(SubGhzProtocolEncoderHormann* instance) {
    furi_assert(instance);

    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2 + 2) * 20 + 1;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    instance->encoder.repeat = 10; //original remote does 10 repeats

    for(size_t repeat = 0; repeat < 20; repeat++) {
        //Send start bit
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_hormann_const.te_short * 24);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_hormann_const.te_short);
        //Send key data
        for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
            if(bit_read(instance->generic.data, i - 1)) {
                //send bit 1
                instance->encoder.upload[index++] =
                    level_duration_make(true, (uint32_t)subghz_protocol_hormann_const.te_long);
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hormann_const.te_short);
            } else {
                //send bit 0
                instance->encoder.upload[index++] =
                    level_duration_make(true, (uint32_t)subghz_protocol_hormann_const.te_short);
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hormann_const.te_long);
            }
        }
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_hormann_const.te_short * 24);
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_hormann_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHormann* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_hormann_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_hormann_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_hormann_stop(void* context) {
    SubGhzProtocolEncoderHormann* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_hormann_yield(void* context) {
    SubGhzProtocolEncoderHormann* instance = context;

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

void* subghz_protocol_decoder_hormann_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHormann* instance = malloc(sizeof(SubGhzProtocolDecoderHormann));
    instance->base.protocol = &subghz_protocol_hormann;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_hormann_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    free(instance);
}

static bool subghz_protocol_decoder_hormann_check_pattern(SubGhzProtocolDecoderHormann* instance) {
    return (instance->decoder.decode_data & HORMANN_HSM_PATTERN) == HORMANN_HSM_PATTERN;
}

void subghz_protocol_decoder_hormann_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    instance->decoder.parser_step = HormannDecoderStepReset;
}

void subghz_protocol_decoder_hormann_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;

    switch(instance->decoder.parser_step) {
    case HormannDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_hormann_const.te_short * 24) <
                       subghz_protocol_hormann_const.te_delta * 24)) {
            instance->decoder.parser_step = HormannDecoderStepFoundStartBit;
        }
        break;
    case HormannDecoderStepFoundStartBit:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_hormann_const.te_short) <
                        subghz_protocol_hormann_const.te_delta)) {
            instance->decoder.parser_step = HormannDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepSaveDuration:
        if(level) { //save interval
            if(duration >= (subghz_protocol_hormann_const.te_short * 5) &&
               subghz_protocol_decoder_hormann_check_pattern(instance)) {
                instance->decoder.parser_step = HormannDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_hormann_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = HormannDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hormann_const.te_short) <
                subghz_protocol_hormann_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_hormann_const.te_long) <
                subghz_protocol_hormann_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = HormannDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hormann_const.te_long) <
                 subghz_protocol_hormann_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_hormann_const.te_short) <
                 subghz_protocol_hormann_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = HormannDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = HormannDecoderStepReset;
        } else {
            instance->decoder.parser_step = HormannDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_hormann_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = (instance->data >> 8) & 0xF;
}

uint8_t subghz_protocol_decoder_hormann_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_hormann_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_hormann_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_hormann_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_hormann_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderHormann* instance = context;
    subghz_protocol_hormann_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s\r\n"
        "%dbit\r\n"
        "Key:0x%03lX%08lX\r\n"
        "Btn:0x%01X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->generic.btn);
}
