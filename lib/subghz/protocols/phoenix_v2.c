#include "phoenix_v2.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolPhoenix_V2"

//transmission only static mode

static const SubGhzBlockConst subghz_protocol_phoenix_v2_const = {
    .te_short = 427,
    .te_long = 853,
    .te_delta = 100,
    .min_count_bit_for_found = 52,
};

struct SubGhzProtocolDecoderPhoenix_V2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderPhoenix_V2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Phoenix_V2DecoderStepReset = 0,
    Phoenix_V2DecoderStepFoundStartBit,
    Phoenix_V2DecoderStepSaveDuration,
    Phoenix_V2DecoderStepCheckDuration,
} Phoenix_V2DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_phoenix_v2_decoder = {
    .alloc = subghz_protocol_decoder_phoenix_v2_alloc,
    .free = subghz_protocol_decoder_phoenix_v2_free,

    .feed = subghz_protocol_decoder_phoenix_v2_feed,
    .reset = subghz_protocol_decoder_phoenix_v2_reset,

    .get_hash_data = subghz_protocol_decoder_phoenix_v2_get_hash_data,
    .serialize = subghz_protocol_decoder_phoenix_v2_serialize,
    .deserialize = subghz_protocol_decoder_phoenix_v2_deserialize,
    .get_string = subghz_protocol_decoder_phoenix_v2_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_phoenix_v2_encoder = {
    .alloc = subghz_protocol_encoder_phoenix_v2_alloc,
    .free = subghz_protocol_encoder_phoenix_v2_free,

    .deserialize = subghz_protocol_encoder_phoenix_v2_deserialize,
    .stop = subghz_protocol_encoder_phoenix_v2_stop,
    .yield = subghz_protocol_encoder_phoenix_v2_yield,
};

const SubGhzProtocol subghz_protocol_phoenix_v2 = {
    .name = SUBGHZ_PROTOCOL_PHOENIX_V2_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_phoenix_v2_decoder,
    .encoder = &subghz_protocol_phoenix_v2_encoder,
};

void* subghz_protocol_encoder_phoenix_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderPhoenix_V2* instance = malloc(sizeof(SubGhzProtocolEncoderPhoenix_V2));

    instance->base.protocol = &subghz_protocol_phoenix_v2;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_phoenix_v2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_phoenix_v2_get_upload(SubGhzProtocolEncoderPhoenix_V2* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_short * 60);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_short * 6);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(!bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_long);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_phoenix_v2_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_phoenix_v2_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_phoenix_v2_stop(void* context) {
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_phoenix_v2_yield(void* context) {
    SubGhzProtocolEncoderPhoenix_V2* instance = context;

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

void* subghz_protocol_decoder_phoenix_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderPhoenix_V2* instance = malloc(sizeof(SubGhzProtocolDecoderPhoenix_V2));
    instance->base.protocol = &subghz_protocol_phoenix_v2;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_phoenix_v2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    free(instance);
}

void subghz_protocol_decoder_phoenix_v2_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
}

void subghz_protocol_decoder_phoenix_v2_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;

    switch(instance->decoder.parser_step) {
    case Phoenix_V2DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_short * 60) <
                        subghz_protocol_phoenix_v2_const.te_delta * 30)) {
            //Found Preambula
            instance->decoder.parser_step = Phoenix_V2DecoderStepFoundStartBit;
        }
        break;
    case Phoenix_V2DecoderStepFoundStartBit:
        if(level && ((DURATION_DIFF(duration, (subghz_protocol_phoenix_v2_const.te_short * 6)) <
                      subghz_protocol_phoenix_v2_const.te_delta * 4))) {
            //Found start bit
            instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
        }
        break;
    case Phoenix_V2DecoderStepSaveDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_phoenix_v2_const.te_short * 10 +
                            subghz_protocol_phoenix_v2_const.te_delta)) {
                instance->decoder.parser_step = Phoenix_V2DecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_phoenix_v2_const.min_count_bit_for_found) {
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
                instance->decoder.parser_step = Phoenix_V2DecoderStepCheckDuration;
            }
        }
        break;
    case Phoenix_V2DecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_phoenix_v2_const.te_short) <
                subghz_protocol_phoenix_v2_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_long) <
                subghz_protocol_phoenix_v2_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_phoenix_v2_const.te_long) <
                 subghz_protocol_phoenix_v2_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_short) <
                 subghz_protocol_phoenix_v2_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_phoenix_v2_check_remote_controller(SubGhzBlockGeneric* instance) {
    uint64_t data_rev =
        subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit + 4);
    instance->serial = data_rev & 0xFFFFFFFF;
    instance->cnt = (data_rev >> 40) & 0xFFFF;
    instance->btn = (data_rev >> 32) & 0xF;
}

uint8_t subghz_protocol_decoder_phoenix_v2_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_phoenix_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_phoenix_v2_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_phoenix_v2_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    subghz_protocol_phoenix_v2_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%02lX%08lX\r\n"
        "Sn:0x%07lX \r\n"
        "Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32) & 0xFFFFFFFF,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
