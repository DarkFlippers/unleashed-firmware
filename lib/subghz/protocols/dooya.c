#include "dooya.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolDooya"

#define DOYA_SINGLE_CHANNEL 0xFF

static const SubGhzBlockConst subghz_protocol_dooya_const = {
    .te_short = 366,
    .te_long = 733,
    .te_delta = 120,
    .min_count_bit_for_found = 40,
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
            (uint32_t)subghz_protocol_dooya_const.te_long * 12 +
                subghz_protocol_dooya_const.te_long);
    } else {
        instance->encoder.upload[index++] = level_duration_make(
            false,
            (uint32_t)subghz_protocol_dooya_const.te_long * 12 +
                subghz_protocol_dooya_const.te_short);
    }

    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_dooya_const.te_short * 13);
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
        if(instance->generic.data_count_bit !=
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
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_dooya_const.te_long * 12) <
                        subghz_protocol_dooya_const.te_delta * 20)) {
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
            DURATION_DIFF(duration, subghz_protocol_dooya_const.te_short * 13) <
            subghz_protocol_dooya_const.te_delta * 5) {
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
                //add last bit
                if(DURATION_DIFF(instance->decoder.te_last, subghz_protocol_dooya_const.te_short) <
                   subghz_protocol_dooya_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else if(
                    DURATION_DIFF(instance->decoder.te_last, subghz_protocol_dooya_const.te_long) <
                    subghz_protocol_dooya_const.te_delta * 2) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else {
                    instance->decoder.parser_step = DooyaDecoderStepReset;
                    break;
                }
                instance->decoder.parser_step = DooyaDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_dooya_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
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
 * 																serial       s/m  ch      key   	
 * long press down   X * E1DC030533, 40b 			111000011101110000000011 0000 0101 0011 0011
 * 
 * short press down  3 * E1DC030533, 40b			111000011101110000000011 0000 0101 0011 0011
 *                   3 * E1DC03053C, 40b 	        111000011101110000000011 0000 0101 0011 1100
 * 	
 * press stop        X * E1DC030555, 40b			111000011101110000000011 0000 0101 0101 0101
 * 
 * long press up     X * E1DC030511, 40b			111000011101110000000011 0000 0101 0001 0001
 * 
 * short press up    3 * E1DC030511, 40b			111000011101110000000011 0000 0101 0001 0001
 *                   3 * E1DC03051E, 40b		    111000011101110000000011 0000 0101 0001 1110
 *
 * serial: 3 byte serial number
 * s/m: single (b0000) / multi (b0001) channel console  
 * ch: channel if single (always b0101) or multi 
 * key: 0b00010001 - long press up
 *      0b00011110 - short press up
 *      0b00110011 - long press down
 *      0b00111100 - short press down
 *      0b01010101 - press stop 
 *      0b01111001 - press up + down
 *      0b10000000 - press up + stop
 *      0b10000001 - press down + stop
 *      0b11001100 - press P2
 *      
*/

    instance->serial = (instance->data >> 16);
    if((instance->data >> 12) & 0x0F) {
        instance->cnt = (instance->data >> 8) & 0x0F;
    } else {
        instance->cnt = DOYA_SINGLE_CHANNEL;
    }
    instance->btn = instance->data & 0xFF;
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
        if(instance->generic.data_count_bit !=
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
 * @param btn Button number, 8 bit
 */
static const char* subghz_protocol_dooya_get_name_button(uint8_t btn) {
    const char* btn_name;
    switch(btn) {
    case 0b00010001:
        btn_name = "Up_Long";
        break;
    case 0b00011110:
        btn_name = "Up_Short";
        break;
    case 0b00110011:
        btn_name = "Down_Long";
        break;
    case 0b00111100:
        btn_name = "Down_Short";
        break;
    case 0b01010101:
        btn_name = "Stop";
        break;
    case 0b01111001:
        btn_name = "Up+Down";
        break;
    case 0b10000000:
        btn_name = "Up+Stop";
        break;
    case 0b10000001:
        btn_name = "Down+Stop";
        break;
    case 0b11001100:
        btn_name = "P2";
        break;
    default:
        btn_name = "Unknown";
        break;
    }
    return btn_name;
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
        "Btn:%s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.data,
        instance->generic.serial,
        subghz_protocol_dooya_get_name_button(instance->generic.btn));
    if(instance->generic.cnt == DOYA_SINGLE_CHANNEL) {
        furi_string_cat_printf(output, "Ch:Single\r\n");
    } else {
        furi_string_cat_printf(output, "Ch:%lu\r\n", instance->generic.cnt);
    }
}
