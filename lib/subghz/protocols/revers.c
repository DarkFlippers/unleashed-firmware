#include "revers.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolRevers"

static const SubGhzBlockConst subghz_protocol_revers_const = {
    .te_short = 200,
    .te_long = 430,
    .te_delta = 80,
    .min_count_bit_for_found = 8,
};

struct SubGhzProtocolDecoderRevers {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t last_data;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderRevers {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    ReversDecoderStepReset = 0,
    ReversDecoderStepCheckPreambula,
    ReversDecoderStepSaveDuration,
    ReversDecoderStepCheckDuration,
} ReversDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_revers_decoder = {
    .alloc = subghz_protocol_decoder_revers_alloc,
    .free = subghz_protocol_decoder_revers_free,

    .feed = subghz_protocol_decoder_revers_feed,
    .reset = subghz_protocol_decoder_revers_reset,

    .get_hash_data = subghz_protocol_decoder_revers_get_hash_data,
    .serialize = subghz_protocol_decoder_revers_serialize,
    .deserialize = subghz_protocol_decoder_revers_deserialize,
    .get_string = subghz_protocol_decoder_revers_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_revers_encoder = {
    .alloc = subghz_protocol_encoder_revers_alloc,
    .free = subghz_protocol_encoder_revers_free,

    .deserialize = subghz_protocol_encoder_revers_deserialize,
    .stop = subghz_protocol_encoder_revers_stop,
    .yield = subghz_protocol_encoder_revers_yield,
};

const SubGhzProtocol subghz_protocol_revers = {
    .name = SUBGHZ_PROTOCOL_REVERS_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_revers_decoder,
    .encoder = &subghz_protocol_revers_encoder,
};

void* subghz_protocol_encoder_revers_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderRevers* instance = malloc(sizeof(SubGhzProtocolEncoderRevers));

    instance->base.protocol = &subghz_protocol_revers;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 512;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_revers_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderRevers* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderRevers instance
 * @return true On success
 */
static bool subghz_protocol_encoder_revers_get_upload(SubGhzProtocolEncoderRevers* instance) {
    furi_assert(instance);
    size_t index = 0;
    /*size_t size_upload = instance->generic.data_count_bit;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }*/

    //Send key data
    /*for(uint8_t i = instance->generic.data_count_bit; i > 1; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_revers_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_revers_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_revers_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_revers_const.te_long);
        }
    }*/

    return true;
}

bool subghz_protocol_encoder_revers_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderRevers* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_revers_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_revers_get_upload(instance)) break;
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_revers_stop(void* context) {
    SubGhzProtocolEncoderRevers* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_revers_yield(void* context) {
    SubGhzProtocolEncoderRevers* instance = context;

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

void* subghz_protocol_decoder_revers_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderRevers* instance = malloc(sizeof(SubGhzProtocolDecoderRevers));
    instance->base.protocol = &subghz_protocol_revers;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_revers_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    free(instance);
}

void subghz_protocol_decoder_revers_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    instance->decoder.parser_step = ReversDecoderStepReset;
    instance->last_data = 0;
}

void subghz_protocol_decoder_revers_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    switch(instance->decoder.parser_step) {
    case ReversDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_revers_const.te_short) <
                          subghz_protocol_revers_const.te_delta) {
            instance->decoder.parser_step = ReversDecoderStepCheckPreambula;
            instance->header_count++;
        }
        break;
    case ReversDecoderStepCheckPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_revers_const.te_short) <
                        subghz_protocol_revers_const.te_delta)) {
            instance->decoder.parser_step = ReversDecoderStepReset;
            break;
        }
        if((instance->header_count > 2) &&
           (DURATION_DIFF(duration, subghz_protocol_revers_const.te_long) <
            subghz_protocol_revers_const.te_delta)) {
            // Preambula found
            instance->decoder.parser_step = ReversDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = ReversDecoderStepReset;
            instance->header_count = 0;
        }
    case ReversDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = ReversDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = ReversDecoderStepReset;
        }
        break;
    case ReversDecoderStepCheckDuration:
        if(!level) {
            /*if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_revers_const.te_short) <
                subghz_protocol_revers_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_revers_const.te_short * 7) <
                subghz_protocol_revers_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = ReversDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_revers_const.te_long) <
                 subghz_protocol_revers_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_revers_const.te_long) <
                 subghz_protocol_revers_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = ReversDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = ReversDecoderStepReset;
            }*/

        } else {
            instance->decoder.parser_step = ReversDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_revers_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8));
}

bool subghz_protocol_decoder_revers_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_revers_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_revers_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_revers_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers* instance = context;

    uint32_t data = instance->generic.data & 0xFF;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        data);
}
