#include "smc5326.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://datasheetspdf.com/pdf-file/532079/Aslic/AX5326-4/1
 *
 */

#define TAG "SubGhzProtocolSMC5326"

#define DIP_P 0b11 //(+)
#define DIP_O 0b10 //(0)
#define DIP_N 0b00 //(-)

#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define SHOW_DIP_P(dip, check_dip)                         \
    ((((dip >> 0xE) & 0x3) == check_dip) ? '*' : '_'),     \
        ((((dip >> 0xC) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0xA) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x8) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x6) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x4) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x2) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x0) & 0x3) == check_dip) ? '*' : '_')

static const SubGhzBlockConst subghz_protocol_smc5326_const = {
    .te_short = 300,
    .te_long = 900,
    .te_delta = 200,
    .min_count_bit_for_found = 25,
};

struct SubGhzProtocolDecoderSMC5326 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
    uint32_t last_data;
};

struct SubGhzProtocolEncoderSMC5326 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};

typedef enum {
    SMC5326DecoderStepReset = 0,
    SMC5326DecoderStepSaveDuration,
    SMC5326DecoderStepCheckDuration,
} SMC5326DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_smc5326_decoder = {
    .alloc = subghz_protocol_decoder_smc5326_alloc,
    .free = subghz_protocol_decoder_smc5326_free,

    .feed = subghz_protocol_decoder_smc5326_feed,
    .reset = subghz_protocol_decoder_smc5326_reset,

    .get_hash_data = subghz_protocol_decoder_smc5326_get_hash_data,
    .serialize = subghz_protocol_decoder_smc5326_serialize,
    .deserialize = subghz_protocol_decoder_smc5326_deserialize,
    .get_string = subghz_protocol_decoder_smc5326_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_smc5326_encoder = {
    .alloc = subghz_protocol_encoder_smc5326_alloc,
    .free = subghz_protocol_encoder_smc5326_free,

    .deserialize = subghz_protocol_encoder_smc5326_deserialize,
    .stop = subghz_protocol_encoder_smc5326_stop,
    .yield = subghz_protocol_encoder_smc5326_yield,
};

const SubGhzProtocol subghz_protocol_smc5326 = {
    .name = SUBGHZ_PROTOCOL_SMC5326_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_smc5326_decoder,
    .encoder = &subghz_protocol_smc5326_encoder,
};

void* subghz_protocol_encoder_smc5326_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderSMC5326* instance = malloc(sizeof(SubGhzProtocolEncoderSMC5326));

    instance->base.protocol = &subghz_protocol_smc5326;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_smc5326_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderSMC5326* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderSMC5326 instance
 * @return true On success
 */
static bool subghz_protocol_encoder_smc5326_get_upload(SubGhzProtocolEncoderSMC5326* instance) {
    furi_assert(instance);

    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)instance->te * 3);
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te);
        } else {
            //send bit 0
            instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)instance->te);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)instance->te * 3);
        }
    }

    //Send Stop bit
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)instance->te);
    //Send PT_GUARD
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te * 25);

    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_smc5326_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderSMC5326* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_smc5326_const.min_count_bit_for_found);
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

        if(!subghz_protocol_encoder_smc5326_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_smc5326_stop(void* context) {
    SubGhzProtocolEncoderSMC5326* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_smc5326_yield(void* context) {
    SubGhzProtocolEncoderSMC5326* instance = context;

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

void* subghz_protocol_decoder_smc5326_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderSMC5326* instance = malloc(sizeof(SubGhzProtocolDecoderSMC5326));
    instance->base.protocol = &subghz_protocol_smc5326;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_smc5326_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
    free(instance);
}

void subghz_protocol_decoder_smc5326_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
    instance->decoder.parser_step = SMC5326DecoderStepReset;
    instance->last_data = 0;
}

void subghz_protocol_decoder_smc5326_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;

    switch(instance->decoder.parser_step) {
    case SMC5326DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_smc5326_const.te_short * 24) <
                        subghz_protocol_smc5326_const.te_delta * 12)) {
            //Found Preambula
            instance->decoder.parser_step = SMC5326DecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = 0;
        }
        break;
    case SMC5326DecoderStepSaveDuration:
        //save duration
        if(level) {
            instance->decoder.te_last = duration;
            instance->te += duration;
            instance->decoder.parser_step = SMC5326DecoderStepCheckDuration;
        }
        break;
    case SMC5326DecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_smc5326_const.te_long * 2)) {
                instance->decoder.parser_step = SMC5326DecoderStepSaveDuration;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_smc5326_const.min_count_bit_for_found) {
                    if((instance->last_data == instance->decoder.decode_data) &&
                       instance->last_data) {
                        instance->te /= (instance->decoder.decode_count_bit * 4 + 1);

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
                break;
            }

            instance->te += duration;

            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_smc5326_const.te_short) <
                subghz_protocol_smc5326_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_smc5326_const.te_long) <
                subghz_protocol_smc5326_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = SMC5326DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_smc5326_const.te_long) <
                 subghz_protocol_smc5326_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_smc5326_const.te_short) <
                 subghz_protocol_smc5326_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = SMC5326DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = SMC5326DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = SMC5326DecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_smc5326_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_smc5326_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
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
    subghz_protocol_decoder_smc5326_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_smc5326_const.min_count_bit_for_found);
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

static void subghz_protocol_smc5326_get_event_serialize(uint8_t event, FuriString* output) {
    furi_string_cat_printf(
        output,
        "%s%s%s%s\r\n",
        (((event >> 6) & 0x3) == 0x3 ? "B1 " : ""),
        (((event >> 4) & 0x3) == 0x3 ? "B2 " : ""),
        (((event >> 2) & 0x3) == 0x3 ? "B3 " : ""),
        (((event >> 0) & 0x3) == 0x3 ? "B4 " : ""));
}

void subghz_protocol_decoder_smc5326_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderSMC5326* instance = context;
    uint32_t data = (uint32_t)((instance->generic.data >> 9) & 0xFFFF);

    furi_string_cat_printf(
        output,
        "%s %ubit\r\n"
        "Key:%07lX         Te:%luus\r\n"
        "  +:   " DIP_PATTERN "\r\n"
        "  o:   " DIP_PATTERN "    ",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0x1FFFFFF),
        instance->te,
        SHOW_DIP_P(data, DIP_P),
        SHOW_DIP_P(data, DIP_O));
    subghz_protocol_smc5326_get_event_serialize(instance->generic.data >> 1, output);
    furi_string_cat_printf(output, "  -:   " DIP_PATTERN "\r\n", SHOW_DIP_P(data, DIP_N));
}
