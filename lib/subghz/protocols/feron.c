#include "feron.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolFeron"

static const SubGhzBlockConst subghz_protocol_feron_const = {
    .te_short = 350,
    .te_long = 750,
    .te_delta = 150,
    .min_count_bit_for_found = 32,
};

struct SubGhzProtocolDecoderFeron {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderFeron);

struct SubGhzProtocolEncoderFeron {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderFeron);

typedef enum {
    FeronDecoderStepReset = 0,
    FeronDecoderStepSaveDuration,
    FeronDecoderStepCheckDuration,
} FeronDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_feron_decoder = {
    .alloc = subghz_protocol_decoder_feron_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_feron_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize,
    .deserialize = subghz_protocol_decoder_feron_deserialize,
    .get_string = subghz_protocol_decoder_feron_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_feron_encoder = {
    .alloc = subghz_protocol_encoder_feron_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_feron_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_feron = {
    .name = SUBGHZ_PROTOCOL_FERON_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send |
            SubGhzProtocolFlag_Sensors,

    .decoder = &subghz_protocol_feron_decoder,
    .encoder = &subghz_protocol_feron_encoder,
};

void* subghz_protocol_encoder_feron_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderFeron), &subghz_protocol_feron, 3, 256);
}

static void subghz_protocol_feron_check_remote_controller(SubGhzBlockGeneric* instance);

/**
 * Generating an upload from data.
 * @param context Pointer to a SubGhzProtocolEncoderFeron instance
 * @return true Always; this encoder has no failure path
 */
static bool subghz_protocol_encoder_feron_get_upload(void* context) {
    SubGhzProtocolEncoderFeron* instance = context;
    furi_assert(instance);

    subghz_protocol_feron_check_remote_controller(&instance->generic);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_feron_const.te_long);
            if(i == 1) {
                //Send 500/500 and gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_feron_const.te_short + 150);
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_feron_const.te_short + 150);
                // Gap
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_feron_const.te_long * 6);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_feron_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_feron_const.te_short);
            if(i == 1) {
                //Send 500/500 and gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_feron_const.te_short + 150);
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_feron_const.te_short + 150);
                // Gap
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_feron_const.te_long * 6);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_feron_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return true;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_feron_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = instance->data >> 16;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_feron_deserialize(void* context, FlipperFormat* flipper_format) {
    return subghz_protocol_encoder_common_deserialize(
        context,
        flipper_format,
        subghz_protocol_feron_const.min_count_bit_for_found,
        subghz_protocol_encoder_feron_get_upload);
}

void* subghz_protocol_decoder_feron_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderFeron), &subghz_protocol_feron);
}

void subghz_protocol_decoder_feron_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFeron* instance = context;

    // Feron Decoder
    // 2025.04 - @xMasterX (MMX)

    // Key samples
    /*
    0110001100111000 1000010101111010 - ON
    0110001100111000 1000010001111011 - OFF

    0110001100111000 1000011001111001 - brightness up
    0110001100111000 1000011101111000 - brightness down
 
    0110001100111000 1000001001111101 - scroll mode command

    ------------------------------------------
    0110001100111000 0111000010001111 - R
    0110001100111000 0001101011100101 - B
    0110001100111000 0100000010111111 - G
    */

    switch(instance->decoder.parser_step) {
    case FeronDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_feron_const.te_long * 6) <
                        subghz_protocol_feron_const.te_delta * 4)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = FeronDecoderStepSaveDuration;
        }
        break;
    case FeronDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = FeronDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = FeronDecoderStepReset;
        }
        break;
    case FeronDecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short and long timing = 350us HIGH (te_last) and 750us LOW
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_feron_const.te_short) <
                subghz_protocol_feron_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_feron_const.te_long) <
                subghz_protocol_feron_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = FeronDecoderStepSaveDuration;
                // Bit 1 is long and short timing = 750us HIGH (te_last) and 350us LOW
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_feron_const.te_long) <
                 subghz_protocol_feron_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_feron_const.te_short) <
                 subghz_protocol_feron_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = FeronDecoderStepSaveDuration;
            } else if(
                // End of the key 500Low(we are here)/500High us
                DURATION_DIFF(
                    duration, (uint16_t)(subghz_protocol_feron_const.te_short + (uint16_t)150)) <
                subghz_protocol_feron_const.te_delta) {
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_feron_const.te_short) <
                    subghz_protocol_feron_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_feron_const.te_long) <
                    subghz_protocol_feron_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 32 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_feron_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = FeronDecoderStepReset;
            } else {
                instance->decoder.parser_step = FeronDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = FeronDecoderStepReset;
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_feron_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFeron* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_feron_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_feron_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderFeron* instance = context;

    subghz_protocol_feron_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%08lX\r\n"
        "Serial: 0x%04lX\r\n"
        "Command: 0x%04lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        (uint32_t)(instance->generic.data & 0xFFFF));
}
