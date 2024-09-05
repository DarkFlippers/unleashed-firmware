#include "marantec24.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolMarantec24"

static const SubGhzBlockConst subghz_protocol_marantec24_const = {
    .te_short = 800,
    .te_long = 1600,
    .te_delta = 200,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderMarantec24 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderMarantec24 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Marantec24DecoderStepReset = 0,
    Marantec24DecoderStepSaveDuration,
    Marantec24DecoderStepCheckDuration,
} Marantec24DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_marantec24_decoder = {
    .alloc = subghz_protocol_decoder_marantec24_alloc,
    .free = subghz_protocol_decoder_marantec24_free,

    .feed = subghz_protocol_decoder_marantec24_feed,
    .reset = subghz_protocol_decoder_marantec24_reset,

    .get_hash_data = subghz_protocol_decoder_marantec24_get_hash_data,
    .serialize = subghz_protocol_decoder_marantec24_serialize,
    .deserialize = subghz_protocol_decoder_marantec24_deserialize,
    .get_string = subghz_protocol_decoder_marantec24_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_marantec24_encoder = {
    .alloc = subghz_protocol_encoder_marantec24_alloc,
    .free = subghz_protocol_encoder_marantec24_free,

    .deserialize = subghz_protocol_encoder_marantec24_deserialize,
    .stop = subghz_protocol_encoder_marantec24_stop,
    .yield = subghz_protocol_encoder_marantec24_yield,
};

const SubGhzProtocol subghz_protocol_marantec24 = {
    .name = SUBGHZ_PROTOCOL_MARANTEC24_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_marantec24_decoder,
    .encoder = &subghz_protocol_marantec24_encoder,
};

void* subghz_protocol_encoder_marantec24_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderMarantec24* instance = malloc(sizeof(SubGhzProtocolEncoderMarantec24));

    instance->base.protocol = &subghz_protocol_marantec24;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_marantec24_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderMarantec24* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderMarantec24 instance
 */
static void
    subghz_protocol_encoder_marantec24_get_upload(SubGhzProtocolEncoderMarantec24* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_marantec24_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_marantec24_const.te_long * 9 +
                        subghz_protocol_marantec24_const.te_short);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_marantec24_const.te_long * 2);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_marantec24_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_marantec24_const.te_long * 9 +
                        subghz_protocol_marantec24_const.te_short);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_marantec24_const.te_short * 3);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_marantec24_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = instance->data >> 4;
    instance->btn = instance->data & 0xF;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_marantec24_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderMarantec24* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_marantec24_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_marantec24_check_remote_controller(&instance->generic);
        subghz_protocol_encoder_marantec24_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_marantec24_stop(void* context) {
    SubGhzProtocolEncoderMarantec24* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_marantec24_yield(void* context) {
    SubGhzProtocolEncoderMarantec24* instance = context;

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

void* subghz_protocol_decoder_marantec24_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderMarantec24* instance = malloc(sizeof(SubGhzProtocolDecoderMarantec24));
    instance->base.protocol = &subghz_protocol_marantec24;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_marantec24_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;
    free(instance);
}

void subghz_protocol_decoder_marantec24_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;
    instance->decoder.parser_step = Marantec24DecoderStepReset;
}

void subghz_protocol_decoder_marantec24_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;

    // Key samples
    // 101011000000010111001000 = AC05C8
    // 101011000000010111000100 = AC05C4
    // 101011000000010111001100 = AC05CC
    // 101011000000010111000000 = AC05C0

    switch(instance->decoder.parser_step) {
    case Marantec24DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_long * 9) <
                        subghz_protocol_marantec24_const.te_delta * 4)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = Marantec24DecoderStepSaveDuration;
        }
        break;
    case Marantec24DecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Marantec24DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Marantec24DecoderStepReset;
        }
        break;
    case Marantec24DecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is long and short x2 timing = 1600us HIGH (te_last) and 2400us LOW
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_marantec24_const.te_long) <
                subghz_protocol_marantec24_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_short * 3) <
                subghz_protocol_marantec24_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Marantec24DecoderStepSaveDuration;
                // Bit 1 is short and long x2 timing = 800us HIGH (te_last) and 3200us LOW
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_marantec24_const.te_short) <
                 subghz_protocol_marantec24_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_long * 2) <
                 subghz_protocol_marantec24_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Marantec24DecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_long * 9) <
                subghz_protocol_marantec24_const.te_delta * 4) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_marantec24_const.te_long) <
                    subghz_protocol_marantec24_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_long * 9) <
                    subghz_protocol_marantec24_const.te_delta * 4)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_marantec24_const.te_short) <
                    subghz_protocol_marantec24_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_marantec24_const.te_long * 9) <
                    subghz_protocol_marantec24_const.te_delta * 4)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 24 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_marantec24_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = Marantec24DecoderStepReset;
            } else {
                instance->decoder.parser_step = Marantec24DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Marantec24DecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_marantec24_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_marantec24_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_marantec24_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_marantec24_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_marantec24_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec24* instance = context;

    subghz_protocol_marantec24_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%06lX\r\n"
        "Serial: 0x%05lX\r\n"
        "Btn: %01X",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
