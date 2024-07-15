#include "doitrand.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolDoitrand"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0001 ? '1' : '0'), (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'),     \
        (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'), (dip & 0x1000 ? '1' : '0'), \
        (dip & 0x0800 ? '1' : '0'), (dip & 0x0400 ? '1' : '0'), (dip & 0x0200 ? '1' : '0'), \
        (dip & 0x0002 ? '1' : '0')

static const SubGhzBlockConst subghz_protocol_doitrand_const = {
    .te_short = 400,
    .te_long = 1100,
    .te_delta = 150,
    .min_count_bit_for_found = 37,
};

struct SubGhzProtocolDecoderDoitrand {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderDoitrand {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    DoitrandDecoderStepReset = 0,
    DoitrandDecoderStepFoundStartBit,
    DoitrandDecoderStepSaveDuration,
    DoitrandDecoderStepCheckDuration,
} DoitrandDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_doitrand_decoder = {
    .alloc = subghz_protocol_decoder_doitrand_alloc,
    .free = subghz_protocol_decoder_doitrand_free,

    .feed = subghz_protocol_decoder_doitrand_feed,
    .reset = subghz_protocol_decoder_doitrand_reset,

    .get_hash_data = subghz_protocol_decoder_doitrand_get_hash_data,
    .serialize = subghz_protocol_decoder_doitrand_serialize,
    .deserialize = subghz_protocol_decoder_doitrand_deserialize,
    .get_string = subghz_protocol_decoder_doitrand_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_doitrand_encoder = {
    .alloc = subghz_protocol_encoder_doitrand_alloc,
    .free = subghz_protocol_encoder_doitrand_free,

    .deserialize = subghz_protocol_encoder_doitrand_deserialize,
    .stop = subghz_protocol_encoder_doitrand_stop,
    .yield = subghz_protocol_encoder_doitrand_yield,
};

const SubGhzProtocol subghz_protocol_doitrand = {
    .name = SUBGHZ_PROTOCOL_DOITRAND_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_doitrand_decoder,
    .encoder = &subghz_protocol_doitrand_encoder,
};

void* subghz_protocol_encoder_doitrand_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderDoitrand* instance = malloc(sizeof(SubGhzProtocolEncoderDoitrand));

    instance->base.protocol = &subghz_protocol_doitrand;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_doitrand_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderDoitrand* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderDoitrand instance
 * @return true On success
 */
static bool subghz_protocol_encoder_doitrand_get_upload(SubGhzProtocolEncoderDoitrand* instance) {
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
        level_duration_make(false, (uint32_t)subghz_protocol_doitrand_const.te_short * 62);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_doitrand_const.te_short * 2 - 100);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_doitrand_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_doitrand_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_doitrand_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_doitrand_const.te_long);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_doitrand_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderDoitrand* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_doitrand_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_doitrand_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_doitrand_stop(void* context) {
    SubGhzProtocolEncoderDoitrand* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_doitrand_yield(void* context) {
    SubGhzProtocolEncoderDoitrand* instance = context;

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

void* subghz_protocol_decoder_doitrand_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderDoitrand* instance = malloc(sizeof(SubGhzProtocolDecoderDoitrand));
    instance->base.protocol = &subghz_protocol_doitrand;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_doitrand_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    free(instance);
}

void subghz_protocol_decoder_doitrand_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    instance->decoder.parser_step = DoitrandDecoderStepReset;
}

void subghz_protocol_decoder_doitrand_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;

    switch(instance->decoder.parser_step) {
    case DoitrandDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_doitrand_const.te_short * 62) <
                        subghz_protocol_doitrand_const.te_delta * 30)) {
            //Found Preambula
            instance->decoder.parser_step = DoitrandDecoderStepFoundStartBit;
        }
        break;
    case DoitrandDecoderStepFoundStartBit:
        if(level && (DURATION_DIFF(duration, (subghz_protocol_doitrand_const.te_short * 2)) <
                     subghz_protocol_doitrand_const.te_delta * 3)) {
            //Found start bit
            instance->decoder.parser_step = DoitrandDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = DoitrandDecoderStepReset;
        }
        break;
    case DoitrandDecoderStepSaveDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_doitrand_const.te_short * 10 +
                            subghz_protocol_doitrand_const.te_delta)) {
                instance->decoder.parser_step = DoitrandDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_doitrand_const.min_count_bit_for_found) {
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
                instance->decoder.parser_step = DoitrandDecoderStepCheckDuration;
            }
        }
        break;
    case DoitrandDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_doitrand_const.te_short) <
                subghz_protocol_doitrand_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_doitrand_const.te_long) <
                subghz_protocol_doitrand_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = DoitrandDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_doitrand_const.te_long) <
                 subghz_protocol_doitrand_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_doitrand_const.te_short) <
                 subghz_protocol_doitrand_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = DoitrandDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = DoitrandDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = DoitrandDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_doitrand_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
*               67892345   0      k    1                    
* 0000082F5F => 00000000000000000 10 000010111101011111 
* 0002082F5F => 00000000000100000 10 000010111101011111
* 0200082F5F => 00010000000000000 10 000010111101011111
* 0400082F5F => 00100000000000000 10 000010111101011111
* 0800082F5F => 01000000000000000 10 000010111101011111
* 1000082F5F => 10000000000000000 10 000010111101011111
* 0020082F5F => 00000001000000000 10 000010111101011111
* 0040082F5F => 00000010000000000 10 000010111101011111
* 0080082F5F => 00000100000000000 10 000010111101011111
* 0100082F5F => 00001000000000000 10 000010111101011111
* 000008AF5F => 00000000000000000 10 001010111101011111
* 1FE208AF5F => 11111111000100000 10 001010111101011111
*
* 0...9 - DIP
* k- KEY
*/
    instance->cnt = (instance->data >> 24) | ((instance->data >> 15) & 0x1);
    instance->btn = ((instance->data >> 18) & 0x3);
}

uint8_t subghz_protocol_decoder_doitrand_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_doitrand_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_doitrand_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_doitrand_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_doitrand_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderDoitrand* instance = context;
    subghz_protocol_doitrand_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%02lX%08lX\r\n"
        "Btn:%X\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32) & 0xFFFFFFFF,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.btn,
        CNT_TO_DIP(instance->generic.cnt));
}
