#include "clemsa.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

// protocol BERNER / ELKA / TEDSEN / TELETASTER
#define TAG "SubGhzProtocolClemsa"

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

static const SubGhzBlockConst subghz_protocol_clemsa_const = {
    .te_short = 385,
    .te_long = 2695,
    .te_delta = 150,
    .min_count_bit_for_found = 18,
};

struct SubGhzProtocolDecoderClemsa {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderClemsa {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    ClemsaDecoderStepReset = 0,
    ClemsaDecoderStepSaveDuration,
    ClemsaDecoderStepCheckDuration,
} ClemsaDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_clemsa_decoder = {
    .alloc = subghz_protocol_decoder_clemsa_alloc,
    .free = subghz_protocol_decoder_clemsa_free,

    .feed = subghz_protocol_decoder_clemsa_feed,
    .reset = subghz_protocol_decoder_clemsa_reset,

    .get_hash_data = subghz_protocol_decoder_clemsa_get_hash_data,
    .serialize = subghz_protocol_decoder_clemsa_serialize,
    .deserialize = subghz_protocol_decoder_clemsa_deserialize,
    .get_string = subghz_protocol_decoder_clemsa_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_clemsa_encoder = {
    .alloc = subghz_protocol_encoder_clemsa_alloc,
    .free = subghz_protocol_encoder_clemsa_free,

    .deserialize = subghz_protocol_encoder_clemsa_deserialize,
    .stop = subghz_protocol_encoder_clemsa_stop,
    .yield = subghz_protocol_encoder_clemsa_yield,
};

const SubGhzProtocol subghz_protocol_clemsa = {
    .name = SUBGHZ_PROTOCOL_CLEMSA_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_clemsa_decoder,
    .encoder = &subghz_protocol_clemsa_encoder,
};

void* subghz_protocol_encoder_clemsa_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderClemsa* instance = malloc(sizeof(SubGhzProtocolEncoderClemsa));

    instance->base.protocol = &subghz_protocol_clemsa;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_clemsa_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderClemsa* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderClemsa instance
 * @return true On success
 */
static bool subghz_protocol_encoder_clemsa_get_upload(SubGhzProtocolEncoderClemsa* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    for(uint8_t i = instance->generic.data_count_bit; i > 1; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_clemsa_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_clemsa_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_clemsa_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_clemsa_const.te_long);
        }
    }
    if(bit_read(instance->generic.data, 0)) {
        //send bit 1
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_clemsa_const.te_long);
        instance->encoder.upload[index++] = level_duration_make(
            false,
            (uint32_t)subghz_protocol_clemsa_const.te_short +
                subghz_protocol_clemsa_const.te_long * 7);
    } else {
        //send bit 0
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_clemsa_const.te_short);
        instance->encoder.upload[index++] = level_duration_make(
            false,
            (uint32_t)subghz_protocol_clemsa_const.te_long +
                subghz_protocol_clemsa_const.te_long * 7);
    }
    return true;
}

bool subghz_protocol_encoder_clemsa_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderClemsa* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_clemsa_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_clemsa_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_clemsa_stop(void* context) {
    SubGhzProtocolEncoderClemsa* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_clemsa_yield(void* context) {
    SubGhzProtocolEncoderClemsa* instance = context;

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

void* subghz_protocol_decoder_clemsa_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderClemsa* instance = malloc(sizeof(SubGhzProtocolDecoderClemsa));
    instance->base.protocol = &subghz_protocol_clemsa;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_clemsa_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    free(instance);
}

void subghz_protocol_decoder_clemsa_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    instance->decoder.parser_step = ClemsaDecoderStepReset;
}

void subghz_protocol_decoder_clemsa_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;

    switch(instance->decoder.parser_step) {
    case ClemsaDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_clemsa_const.te_short * 51) <
                        subghz_protocol_clemsa_const.te_delta * 25)) {
            instance->decoder.parser_step = ClemsaDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;

    case ClemsaDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = ClemsaDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = ClemsaDecoderStepReset;
        }
        break;

    case ClemsaDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_clemsa_const.te_short) <
                subghz_protocol_clemsa_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_clemsa_const.te_long) <
                subghz_protocol_clemsa_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = ClemsaDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_clemsa_const.te_long) <
                 subghz_protocol_clemsa_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_clemsa_const.te_short) <
                 subghz_protocol_clemsa_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = ClemsaDecoderStepSaveDuration;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_clemsa_const.te_short * 51) <
                subghz_protocol_clemsa_const.te_delta * 25) {
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_clemsa_const.te_short) <
                    subghz_protocol_clemsa_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else if((DURATION_DIFF(
                               instance->decoder.te_last, subghz_protocol_clemsa_const.te_long) <
                           subghz_protocol_clemsa_const.te_delta * 3)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else {
                    instance->decoder.parser_step = ClemsaDecoderStepReset;
                }

                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_clemsa_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.parser_step = ClemsaDecoderStepSaveDuration;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;

            } else {
                instance->decoder.parser_step = ClemsaDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = ClemsaDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_clemsa_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = (instance->data >> 2) & 0xFFFF;
    instance->btn = (instance->data & 0x03);
}

uint8_t subghz_protocol_decoder_clemsa_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_clemsa_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_clemsa_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_clemsa_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_clemsa_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderClemsa* instance = context;
    subghz_protocol_clemsa_check_remote_controller(&instance->generic);
    //uint32_t data = (uint32_t)(instance->generic.data & 0xFFFFFF);
    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%05lX   Btn %X\r\n"
        "  +:   " DIP_PATTERN "\r\n"
        "  o:   " DIP_PATTERN "\r\n"
        "  -:   " DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0x3FFFF),
        instance->generic.btn,
        SHOW_DIP_P(instance->generic.serial, DIP_P),
        SHOW_DIP_P(instance->generic.serial, DIP_O),
        SHOW_DIP_P(instance->generic.serial, DIP_N));
}
