#include "dooya.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolDOOYA"

static const SubGhzBlockConst subghz_protocol_dooya_const = {
    .te_short = 300,
    .te_long = 750,
    .te_delta = 100,
    .min_count_bit_for_found = 39,
};

struct SubGhzProtocolDecoderDooya {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderDooya {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    DooyaDecoderStepReset = 0,
    DooyaDecoderStepFoundStartBit,
    DooyaDecoderStepSaveDuration,
    DooyaDecoderStepCheckDuration,
} DooyaDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_dooya_decoder = {
    .alloc = subghz_protocol_decoder_dooya_alloc,
    .free = subghz_protocol_decoder_dooya_free,

    .feed = subghz_protocol_decoder_dooya_feed,
    .reset = subghz_protocol_decoder_dooya_reset,

    .get_hash_data = subghz_protocol_decoder_dooya_get_hash_data,
    .serialize = subghz_protocol_decoder_dooya_serialize,
    .deserialize = subghz_protocol_decoder_dooya_deserialize,
    .get_string = subghz_protocol_decoder_dooya_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_dooya_encoder = {
    .alloc = subghz_protocol_encoder_dooya_alloc,
    .free = subghz_protocol_encoder_dooya_free,

    .deserialize = subghz_protocol_encoder_dooya_deserialize,
    .stop = subghz_protocol_encoder_dooya_stop,
    .yield = subghz_protocol_encoder_dooya_yield,
};

const SubGhzProtocol subghz_protocol_dooya = {
    .name = SUBGHZ_PROTOCOL_DOOYA_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_dooya_decoder,
    .encoder = &subghz_protocol_dooya_encoder,
};

void* subghz_protocol_encoder_dooya_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderDooya* instance = malloc(sizeof(SubGhzProtocolEncoderDooya));

    instance->base.protocol = &subghz_protocol_dooya;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_dooya_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderDooya* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderDooya instance
 * @return true On success
 */
static bool subghz_protocol_encoder_dooya_get_upload(SubGhzProtocolEncoderDooya* instance) {
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
    if(bit_read(instance->generic.data, 0)) {
        instance->encoder.upload[index++] = level_duration_make(
            false,
            (uint32_t)subghz_protocol_dooya_const.te_long * 10 +
                subghz_protocol_dooya_const.te_long);
    } else {
        instance->encoder.upload[index++] = level_duration_make(
            false,
            (uint32_t)subghz_protocol_dooya_const.te_long * 10 +
                subghz_protocol_dooya_const.te_short);
    }

    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_dooya_const.te_short * 16);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_dooya_const.te_long * 2);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_dooya_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_dooya_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_dooya_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_dooya_const.te_long);
        }
    }
    return true;
}

bool subghz_protocol_encoder_dooya_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderDooya* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit - 1 !=
           subghz_protocol_dooya_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_dooya_get_upload(instance)) break;
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_dooya_stop(void* context) {
    SubGhzProtocolEncoderDooya* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_dooya_yield(void* context) {
    SubGhzProtocolEncoderDooya* instance = context;

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

void* subghz_protocol_decoder_dooya_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderDooya* instance = malloc(sizeof(SubGhzProtocolDecoderDooya));
    instance->base.protocol = &subghz_protocol_dooya;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_dooya_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;
    free(instance);
}

static bool subghz_protocol_decoder_dooya_check_parity(SubGhzProtocolDecoderDooya* instance) {
    uint8_t msg[] = {
        instance->decoder.decode_data >> 33,
        instance->decoder.decode_data >> 25,
        instance->decoder.decode_data >> 17,
        instance->decoder.decode_data >> 9,
        instance->decoder.decode_data >> 1};
    return subghz_protocol_blocks_parity_bytes(msg, 5) !=
           (uint8_t)(instance->decoder.decode_data & 1);
}

void subghz_protocol_decoder_dooya_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;
    instance->decoder.parser_step = DooyaDecoderStepReset;
}

void subghz_protocol_decoder_dooya_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;

    switch(instance->decoder.parser_step) {
    case DooyaDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_dooya_const.te_long * 10) <
                        subghz_protocol_dooya_const.te_delta * 10)) {
            instance->decoder.parser_step = DooyaDecoderStepFoundStartBit;
        }
        break;

    case DooyaDecoderStepFoundStartBit:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_dooya_const.te_long * 2) <
               subghz_protocol_dooya_const.te_delta * 3) {
                instance->decoder.parser_step = DooyaDecoderStepSaveDuration;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
            } else {
                instance->decoder.parser_step = DooyaDecoderStepReset;
            }
        } else if(
            DURATION_DIFF(duration, subghz_protocol_dooya_const.te_long * 6) <
            subghz_protocol_dooya_const.te_delta * 10) {
            break;
        } else {
            instance->decoder.parser_step = DooyaDecoderStepReset;
        }
        break;

    case DooyaDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = DooyaDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = DooyaDecoderStepReset;
        }
        break;

    case DooyaDecoderStepCheckDuration:
        if(!level) {
            if(duration >= (subghz_protocol_dooya_const.te_long * 4)) {
                instance->decoder.parser_step = DooyaDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_dooya_const.min_count_bit_for_found) {
                    //add last bit
                    if(DURATION_DIFF(
                           instance->decoder.te_last, subghz_protocol_dooya_const.te_short) <
                       subghz_protocol_dooya_const.te_delta) {
                        subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                        instance->decoder.parser_step = DooyaDecoderStepSaveDuration;
                    } else if(
                        DURATION_DIFF(
                            instance->decoder.te_last, subghz_protocol_dooya_const.te_long) <
                        subghz_protocol_dooya_const.te_delta * 2) {
                        subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                        instance->decoder.parser_step = DooyaDecoderStepSaveDuration;
                    } else {
                        instance->decoder.parser_step = DooyaDecoderStepReset;
                        break;
                    }

                    if(subghz_protocol_decoder_dooya_check_parity(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                break;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_dooya_const.te_short) <
                 subghz_protocol_dooya_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_dooya_const.te_long) <
                 subghz_protocol_dooya_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = DooyaDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_dooya_const.te_long) <
                 subghz_protocol_dooya_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_dooya_const.te_short) <
                 subghz_protocol_dooya_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = DooyaDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = DooyaDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = DooyaDecoderStepReset;
        }
        break;
    }
}

/**
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_somfy_telis_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
 * 																serial + channel      k  r	x  !p
 * long press down   X * E1DC030533, 40b 			111000011101110000000011000001010 01 10 01 1
 *
 * short press down  3 * E1DC030533, 40b			111000011101110000000011000001010 01 10 01 1
 *                   3 * E1DC03053C, 40b 	        111000011101110000000011000001010 01 11 10 0
 *
 * press stop        X * E1DC030555, 40b			111000011101110000000011000001010 10 10 10 1
 *
 * long press up     X * E1DC030511, 40b			111000011101110000000011000001010 00 10 00 1
 *
 * short press up    3 * E1DC030511, 40b			111000011101110000000011000001010 00 10 00 1
 *                   3 * E1DC03051E, 40b		    111000011101110000000011000001010 00 11 11 0
 *
 * serial + channel: serial + channel (need more information)
 * k: key  00b - up, 01b - down, 10b - stop, 11 - unknown
 * r: 10b - long press 11b - short press
 * x: unknown
 * !p: invert parity
 *
*/

    instance->serial = (instance->data >> 7);
    instance->btn = (instance->data >> 5) & 0x3;
    instance->cnt = (instance->data >> 3) & 0x3;
}

uint8_t subghz_protocol_decoder_dooya_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_dooya_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_dooya_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit - 1 !=
           subghz_protocol_dooya_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

/**
 * Get button name.
 * @param btn Button number, 2 bit
 */
static const char* subghz_protocol_dooya_get_name_button(uint8_t btn) {
    const char* name_btn[0x04] = {"UP", "Down", "Stop", "Unknown"};
    return btn < 0x04 ? name_btn[btn] : name_btn[4];
}

/**
 * Get time press button.
 * @param cnt Cnt number, 2 bit
 */
static const char* subghz_protocol_dooya_get_time_press_button(uint8_t cnt) {
    const char* name_cnt[0x04] = {"Unknown", "Unknown", "Long", "Short"};
    return cnt < 0x04 ? name_cnt[cnt] : name_cnt[0];
}

void subghz_protocol_decoder_dooya_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderDooya* instance = context;

    subghz_protocol_somfy_telis_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%010llX\r\n"
        "Sn:0x%08lX\r\n"
        "Btn:%s Press:%s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.data,
        instance->generic.serial,
        subghz_protocol_dooya_get_name_button(instance->generic.btn),
        subghz_protocol_dooya_get_time_press_button(instance->generic.cnt));
}
