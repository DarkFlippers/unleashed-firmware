#include "faac_slh.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolFaacSHL"

static const SubGhzBlockConst subghz_protocol_faac_slh_const = {
    .te_short = 255,
    .te_long = 595,
    .te_delta = 100,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderFaacSLH {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderFaacSLH {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    FaacSLHDecoderStepReset = 0,
    FaacSLHDecoderStepFoundPreambula,
    FaacSLHDecoderStepSaveDuration,
    FaacSLHDecoderStepCheckDuration,
} FaacSLHDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_faac_slh_decoder = {
    .alloc = subghz_protocol_decoder_faac_slh_alloc,
    .free = subghz_protocol_decoder_faac_slh_free,

    .feed = subghz_protocol_decoder_faac_slh_feed,
    .reset = subghz_protocol_decoder_faac_slh_reset,

    .get_hash_data = subghz_protocol_decoder_faac_slh_get_hash_data,
    .serialize = subghz_protocol_decoder_faac_slh_serialize,
    .deserialize = subghz_protocol_decoder_faac_slh_deserialize,
    .get_string = subghz_protocol_decoder_faac_slh_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_faac_slh_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_faac_slh = {
    .name = SUBGHZ_PROTOCOL_FAAC_SLH_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_faac_slh_decoder,
    .encoder = &subghz_protocol_faac_slh_encoder,
};

void* subghz_protocol_decoder_faac_slh_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderFaacSLH* instance = malloc(sizeof(SubGhzProtocolDecoderFaacSLH));
    instance->base.protocol = &subghz_protocol_faac_slh;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_faac_slh_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    free(instance);
}

void subghz_protocol_decoder_faac_slh_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    instance->decoder.parser_step = FaacSLHDecoderStepReset;
}

void subghz_protocol_decoder_faac_slh_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;

    switch(instance->decoder.parser_step) {
    case FaacSLHDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long * 2) <
                       subghz_protocol_faac_slh_const.te_delta * 3)) {
            instance->decoder.parser_step = FaacSLHDecoderStepFoundPreambula;
        }
        break;
    case FaacSLHDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long * 2) <
                        subghz_protocol_faac_slh_const.te_delta * 3)) {
            //Found Preambula
            instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepSaveDuration:
        if(level) {
            if(duration >= ((uint32_t)subghz_protocol_faac_slh_const.te_short * 3 +
                            subghz_protocol_faac_slh_const.te_delta)) {
                instance->decoder.parser_step = FaacSLHDecoderStepFoundPreambula;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_faac_slh_const.min_count_bit_for_found) {
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
                instance->decoder.parser_step = FaacSLHDecoderStepCheckDuration;
            }

        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_slh_const.te_short) <
                subghz_protocol_faac_slh_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_long) <
                subghz_protocol_faac_slh_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_slh_const.te_long) <
                 subghz_protocol_faac_slh_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_faac_slh_const.te_short) <
                 subghz_protocol_faac_slh_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = FaacSLHDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = FaacSLHDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_faac_slh_check_remote_controller(SubGhzBlockGeneric* instance) {
    uint64_t code_found_reverse =
        subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFFFF;

    instance->serial = code_fix & 0xFFFFFFF;
    instance->btn = (code_fix >> 28) & 0x0F;
}

uint8_t subghz_protocol_decoder_faac_slh_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_faac_slh_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_faac_slh_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_faac_slh_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacSLH* instance = context;
    subghz_protocol_faac_slh_check_remote_controller(&instance->generic);
    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFFFF;
    uint32_t code_hop = (code_found_reverse >> 32) & 0xFFFFFFFF;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%lX%08lX\r\n"
        "Fix:%08lX \r\n"
        "Hop:%08lX \r\n"
        "Sn:%07lX Btn:%lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        code_fix,
        code_hop,
        instance->generic.serial,
        instance->generic.btn);
}
