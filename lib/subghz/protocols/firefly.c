#include "firefly.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolFirefly"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define DATA_TO_DIP(dip)                                                                    \
    (dip & 0x0200 ? '1' : '0'), (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'),     \
        (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), \
        (dip & 0x0008 ? '1' : '0'), (dip & 0x0004 ? '1' : '0'), (dip & 0x0002 ? '1' : '0'), \
        (dip & 0x0001 ? '1' : '0')

static const SubGhzBlockConst subghz_protocol_firefly_const = {
    .te_short = 500,
    .te_long = 1500,
    .te_delta = 150,
    .min_count_bit_for_found = 10,
};

struct SubGhzProtocolDecoderFirefly {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderFirefly {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    FireflyDecoderStepReset = 0,
    FireflyDecoderStepSaveDuration,
    FireflyDecoderStepCheckDuration,
} FireflyDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_firefly_decoder = {
    .alloc = subghz_protocol_decoder_firefly_alloc,
    .free = subghz_protocol_decoder_firefly_free,

    .feed = subghz_protocol_decoder_firefly_feed,
    .reset = subghz_protocol_decoder_firefly_reset,

    .get_hash_data = subghz_protocol_decoder_firefly_get_hash_data,
    .serialize = subghz_protocol_decoder_firefly_serialize,
    .deserialize = subghz_protocol_decoder_firefly_deserialize,
    .get_string = subghz_protocol_decoder_firefly_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_firefly_encoder = {
    .alloc = subghz_protocol_encoder_firefly_alloc,
    .free = subghz_protocol_encoder_firefly_free,

    .deserialize = subghz_protocol_encoder_firefly_deserialize,
    .stop = subghz_protocol_encoder_firefly_stop,
    .yield = subghz_protocol_encoder_firefly_yield,
};

const SubGhzProtocol subghz_protocol_firefly = {
    .name = SUBGHZ_PROTOCOL_FIREFLY_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_firefly_decoder,
    .encoder = &subghz_protocol_firefly_encoder,
};

void* subghz_protocol_encoder_firefly_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderFirefly* instance = malloc(sizeof(SubGhzProtocolEncoderFirefly));

    instance->base.protocol = &subghz_protocol_firefly;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 28; //max 10bit*2 + 2 (start, stop)
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_firefly_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderFirefly* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderFirefly instance
 * @return true On success
 */
static bool subghz_protocol_encoder_firefly_get_upload(SubGhzProtocolEncoderFirefly* instance) {
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
                level_duration_make(true, (uint32_t)subghz_protocol_firefly_const.te_short * 3);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_firefly_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_firefly_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_firefly_const.te_short * 3);
        }
    }
    //Send end bit
    if(bit_read(instance->generic.data, 0)) {
        //send bit 1
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_firefly_const.te_short * 3);
        //Send PT_GUARD
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_firefly_const.te_short * 42);
    } else {
        //send bit 0
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_firefly_const.te_short);
        //Send PT_GUARD
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_firefly_const.te_short * 44);
    }

    return true;
}

bool subghz_protocol_encoder_firefly_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderFirefly* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_firefly_get_upload(instance);
        instance->encoder.is_runing = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_firefly_stop(void* context) {
    SubGhzProtocolEncoderFirefly* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_firefly_yield(void* context) {
    SubGhzProtocolEncoderFirefly* instance = context;

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

void* subghz_protocol_decoder_firefly_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderFirefly* instance = malloc(sizeof(SubGhzProtocolDecoderFirefly));
    instance->base.protocol = &subghz_protocol_firefly;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_firefly_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    free(instance);
}

void subghz_protocol_decoder_firefly_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    instance->decoder.parser_step = FireflyDecoderStepReset;
}

void subghz_protocol_decoder_firefly_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    switch(instance->decoder.parser_step) {
    case FireflyDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_firefly_const.te_short * 42) <
                        subghz_protocol_firefly_const.te_delta * 20)) {
            //Found header Firefly
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = FireflyDecoderStepSaveDuration;
        }
        break;
    case FireflyDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = FireflyDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = FireflyDecoderStepReset;
        }
        break;
    case FireflyDecoderStepCheckDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_firefly_const.te_short * 5)) {
                instance->decoder.parser_step = FireflyDecoderStepReset;
                //checking that the duration matches the guardtime
                if((DURATION_DIFF(duration, subghz_protocol_firefly_const.te_short * 42) >
                    subghz_protocol_firefly_const.te_delta * 20)) {
                    break;
                }
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_firefly_const.te_short) <
                   subghz_protocol_firefly_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_firefly_const.te_long) <
                    subghz_protocol_firefly_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_firefly_const.min_count_bit_for_found) {
                    instance->generic.serial = 0x0;
                    instance->generic.btn = 0x0;

                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }

            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_firefly_const.te_short) <
                subghz_protocol_firefly_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_firefly_const.te_long) <
                subghz_protocol_firefly_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = FireflyDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_firefly_const.te_long) <
                 subghz_protocol_firefly_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_firefly_const.te_short) <
                 subghz_protocol_firefly_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = FireflyDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = FireflyDecoderStepReset;
            }

        } else {
            instance->decoder.parser_step = FireflyDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_firefly_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_firefly_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, frequency, preset);
}

bool subghz_protocol_decoder_firefly_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;
    return subghz_block_generic_deserialize(&instance->generic, flipper_format);
}

void subghz_protocol_decoder_firefly_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderFirefly* instance = context;

    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%08lX\r\n"
        "Yek:0x%08lX\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_lo,
        code_found_reverse_lo,
        DATA_TO_DIP(code_found_lo));
}
