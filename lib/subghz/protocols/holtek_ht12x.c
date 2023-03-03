#include "holtek_ht12x.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://www.holtek.com/documents/10179/116711/HT12A_Ev130.pdf
 *
 */

#define TAG "SubGhzProtocolHoltek_HT12X"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0080 ? '0' : '1'), (dip & 0x0040 ? '0' : '1'), (dip & 0x0020 ? '0' : '1'),     \
        (dip & 0x0010 ? '0' : '1'), (dip & 0x0008 ? '0' : '1'), (dip & 0x0004 ? '0' : '1'), \
        (dip & 0x0002 ? '0' : '1'), (dip & 0x0001 ? '0' : '1')

static const SubGhzBlockConst subghz_protocol_holtek_th12x_const = {
    .te_short = 320,
    .te_long = 640,
    .te_delta = 200,
    .min_count_bit_for_found = 12,
};

struct SubGhzProtocolDecoderHoltek_HT12X {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
    uint32_t last_data;
};

struct SubGhzProtocolEncoderHoltek_HT12X {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};

typedef enum {
    Holtek_HT12XDecoderStepReset = 0,
    Holtek_HT12XDecoderStepFoundStartBit,
    Holtek_HT12XDecoderStepSaveDuration,
    Holtek_HT12XDecoderStepCheckDuration,
} Holtek_HT12XDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_holtek_th12x_decoder = {
    .alloc = subghz_protocol_decoder_holtek_th12x_alloc,
    .free = subghz_protocol_decoder_holtek_th12x_free,

    .feed = subghz_protocol_decoder_holtek_th12x_feed,
    .reset = subghz_protocol_decoder_holtek_th12x_reset,

    .get_hash_data = subghz_protocol_decoder_holtek_th12x_get_hash_data,
    .serialize = subghz_protocol_decoder_holtek_th12x_serialize,
    .deserialize = subghz_protocol_decoder_holtek_th12x_deserialize,
    .get_string = subghz_protocol_decoder_holtek_th12x_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_holtek_th12x_encoder = {
    .alloc = subghz_protocol_encoder_holtek_th12x_alloc,
    .free = subghz_protocol_encoder_holtek_th12x_free,

    .deserialize = subghz_protocol_encoder_holtek_th12x_deserialize,
    .stop = subghz_protocol_encoder_holtek_th12x_stop,
    .yield = subghz_protocol_encoder_holtek_th12x_yield,
};

const SubGhzProtocol subghz_protocol_holtek_th12x = {
    .name = SUBGHZ_PROTOCOL_HOLTEK_HT12X_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_holtek_th12x_decoder,
    .encoder = &subghz_protocol_holtek_th12x_encoder,
};

void* subghz_protocol_encoder_holtek_th12x_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHoltek_HT12X* instance =
        malloc(sizeof(SubGhzProtocolEncoderHoltek_HT12X));

    instance->base.protocol = &subghz_protocol_holtek_th12x;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_holtek_th12x_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHoltek_HT12X* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_holtek_th12x_get_upload(SubGhzProtocolEncoderHoltek_HT12X* instance) {
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
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te * 36);
    //Send start bit
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)instance->te);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)instance->te * 2);
            instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)instance->te);
        } else {
            //send bit 0
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)instance->te * 2);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_holtek_th12x_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHoltek_HT12X* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_holtek_th12x_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "TE", (uint32_t*)&instance->te, 1)) {
            FURI_LOG_E(TAG, "Missing TE");
            ret = SubGhzProtocolStatusErrorParserTe;
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_holtek_th12x_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_holtek_th12x_stop(void* context) {
    SubGhzProtocolEncoderHoltek_HT12X* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_holtek_th12x_yield(void* context) {
    SubGhzProtocolEncoderHoltek_HT12X* instance = context;

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

void* subghz_protocol_decoder_holtek_th12x_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHoltek_HT12X* instance =
        malloc(sizeof(SubGhzProtocolDecoderHoltek_HT12X));
    instance->base.protocol = &subghz_protocol_holtek_th12x;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_holtek_th12x_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    free(instance);
}

void subghz_protocol_decoder_holtek_th12x_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    instance->decoder.parser_step = Holtek_HT12XDecoderStepReset;
}

void subghz_protocol_decoder_holtek_th12x_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;

    switch(instance->decoder.parser_step) {
    case Holtek_HT12XDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_holtek_th12x_const.te_short * 36) <
                        subghz_protocol_holtek_th12x_const.te_delta * 36)) {
            //Found Preambula
            instance->decoder.parser_step = Holtek_HT12XDecoderStepFoundStartBit;
        }
        break;
    case Holtek_HT12XDecoderStepFoundStartBit:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_holtek_th12x_const.te_short) <
                       subghz_protocol_holtek_th12x_const.te_delta)) {
            //Found StartBit
            instance->decoder.parser_step = Holtek_HT12XDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = duration;
        } else {
            instance->decoder.parser_step = Holtek_HT12XDecoderStepReset;
        }
        break;
    case Holtek_HT12XDecoderStepSaveDuration:
        //save duration
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_holtek_th12x_const.te_short * 10 +
                            subghz_protocol_holtek_th12x_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_holtek_th12x_const.min_count_bit_for_found) {
                    if((instance->last_data == instance->decoder.decode_data) &&
                       instance->last_data) {
                        instance->te /= (instance->decoder.decode_count_bit * 3 + 1);

                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->last_data = instance->decoder.decode_data;
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->te = 0;
                instance->decoder.parser_step = Holtek_HT12XDecoderStepFoundStartBit;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->te += duration;
                instance->decoder.parser_step = Holtek_HT12XDecoderStepCheckDuration;
            }
        } else {
            instance->decoder.parser_step = Holtek_HT12XDecoderStepReset;
        }
        break;
    case Holtek_HT12XDecoderStepCheckDuration:
        if(level) {
            instance->te += duration;
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_holtek_th12x_const.te_long) <
                subghz_protocol_holtek_th12x_const.te_delta * 2) &&
               (DURATION_DIFF(duration, subghz_protocol_holtek_th12x_const.te_short) <
                subghz_protocol_holtek_th12x_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Holtek_HT12XDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_holtek_th12x_const.te_short) <
                 subghz_protocol_holtek_th12x_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_holtek_th12x_const.te_long) <
                 subghz_protocol_holtek_th12x_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Holtek_HT12XDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Holtek_HT12XDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Holtek_HT12XDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_holtek_th12x_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = instance->data & 0x0F;
    instance->cnt = (instance->data >> 4) & 0xFF;
}

uint8_t subghz_protocol_decoder_holtek_th12x_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_holtek_th12x_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_uint32(flipper_format, "TE", &instance->te, 1)) {
        FURI_LOG_E(TAG, "Unable to add TE");
        ret = SubGhzProtocolStatusErrorParserTe;
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_holtek_th12x_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_holtek_th12x_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "TE", (uint32_t*)&instance->te, 1)) {
            FURI_LOG_E(TAG, "Missing TE");
            ret = SubGhzProtocolStatusErrorParserTe;
            break;
        }
    } while(false);
    return ret;
}

static void subghz_protocol_holtek_th12x_event_serialize(uint8_t event, FuriString* output) {
    furi_string_cat_printf(
        output,
        "%s%s%s%s\r\n",
        (((event >> 3) & 0x1) == 0x0 ? "B1 " : ""),
        (((event >> 2) & 0x1) == 0x0 ? "B2 " : ""),
        (((event >> 1) & 0x1) == 0x0 ? "B3 " : ""),
        (((event >> 0) & 0x1) == 0x0 ? "B4 " : ""));
}

void subghz_protocol_decoder_holtek_th12x_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderHoltek_HT12X* instance = context;
    subghz_protocol_holtek_th12x_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%03lX\r\n"
        "Btn: ",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFF));
    subghz_protocol_holtek_th12x_event_serialize(instance->generic.btn, output);
    furi_string_cat_printf(
        output,
        "DIP:" DIP_PATTERN "\r\n"
        "Te:%luus\r\n",
        CNT_TO_DIP(instance->generic.cnt),
        instance->te);
}
