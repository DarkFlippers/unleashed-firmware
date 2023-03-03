#include "ansonic.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolAnsonic"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0800 ? '1' : '0'), (dip & 0x0400 ? '1' : '0'), (dip & 0x0200 ? '1' : '0'),     \
        (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'), \
        (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), (dip & 0x0001 ? '1' : '0'), \
        (dip & 0x0008 ? '1' : '0')

static const SubGhzBlockConst subghz_protocol_ansonic_const = {
    .te_short = 555,
    .te_long = 1111,
    .te_delta = 120,
    .min_count_bit_for_found = 12,
};

struct SubGhzProtocolDecoderAnsonic {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderAnsonic {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    AnsonicDecoderStepReset = 0,
    AnsonicDecoderStepFoundStartBit,
    AnsonicDecoderStepSaveDuration,
    AnsonicDecoderStepCheckDuration,
} AnsonicDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_ansonic_decoder = {
    .alloc = subghz_protocol_decoder_ansonic_alloc,
    .free = subghz_protocol_decoder_ansonic_free,

    .feed = subghz_protocol_decoder_ansonic_feed,
    .reset = subghz_protocol_decoder_ansonic_reset,

    .get_hash_data = subghz_protocol_decoder_ansonic_get_hash_data,
    .serialize = subghz_protocol_decoder_ansonic_serialize,
    .deserialize = subghz_protocol_decoder_ansonic_deserialize,
    .get_string = subghz_protocol_decoder_ansonic_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_ansonic_encoder = {
    .alloc = subghz_protocol_encoder_ansonic_alloc,
    .free = subghz_protocol_encoder_ansonic_free,

    .deserialize = subghz_protocol_encoder_ansonic_deserialize,
    .stop = subghz_protocol_encoder_ansonic_stop,
    .yield = subghz_protocol_encoder_ansonic_yield,
};

const SubGhzProtocol subghz_protocol_ansonic = {
    .name = SUBGHZ_PROTOCOL_ANSONIC_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_FM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_ansonic_decoder,
    .encoder = &subghz_protocol_ansonic_encoder,
};

void* subghz_protocol_encoder_ansonic_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderAnsonic* instance = malloc(sizeof(SubGhzProtocolEncoderAnsonic));

    instance->base.protocol = &subghz_protocol_ansonic;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_ansonic_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderAnsonic* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderAnsonic instance
 * @return true On success
 */
static bool subghz_protocol_encoder_ansonic_get_upload(SubGhzProtocolEncoderAnsonic* instance) {
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
        level_duration_make(false, (uint32_t)subghz_protocol_ansonic_const.te_short * 35);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_ansonic_const.te_short);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_ansonic_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_ansonic_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_ansonic_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_ansonic_const.te_short);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_ansonic_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderAnsonic* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        res = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_ansonic_const.min_count_bit_for_found);
        if(res != SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_ansonic_get_upload(instance)) {
            res = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_ansonic_stop(void* context) {
    SubGhzProtocolEncoderAnsonic* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_ansonic_yield(void* context) {
    SubGhzProtocolEncoderAnsonic* instance = context;

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

void* subghz_protocol_decoder_ansonic_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderAnsonic* instance = malloc(sizeof(SubGhzProtocolDecoderAnsonic));
    instance->base.protocol = &subghz_protocol_ansonic;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_ansonic_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    free(instance);
}

void subghz_protocol_decoder_ansonic_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    instance->decoder.parser_step = AnsonicDecoderStepReset;
}

void subghz_protocol_decoder_ansonic_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;

    switch(instance->decoder.parser_step) {
    case AnsonicDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_ansonic_const.te_short * 35) <
                        subghz_protocol_ansonic_const.te_delta * 35)) {
            //Found header Ansonic
            instance->decoder.parser_step = AnsonicDecoderStepFoundStartBit;
        }
        break;
    case AnsonicDecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_ansonic_const.te_short) <
            subghz_protocol_ansonic_const.te_delta) {
            //Found start bit Ansonic
            instance->decoder.parser_step = AnsonicDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = AnsonicDecoderStepReset;
        }
        break;
    case AnsonicDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_ansonic_const.te_short * 4)) {
                instance->decoder.parser_step = AnsonicDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_ansonic_const.min_count_bit_for_found) {
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
            instance->decoder.parser_step = AnsonicDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = AnsonicDecoderStepReset;
        }
        break;
    case AnsonicDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_ansonic_const.te_short) <
                subghz_protocol_ansonic_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_ansonic_const.te_long) <
                subghz_protocol_ansonic_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = AnsonicDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_ansonic_const.te_long) <
                 subghz_protocol_ansonic_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_ansonic_const.te_short) <
                 subghz_protocol_ansonic_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = AnsonicDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = AnsonicDecoderStepReset;
        } else {
            instance->decoder.parser_step = AnsonicDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_ansonic_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
 *        12345678(10) k   9                    
 * AAA => 10101010 1   01  0
 *
 * 1...10 - DIP
 * k- KEY
 */
    instance->cnt = instance->data & 0xFFF;
    instance->btn = ((instance->data >> 1) & 0x3);
}

uint8_t subghz_protocol_decoder_ansonic_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_ansonic_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_ansonic_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_ansonic_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_ansonic_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderAnsonic* instance = context;
    subghz_protocol_ansonic_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%03lX\r\n"
        "Btn:%X\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.btn,
        CNT_TO_DIP(instance->generic.cnt));
}
