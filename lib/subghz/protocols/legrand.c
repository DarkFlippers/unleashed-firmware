#include "legrand.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolLegrand"

static const SubGhzBlockConst subghz_protocol_legrand_const = {
    .te_short = 375,
    .te_long = 1125,
    .te_delta = 150,
    .min_count_bit_for_found = 18,
};

struct SubGhzProtocolDecoderLegrand {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
    uint32_t last_data;
};

struct SubGhzProtocolEncoderLegrand {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};

typedef enum {
    LegrandDecoderStepReset = 0,
    LegrandDecoderStepFirstBit,
    LegrandDecoderStepSaveDuration,
    LegrandDecoderStepCheckDuration,
} LegrandDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_legrand_decoder = {
    .alloc = subghz_protocol_decoder_legrand_alloc,
    .free = subghz_protocol_decoder_legrand_free,

    .feed = subghz_protocol_decoder_legrand_feed,
    .reset = subghz_protocol_decoder_legrand_reset,

    .get_hash_data = subghz_protocol_decoder_legrand_get_hash_data,
    .serialize = subghz_protocol_decoder_legrand_serialize,
    .deserialize = subghz_protocol_decoder_legrand_deserialize,
    .get_string = subghz_protocol_decoder_legrand_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_legrand_encoder = {
    .alloc = subghz_protocol_encoder_legrand_alloc,
    .free = subghz_protocol_encoder_legrand_free,

    .deserialize = subghz_protocol_encoder_legrand_deserialize,
    .stop = subghz_protocol_encoder_legrand_stop,
    .yield = subghz_protocol_encoder_legrand_yield,
};

const SubGhzProtocol subghz_protocol_legrand = {
    .name = SUBGHZ_PROTOCOL_LEGRAND_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_legrand_decoder,
    .encoder = &subghz_protocol_legrand_encoder,
};

void* subghz_protocol_encoder_legrand_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderLegrand* instance = malloc(sizeof(SubGhzProtocolEncoderLegrand));

    instance->base.protocol = &subghz_protocol_legrand;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = subghz_protocol_legrand_const.min_count_bit_for_found * 2 + 1;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_legrand_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderLegrand* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderLegrand instance
 * @return true On success
 */
static bool subghz_protocol_encoder_legrand_get_upload(SubGhzProtocolEncoderLegrand* instance) {
    furi_assert(instance);

    size_t size_upload = (instance->generic.data_count_bit * 2) + 1;
    if(size_upload != instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Invalid data bit count");
        return false;
    }

    size_t index = 0;

    // Send sync
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te * 16);

    // Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // send bit 1
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)instance->te * 3);
        } else {
            // send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)instance->te * 3);
            instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)instance->te);
        }
    }

    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_legrand_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderLegrand* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_legrand_const.min_count_bit_for_found);
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
        // optional parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_legrand_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_legrand_stop(void* context) {
    SubGhzProtocolEncoderLegrand* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_legrand_yield(void* context) {
    SubGhzProtocolEncoderLegrand* instance = context;

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

void* subghz_protocol_decoder_legrand_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderLegrand* instance = malloc(sizeof(SubGhzProtocolDecoderLegrand));
    instance->base.protocol = &subghz_protocol_legrand;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_legrand_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;
    free(instance);
}

void subghz_protocol_decoder_legrand_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;
    instance->decoder.parser_step = LegrandDecoderStepReset;
    instance->last_data = 0;
}

void subghz_protocol_decoder_legrand_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;

    switch(instance->decoder.parser_step) {
    case LegrandDecoderStepReset:
        if(!level && DURATION_DIFF(duration, subghz_protocol_legrand_const.te_short * 16) <
                         subghz_protocol_legrand_const.te_delta * 8) {
            instance->decoder.parser_step = LegrandDecoderStepFirstBit;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = 0;
        }
        break;
    case LegrandDecoderStepFirstBit:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_legrand_const.te_short) <
               subghz_protocol_legrand_const.te_delta) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->te += duration * 4; // long low that is part of sync, then short high
            }

            if(DURATION_DIFF(duration, subghz_protocol_legrand_const.te_long) <
               subghz_protocol_legrand_const.te_delta * 3) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->te += duration / 3 * 4; // short low that is part of sync, then long high
            }

            if(instance->decoder.decode_count_bit > 0) {
                // advance to the next step if either short or long is found
                instance->decoder.parser_step = LegrandDecoderStepSaveDuration;
                break;
            }
        }

        instance->decoder.parser_step = LegrandDecoderStepReset;
        break;
    case LegrandDecoderStepSaveDuration:
        if(!level) {
            instance->decoder.te_last = duration;
            instance->te += duration;
            instance->decoder.parser_step = LegrandDecoderStepCheckDuration;
            break;
        }

        instance->decoder.parser_step = LegrandDecoderStepReset;
        break;
    case LegrandDecoderStepCheckDuration:
        if(level) {
            uint8_t found = 0;

            if(DURATION_DIFF(instance->decoder.te_last, subghz_protocol_legrand_const.te_long) <
                   subghz_protocol_legrand_const.te_delta * 3 &&
               DURATION_DIFF(duration, subghz_protocol_legrand_const.te_short) <
                   subghz_protocol_legrand_const.te_delta) {
                found = 1;
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            }

            if(DURATION_DIFF(instance->decoder.te_last, subghz_protocol_legrand_const.te_short) <
                   subghz_protocol_legrand_const.te_delta &&
               DURATION_DIFF(duration, subghz_protocol_legrand_const.te_long) <
                   subghz_protocol_legrand_const.te_delta * 3) {
                found = 1;
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            }

            if(found) {
                instance->te += duration;

                if(instance->decoder.decode_count_bit <
                   subghz_protocol_legrand_const.min_count_bit_for_found) {
                    instance->decoder.parser_step = LegrandDecoderStepSaveDuration;
                    break;
                }

                // enough bits for a packet found, save it only if there was a previous packet
                // with the same data
                if(instance->last_data && (instance->last_data == instance->decoder.decode_data)) {
                    instance->te /= instance->decoder.decode_count_bit * 4;

                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback) {
                        instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->last_data = instance->decoder.decode_data;
                // fallthrough to reset, the next bit is expected to be a sync
                // it also takes care of resetting the decoder state
            }
        }

        instance->decoder.parser_step = LegrandDecoderStepReset;
        break;
    }
}

uint8_t subghz_protocol_decoder_legrand_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_legrand_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;
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
    subghz_protocol_decoder_legrand_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_legrand_const.min_count_bit_for_found);
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

void subghz_protocol_decoder_legrand_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderLegrand* instance = context;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%05lX\r\n"
        "Te:%luus\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        instance->te);
}
