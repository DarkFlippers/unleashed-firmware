#include "elplast.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolElplast"

static const SubGhzBlockConst subghz_protocol_elplast_const = {
    .te_short = 230,
    .te_long = 1550,
    .te_delta = 160,
    .min_count_bit_for_found = 18,
};

struct SubGhzProtocolDecoderElplast {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderElplast {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    ElplastDecoderStepReset = 0,
    ElplastDecoderStepSaveDuration,
    ElplastDecoderStepCheckDuration,
} ElplastDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_elplast_decoder = {
    .alloc = subghz_protocol_decoder_elplast_alloc,
    .free = subghz_protocol_decoder_elplast_free,

    .feed = subghz_protocol_decoder_elplast_feed,
    .reset = subghz_protocol_decoder_elplast_reset,

    .get_hash_data = subghz_protocol_decoder_elplast_get_hash_data,
    .serialize = subghz_protocol_decoder_elplast_serialize,
    .deserialize = subghz_protocol_decoder_elplast_deserialize,
    .get_string = subghz_protocol_decoder_elplast_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_elplast_encoder = {
    .alloc = subghz_protocol_encoder_elplast_alloc,
    .free = subghz_protocol_encoder_elplast_free,

    .deserialize = subghz_protocol_encoder_elplast_deserialize,
    .stop = subghz_protocol_encoder_elplast_stop,
    .yield = subghz_protocol_encoder_elplast_yield,
};

const SubGhzProtocol subghz_protocol_elplast = {
    .name = SUBGHZ_PROTOCOL_ELPLAST_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_elplast_decoder,
    .encoder = &subghz_protocol_elplast_encoder,
};

void* subghz_protocol_encoder_elplast_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderElplast* instance = malloc(sizeof(SubGhzProtocolEncoderElplast));

    instance->base.protocol = &subghz_protocol_elplast;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_elplast_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderElplast* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderElplast instance
 */
static void subghz_protocol_encoder_elplast_get_upload(SubGhzProtocolEncoderElplast* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_elplast_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_elplast_const.te_long * 8);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_elplast_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_elplast_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_elplast_const.te_long * 8);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_elplast_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_elplast_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderElplast* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_elplast_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_elplast_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_elplast_stop(void* context) {
    SubGhzProtocolEncoderElplast* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_elplast_yield(void* context) {
    SubGhzProtocolEncoderElplast* instance = context;

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

void* subghz_protocol_decoder_elplast_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderElplast* instance = malloc(sizeof(SubGhzProtocolDecoderElplast));
    instance->base.protocol = &subghz_protocol_elplast;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_elplast_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;
    free(instance);
}

void subghz_protocol_decoder_elplast_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;
    instance->decoder.parser_step = ElplastDecoderStepReset;
}

void subghz_protocol_decoder_elplast_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;

    // Elplast/P-11B/3BK/E.C.A Decoder
    // 2025.09 - @xMasterX (MMX)

    // Key samples
    // 00110010110000001010 = 32C0A
    // 00110010110010000010 = 32C82

    switch(instance->decoder.parser_step) {
    case ElplastDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_elplast_const.te_long * 8) <
                        subghz_protocol_elplast_const.te_delta * 13)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = ElplastDecoderStepSaveDuration;
        }
        break;
    case ElplastDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = ElplastDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = ElplastDecoderStepReset;
        }
        break;
    case ElplastDecoderStepCheckDuration:
        if(!level) {
            // Bit 1 is long and short timing = 1550us HIGH (te_last) and 230us LOW
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_elplast_const.te_long) <
                subghz_protocol_elplast_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_elplast_const.te_short) <
                subghz_protocol_elplast_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = ElplastDecoderStepSaveDuration;
                // Bit 0 is short and long timing = 230us HIGH (te_last) and 1550us LOW
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_elplast_const.te_short) <
                 subghz_protocol_elplast_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_elplast_const.te_long) <
                 subghz_protocol_elplast_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = ElplastDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_elplast_const.te_long * 8) <
                subghz_protocol_elplast_const.te_delta * 13) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_elplast_const.te_long) <
                    subghz_protocol_elplast_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_elplast_const.te_short) <
                    subghz_protocol_elplast_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                // If got 18 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_elplast_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = ElplastDecoderStepReset;
            } else {
                instance->decoder.parser_step = ElplastDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = ElplastDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_elplast_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_elplast_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_elplast_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_elplast_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_elplast_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderElplast* instance = context;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_lo = code_found_reverse & 0x000003ffffffffff;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%05lX\r\n"
        "Yek: 0x%05lX",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        code_found_reverse_lo);
}
