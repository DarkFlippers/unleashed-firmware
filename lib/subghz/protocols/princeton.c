#include "princeton.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

#define TAG "SubGhzProtocolPrinceton"

static const SubGhzBlockConst subghz_protocol_princeton_const = {
    .te_short = 400,
    .te_long = 1200,
    .te_delta = 300,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderPrinceton {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
    uint32_t last_data;
};

struct SubGhzProtocolEncoderPrinceton {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};

typedef enum {
    PrincetonDecoderStepReset = 0,
    PrincetonDecoderStepSaveDuration,
    PrincetonDecoderStepCheckDuration,
} PrincetonDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_princeton_decoder = {
    .alloc = subghz_protocol_decoder_princeton_alloc,
    .free = subghz_protocol_decoder_princeton_free,

    .feed = subghz_protocol_decoder_princeton_feed,
    .reset = subghz_protocol_decoder_princeton_reset,

    .get_hash_data = subghz_protocol_decoder_princeton_get_hash_data,
    .serialize = subghz_protocol_decoder_princeton_serialize,
    .deserialize = subghz_protocol_decoder_princeton_deserialize,
    .get_string = subghz_protocol_decoder_princeton_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_princeton_encoder = {
    .alloc = subghz_protocol_encoder_princeton_alloc,
    .free = subghz_protocol_encoder_princeton_free,

    .deserialize = subghz_protocol_encoder_princeton_deserialize,
    .stop = subghz_protocol_encoder_princeton_stop,
    .yield = subghz_protocol_encoder_princeton_yield,
};

const SubGhzProtocol subghz_protocol_princeton = {
    .name = SUBGHZ_PROTOCOL_PRINCETON_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_princeton_decoder,
    .encoder = &subghz_protocol_princeton_encoder,
};

void* subghz_protocol_encoder_princeton_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderPrinceton* instance = malloc(sizeof(SubGhzProtocolEncoderPrinceton));

    instance->base.protocol = &subghz_protocol_princeton;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52; //max 24bit*2 + 2 (start, stop)
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_princeton_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderPrinceton* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderPrinceton instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_princeton_get_upload(SubGhzProtocolEncoderPrinceton* instance) {
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
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)instance->te * 30);

    return true;
}

bool subghz_protocol_encoder_princeton_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderPrinceton* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "TE", (uint32_t*)&instance->te, 1)) {
            FURI_LOG_E(TAG, "Missing TE");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_princeton_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_princeton_get_upload(instance);
        instance->encoder.is_runing = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_princeton_stop(void* context) {
    SubGhzProtocolEncoderPrinceton* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_princeton_yield(void* context) {
    SubGhzProtocolEncoderPrinceton* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_runing) {
        instance->encoder.is_runing = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_princeton_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderPrinceton* instance = malloc(sizeof(SubGhzProtocolDecoderPrinceton));
    instance->base.protocol = &subghz_protocol_princeton;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_princeton_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;
    free(instance);
}

void subghz_protocol_decoder_princeton_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;
    instance->decoder.parser_step = PrincetonDecoderStepReset;
    instance->last_data = 0;
}

void subghz_protocol_decoder_princeton_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;

    switch(instance->decoder.parser_step) {
    case PrincetonDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_princeton_const.te_short * 36) <
                        subghz_protocol_princeton_const.te_delta * 36)) {
            //Found Preambula
            instance->decoder.parser_step = PrincetonDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = 0;
        }
        break;
    case PrincetonDecoderStepSaveDuration:
        //save duration
        if(level) {
            instance->decoder.te_last = duration;
            instance->te += duration;
            instance->decoder.parser_step = PrincetonDecoderStepCheckDuration;
        }
        break;
    case PrincetonDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_princeton_const.te_short * 10 +
                            subghz_protocol_princeton_const.te_delta)) {
                instance->decoder.parser_step = PrincetonDecoderStepSaveDuration;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_princeton_const.min_count_bit_for_found) {
                    if((instance->last_data == instance->decoder.decode_data) &&
                       instance->last_data) {
                        instance->te /= (instance->decoder.decode_count_bit * 4 + 1);

                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        instance->generic.serial = instance->decoder.decode_data >> 4;
                        instance->generic.btn = (uint8_t)instance->decoder.decode_data & 0x00000F;

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

            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_princeton_const.te_short) <
                subghz_protocol_princeton_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_princeton_const.te_long) <
                subghz_protocol_princeton_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = PrincetonDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_princeton_const.te_long) <
                 subghz_protocol_princeton_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_princeton_const.te_short) <
                 subghz_protocol_princeton_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = PrincetonDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = PrincetonDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = PrincetonDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_princeton_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_princeton_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;
    bool res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if(res && !flipper_format_write_uint32(flipper_format, "TE", &instance->te, 1)) {
        FURI_LOG_E(TAG, "Unable to add TE");
        res = false;
    }
    return res;
}

bool subghz_protocol_decoder_princeton_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_princeton_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "TE", (uint32_t*)&instance->te, 1)) {
            FURI_LOG_E(TAG, "Missing TE");
            break;
        }
        res = true;
    } while(false);

    return res;
}

void subghz_protocol_decoder_princeton_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderPrinceton* instance = context;

    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%08lX\r\n"
        "Yek:0x%08lX\r\n"
        "Sn:0x%05lX BTN:%02X\r\n"
        "Te:%dus\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_lo,
        code_found_reverse_lo,
        instance->generic.serial,
        instance->generic.btn,
        instance->te);
}
