#include "holtek.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://pdf1.alldatasheet.com/datasheet-pdf/view/82103/HOLTEK/HT640.html
 * https://fccid.io/OJM-CMD-HHLR-XXXA
 *
 */

#define TAG "SubGhzProtocolHoltek"

#define HOLTEK_HEADER_MASK 0xF000000000
#define HOLTEK_HEADER 0x5000000000

static const SubGhzBlockConst subghz_protocol_holtek_const = {
    .te_short = 430,
    .te_long = 870,
    .te_delta = 100,
    .min_count_bit_for_found = 40,
};

struct SubGhzProtocolDecoderHoltek {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderHoltek {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    HoltekDecoderStepReset = 0,
    HoltekDecoderStepFoundStartBit,
    HoltekDecoderStepSaveDuration,
    HoltekDecoderStepCheckDuration,
} HoltekDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_holtek_decoder = {
    .alloc = subghz_protocol_decoder_holtek_alloc,
    .free = subghz_protocol_decoder_holtek_free,

    .feed = subghz_protocol_decoder_holtek_feed,
    .reset = subghz_protocol_decoder_holtek_reset,

    .get_hash_data = subghz_protocol_decoder_holtek_get_hash_data,
    .serialize = subghz_protocol_decoder_holtek_serialize,
    .deserialize = subghz_protocol_decoder_holtek_deserialize,
    .get_string = subghz_protocol_decoder_holtek_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_holtek_encoder = {
    .alloc = subghz_protocol_encoder_holtek_alloc,
    .free = subghz_protocol_encoder_holtek_free,

    .deserialize = subghz_protocol_encoder_holtek_deserialize,
    .stop = subghz_protocol_encoder_holtek_stop,
    .yield = subghz_protocol_encoder_holtek_yield,
};

const SubGhzProtocol subghz_protocol_holtek = {
    .name = SUBGHZ_PROTOCOL_HOLTEK_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_holtek_decoder,
    .encoder = &subghz_protocol_holtek_encoder,
};

void* subghz_protocol_encoder_holtek_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHoltek* instance = malloc(sizeof(SubGhzProtocolEncoderHoltek));

    instance->base.protocol = &subghz_protocol_holtek;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_holtek_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHoltek* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHoltek instance
 * @return true On success
 */
static bool subghz_protocol_encoder_holtek_get_upload(SubGhzProtocolEncoderHoltek* instance) {
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
        level_duration_make(false, (uint32_t)subghz_protocol_holtek_const.te_short * 36);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_holtek_const.te_short);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_holtek_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_holtek_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_holtek_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_holtek_const.te_long);
        }
    }
    return true;
}

bool subghz_protocol_encoder_holtek_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHoltek* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_holtek_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_holtek_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_holtek_stop(void* context) {
    SubGhzProtocolEncoderHoltek* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_holtek_yield(void* context) {
    SubGhzProtocolEncoderHoltek* instance = context;

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

void* subghz_protocol_decoder_holtek_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHoltek* instance = malloc(sizeof(SubGhzProtocolDecoderHoltek));
    instance->base.protocol = &subghz_protocol_holtek;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_holtek_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    free(instance);
}

void subghz_protocol_decoder_holtek_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    instance->decoder.parser_step = HoltekDecoderStepReset;
}

void subghz_protocol_decoder_holtek_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;

    switch(instance->decoder.parser_step) {
    case HoltekDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_holtek_const.te_short * 36) <
                        subghz_protocol_holtek_const.te_delta * 36)) {
            //Found Preambula
            instance->decoder.parser_step = HoltekDecoderStepFoundStartBit;
        }
        break;
    case HoltekDecoderStepFoundStartBit:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_holtek_const.te_short) <
                       subghz_protocol_holtek_const.te_delta)) {
            //Found StartBit
            instance->decoder.parser_step = HoltekDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = HoltekDecoderStepReset;
        }
        break;
    case HoltekDecoderStepSaveDuration:
        //save duration
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_holtek_const.te_short * 10 +
                            subghz_protocol_holtek_const.te_delta)) {
                instance->decoder.parser_step = HoltekDecoderStepSaveDuration;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_holtek_const.min_count_bit_for_found) {
                    if((instance->decoder.decode_data & HOLTEK_HEADER_MASK) == HOLTEK_HEADER) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = HoltekDecoderStepFoundStartBit;
                break;
            } else {
                instance->decoder.te_last = duration;

                instance->decoder.parser_step = HoltekDecoderStepCheckDuration;
            }
        } else {
            instance->decoder.parser_step = HoltekDecoderStepReset;
        }
        break;
    case HoltekDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_holtek_const.te_short) <
                subghz_protocol_holtek_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_holtek_const.te_long) <
                subghz_protocol_holtek_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = HoltekDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_holtek_const.te_long) <
                 subghz_protocol_holtek_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_holtek_const.te_short) <
                 subghz_protocol_holtek_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = HoltekDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = HoltekDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = HoltekDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_holtek_check_remote_controller(SubGhzBlockGeneric* instance) {
    if((instance->data & HOLTEK_HEADER_MASK) == HOLTEK_HEADER) {
        instance->serial =
            subghz_protocol_blocks_reverse_key((instance->data >> 16) & 0xFFFFF, 20);
        uint16_t btn = instance->data & 0xFFFF;
        if((btn & 0xf) != 0xA) {
            instance->btn = 0x1 << 4 | (btn & 0xF);
        } else if(((btn >> 4) & 0xF) != 0xA) {
            instance->btn = 0x2 << 4 | ((btn >> 4) & 0xF);
        } else if(((btn >> 8) & 0xF) != 0xA) {
            instance->btn = 0x3 << 4 | ((btn >> 8) & 0xF);
        } else if(((btn >> 12) & 0xF) != 0xA) {
            instance->btn = 0x4 << 4 | ((btn >> 12) & 0xF);
        } else {
            instance->btn = 0;
        }
    } else {
        instance->serial = 0;
        instance->btn = 0;
        instance->cnt = 0;
    }
}

uint8_t subghz_protocol_decoder_holtek_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_holtek_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_holtek_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_holtek_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_holtek_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek* instance = context;
    subghz_protocol_holtek_check_remote_controller(&instance->generic);

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%05lX Btn:%X ",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)((instance->generic.data >> 32) & 0xFFFFFFFF),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn >> 4);

    if((instance->generic.btn & 0xF) == 0xE) {
        string_cat_printf(output, "ON\r\n");
    } else if(((instance->generic.btn & 0xF) == 0xB)) {
        string_cat_printf(output, "OFF\r\n");
    }
}
