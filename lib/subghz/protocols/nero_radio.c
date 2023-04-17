#include "nero_radio.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolNeroRadio"

static const SubGhzBlockConst subghz_protocol_nero_radio_const = {
    .te_short = 200,
    .te_long = 400,
    .te_delta = 80,
    .min_count_bit_for_found = 56,
};

struct SubGhzProtocolDecoderNeroRadio {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
};

struct SubGhzProtocolEncoderNeroRadio {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    NeroRadioDecoderStepReset = 0,
    NeroRadioDecoderStepCheckPreambula,
    NeroRadioDecoderStepSaveDuration,
    NeroRadioDecoderStepCheckDuration,
} NeroRadioDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_nero_radio_decoder = {
    .alloc = subghz_protocol_decoder_nero_radio_alloc,
    .free = subghz_protocol_decoder_nero_radio_free,

    .feed = subghz_protocol_decoder_nero_radio_feed,
    .reset = subghz_protocol_decoder_nero_radio_reset,

    .get_hash_data = subghz_protocol_decoder_nero_radio_get_hash_data,
    .serialize = subghz_protocol_decoder_nero_radio_serialize,
    .deserialize = subghz_protocol_decoder_nero_radio_deserialize,
    .get_string = subghz_protocol_decoder_nero_radio_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nero_radio_encoder = {
    .alloc = subghz_protocol_encoder_nero_radio_alloc,
    .free = subghz_protocol_encoder_nero_radio_free,

    .deserialize = subghz_protocol_encoder_nero_radio_deserialize,
    .stop = subghz_protocol_encoder_nero_radio_stop,
    .yield = subghz_protocol_encoder_nero_radio_yield,
};

const SubGhzProtocol subghz_protocol_nero_radio = {
    .name = SUBGHZ_PROTOCOL_NERO_RADIO_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_nero_radio_decoder,
    .encoder = &subghz_protocol_nero_radio_encoder,
};

void* subghz_protocol_encoder_nero_radio_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderNeroRadio* instance = malloc(sizeof(SubGhzProtocolEncoderNeroRadio));

    instance->base.protocol = &subghz_protocol_nero_radio;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_nero_radio_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroRadio* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderNeroRadio instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_nero_radio_get_upload(SubGhzProtocolEncoderNeroRadio* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = 49 * 2 + 2 + (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 0; i < 49; i++) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);
    }

    //Send start bit
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)830);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 1; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_long);
        }
    }
    if(bit_read(instance->generic.data, 0)) {
        //send bit 1
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_long);
        if(instance->generic.data_count_bit == 57) {
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)1300);
        } else {
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_nero_radio_const.te_short * 23);
        }
    } else {
        //send bit 0
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        if(instance->generic.data_count_bit == 57) {
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)1300);
        } else {
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_nero_radio_const.te_short * 23);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroRadio* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_nero_radio_const.min_count_bit_for_found);

        if((ret == SubGhzProtocolStatusErrorValueBitCount) &&
           (instance->generic.data_count_bit == 57)) {
            ret = SubGhzProtocolStatusOk;
        } else {
            if(ret != SubGhzProtocolStatusOk) {
                break;
            }
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_nero_radio_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_nero_radio_stop(void* context) {
    SubGhzProtocolEncoderNeroRadio* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_nero_radio_yield(void* context) {
    SubGhzProtocolEncoderNeroRadio* instance = context;

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

void* subghz_protocol_decoder_nero_radio_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderNeroRadio* instance = malloc(sizeof(SubGhzProtocolDecoderNeroRadio));
    instance->base.protocol = &subghz_protocol_nero_radio;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_nero_radio_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    free(instance);
}

void subghz_protocol_decoder_nero_radio_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    instance->decoder.parser_step = NeroRadioDecoderStepReset;
}

void subghz_protocol_decoder_nero_radio_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;

    switch(instance->decoder.parser_step) {
    case NeroRadioDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                       subghz_protocol_nero_radio_const.te_delta)) {
            instance->decoder.parser_step = NeroRadioDecoderStepCheckPreambula;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;
    case NeroRadioDecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                subghz_protocol_nero_radio_const.te_delta) ||
               (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short * 4) <
                subghz_protocol_nero_radio_const.te_delta)) {
                instance->decoder.te_last = duration;
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else if(
            DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
            subghz_protocol_nero_radio_const.te_delta) {
            if(DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
               subghz_protocol_nero_radio_const.te_delta) {
                // Found header
                instance->header_count++;
                break;
            } else if(
                DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short * 4) <
                subghz_protocol_nero_radio_const.te_delta) {
                // Found start bit
                if(instance->header_count > 40) {
                    instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                } else {
                    instance->decoder.parser_step = NeroRadioDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = NeroRadioDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)1250)) {
                //Found stop bit
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
                   subghz_protocol_nero_radio_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_nero_radio_const.te_long) <
                    subghz_protocol_nero_radio_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_nero_radio_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit ==
                    subghz_protocol_nero_radio_const.min_count_bit_for_found + 1)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = NeroRadioDecoderStepReset; //-V1048
                break;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
                 subghz_protocol_nero_radio_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_long) <
                 subghz_protocol_nero_radio_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nero_radio_const.te_long) <
                 subghz_protocol_nero_radio_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                 subghz_protocol_nero_radio_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_nero_radio_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_nero_radio_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    SubGhzProtocolStatus stat;

    stat = subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_nero_radio_const.min_count_bit_for_found);

    if((stat == SubGhzProtocolStatusErrorValueBitCount) &&
       (instance->generic.data_count_bit == 57)) {
        return SubGhzProtocolStatusOk;
    } else {
        return stat;
    }
}

void subghz_protocol_decoder_nero_radio_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Yek:0x%lX%08lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        code_found_reverse_lo);
}
