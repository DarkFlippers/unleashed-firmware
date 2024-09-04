#include "gangqi.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolGangQi"

static const SubGhzBlockConst subghz_protocol_gangqi_const = {
    .te_short = 500,
    .te_long = 1200,
    .te_delta = 200,
    .min_count_bit_for_found = 34,
};

struct SubGhzProtocolDecoderGangQi {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderGangQi {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    GangQiDecoderStepReset = 0,
    GangQiDecoderStepSaveDuration,
    GangQiDecoderStepCheckDuration,
} GangQiDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_gangqi_decoder = {
    .alloc = subghz_protocol_decoder_gangqi_alloc,
    .free = subghz_protocol_decoder_gangqi_free,

    .feed = subghz_protocol_decoder_gangqi_feed,
    .reset = subghz_protocol_decoder_gangqi_reset,

    .get_hash_data = subghz_protocol_decoder_gangqi_get_hash_data,
    .serialize = subghz_protocol_decoder_gangqi_serialize,
    .deserialize = subghz_protocol_decoder_gangqi_deserialize,
    .get_string = subghz_protocol_decoder_gangqi_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_gangqi_encoder = {
    .alloc = subghz_protocol_encoder_gangqi_alloc,
    .free = subghz_protocol_encoder_gangqi_free,

    .deserialize = subghz_protocol_encoder_gangqi_deserialize,
    .stop = subghz_protocol_encoder_gangqi_stop,
    .yield = subghz_protocol_encoder_gangqi_yield,
};

const SubGhzProtocol subghz_protocol_gangqi = {
    .name = SUBGHZ_PROTOCOL_GANGQI_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_gangqi_decoder,
    .encoder = &subghz_protocol_gangqi_encoder,
};

void* subghz_protocol_encoder_gangqi_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderGangQi* instance = malloc(sizeof(SubGhzProtocolEncoderGangQi));

    instance->base.protocol = &subghz_protocol_gangqi;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_gangqi_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGangQi* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderGangQi instance
 */
static void subghz_protocol_encoder_gangqi_get_upload(SubGhzProtocolEncoderGangQi* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gangqi_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_gangqi_const.te_short * 4 +
                        subghz_protocol_gangqi_const.te_delta);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_gangqi_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gangqi_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_gangqi_const.te_short * 4 +
                        subghz_protocol_gangqi_const.te_delta);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_gangqi_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data and parsing serial number
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_gangqi_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = (instance->data & 0xFFFFF0000) >> 16;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderGangQi* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_gangqi_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_gangqi_remote_controller(&instance->generic);
        subghz_protocol_encoder_gangqi_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_gangqi_stop(void* context) {
    SubGhzProtocolEncoderGangQi* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_gangqi_yield(void* context) {
    SubGhzProtocolEncoderGangQi* instance = context;

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

void* subghz_protocol_decoder_gangqi_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderGangQi* instance = malloc(sizeof(SubGhzProtocolDecoderGangQi));
    instance->base.protocol = &subghz_protocol_gangqi;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_gangqi_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    free(instance);
}

void subghz_protocol_decoder_gangqi_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    instance->decoder.parser_step = GangQiDecoderStepReset;
}

void subghz_protocol_decoder_gangqi_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;

    // Key example
    // 00 10011010111101001101110101011101 00

    switch(instance->decoder.parser_step) {
    case GangQiDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_long * 2) <
                        subghz_protocol_gangqi_const.te_delta * 3)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
        }
        break;
    case GangQiDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = GangQiDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = GangQiDecoderStepReset;
        }
        break;
    case GangQiDecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short and long timing
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_short) <
                subghz_protocol_gangqi_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_long) <
                subghz_protocol_gangqi_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
                // Bit 1 is long and short timing
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_long) <
                 subghz_protocol_gangqi_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short) <
                 subghz_protocol_gangqi_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                subghz_protocol_gangqi_const.te_delta) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_gangqi_const.te_short) <
                    subghz_protocol_gangqi_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                    subghz_protocol_gangqi_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_long) <
                    subghz_protocol_gangqi_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                    subghz_protocol_gangqi_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 34 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_gangqi_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = GangQiDecoderStepReset;
            } else {
                instance->decoder.parser_step = GangQiDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = GangQiDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_gangqi_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_gangqi_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_gangqi_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_gangqi_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;

    // Parse serial
    subghz_protocol_gangqi_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%X%08lX\r\n"
        "Serial: 0x%05lX\r\n"
        "Button code: 0x%04lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint8_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        (uint32_t)(instance->generic.data & 0xFFFF));
}
