#include "nice_flo.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolNiceFLO"

static const SubGhzBlockConst subghz_protocol_nice_flo_const = {
    .te_short = 700,
    .te_long = 1400,
    .te_delta = 200,
    .min_count_bit_for_found = 12,
};

struct SubGhzProtocolDecoderNiceFlo {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderNiceFlo {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    NiceFloDecoderStepReset = 0,
    NiceFloDecoderStepFoundStartBit,
    NiceFloDecoderStepSaveDuration,
    NiceFloDecoderStepCheckDuration,
} NiceFloDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_nice_flo_decoder = {
    .alloc = subghz_protocol_decoder_nice_flo_alloc,
    .free = subghz_protocol_decoder_nice_flo_free,

    .feed = subghz_protocol_decoder_nice_flo_feed,
    .reset = subghz_protocol_decoder_nice_flo_reset,

    .get_hash_data = subghz_protocol_decoder_nice_flo_get_hash_data,
    .serialize = subghz_protocol_decoder_nice_flo_serialize,
    .deserialize = subghz_protocol_decoder_nice_flo_deserialize,
    .get_string = subghz_protocol_decoder_nice_flo_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nice_flo_encoder = {
    .alloc = subghz_protocol_encoder_nice_flo_alloc,
    .free = subghz_protocol_encoder_nice_flo_free,

    .deserialize = subghz_protocol_encoder_nice_flo_deserialize,
    .stop = subghz_protocol_encoder_nice_flo_stop,
    .yield = subghz_protocol_encoder_nice_flo_yield,
};

const SubGhzProtocol subghz_protocol_nice_flo = {
    .name = SUBGHZ_PROTOCOL_NICE_FLO_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_nice_flo_decoder,
    .encoder = &subghz_protocol_nice_flo_encoder,
};

void* subghz_protocol_encoder_nice_flo_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderNiceFlo* instance = malloc(sizeof(SubGhzProtocolEncoderNiceFlo));

    instance->base.protocol = &subghz_protocol_nice_flo;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52; //max 24bit*2 + 2 (start, stop)
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_nice_flo_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderNiceFlo* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderNiceFlo instance
 * @return true On success
 */
static bool subghz_protocol_encoder_nice_flo_get_upload(SubGhzProtocolEncoderNiceFlo* instance) {
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
        level_duration_make(false, (uint32_t)subghz_protocol_nice_flo_const.te_short * 36);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_nice_flo_const.te_short);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nice_flo_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nice_flo_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nice_flo_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nice_flo_const.te_long);
        }
    }
    return true;
}

bool subghz_protocol_encoder_nice_flo_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderNiceFlo* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_nice_flo_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit !=
            2 * subghz_protocol_nice_flo_const.min_count_bit_for_found)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_nice_flo_get_upload(instance);
        instance->encoder.is_runing = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_nice_flo_stop(void* context) {
    SubGhzProtocolEncoderNiceFlo* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_nice_flo_yield(void* context) {
    SubGhzProtocolEncoderNiceFlo* instance = context;

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

void* subghz_protocol_decoder_nice_flo_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderNiceFlo* instance = malloc(sizeof(SubGhzProtocolDecoderNiceFlo));
    instance->base.protocol = &subghz_protocol_nice_flo;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_nice_flo_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;
    free(instance);
}

void subghz_protocol_decoder_nice_flo_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;
    instance->decoder.parser_step = NiceFloDecoderStepReset;
}

void subghz_protocol_decoder_nice_flo_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;

    switch(instance->decoder.parser_step) {
    case NiceFloDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nice_flo_const.te_short * 36) <
                        subghz_protocol_nice_flo_const.te_delta * 36)) {
            //Found header Nice Flo
            instance->decoder.parser_step = NiceFloDecoderStepFoundStartBit;
        }
        break;
    case NiceFloDecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_nice_flo_const.te_short) <
            subghz_protocol_nice_flo_const.te_delta) {
            //Found start bit Nice Flo
            instance->decoder.parser_step = NiceFloDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = NiceFloDecoderStepReset;
        }
        break;
    case NiceFloDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_nice_flo_const.te_short * 4)) {
                instance->decoder.parser_step = NiceFloDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_nice_flo_const.min_count_bit_for_found) {
                    instance->generic.serial = 0x0;
                    instance->generic.btn = 0x0;

                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = NiceFloDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = NiceFloDecoderStepReset;
        }
        break;
    case NiceFloDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nice_flo_const.te_short) <
                subghz_protocol_nice_flo_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_nice_flo_const.te_long) <
                subghz_protocol_nice_flo_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NiceFloDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nice_flo_const.te_long) <
                 subghz_protocol_nice_flo_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nice_flo_const.te_short) <
                 subghz_protocol_nice_flo_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NiceFloDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = NiceFloDecoderStepReset;
        } else {
            instance->decoder.parser_step = NiceFloDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_nice_flo_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_nice_flo_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_nice_flo_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_nice_flo_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit !=
            2 * subghz_protocol_nice_flo_const.min_count_bit_for_found)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_nice_flo_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlo* instance = context;

    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;
    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%08lX\r\n"
        "Yek:0x%08lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_lo,
        code_found_reverse_lo);
}
