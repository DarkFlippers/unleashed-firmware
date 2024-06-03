#include "intertechno_v3.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolIntertechnoV3"

#define CH_PATTERN "%c%c%c%c"
#define CNT_TO_CH(ch) \
    (ch & 0x8 ? '1' : '0'), (ch & 0x4 ? '1' : '0'), (ch & 0x2 ? '1' : '0'), (ch & 0x1 ? '1' : '0')

#define INTERTECHNO_V3_DIMMING_COUNT_BIT 36

static const SubGhzBlockConst subghz_protocol_intertechno_v3_const = {
    .te_short = 275,
    .te_long = 1375,
    .te_delta = 150,
    .min_count_bit_for_found = 32,
};

struct SubGhzProtocolDecoderIntertechno_V3 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderIntertechno_V3 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    IntertechnoV3DecoderStepReset = 0,
    IntertechnoV3DecoderStepStartSync,
    IntertechnoV3DecoderStepFoundSync,
    IntertechnoV3DecoderStepStartDuration,
    IntertechnoV3DecoderStepSaveDuration,
    IntertechnoV3DecoderStepCheckDuration,
    IntertechnoV3DecoderStepEndDuration,
} IntertechnoV3DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_intertechno_v3_decoder = {
    .alloc = subghz_protocol_decoder_intertechno_v3_alloc,
    .free = subghz_protocol_decoder_intertechno_v3_free,

    .feed = subghz_protocol_decoder_intertechno_v3_feed,
    .reset = subghz_protocol_decoder_intertechno_v3_reset,

    .get_hash_data = subghz_protocol_decoder_intertechno_v3_get_hash_data,
    .serialize = subghz_protocol_decoder_intertechno_v3_serialize,
    .deserialize = subghz_protocol_decoder_intertechno_v3_deserialize,
    .get_string = subghz_protocol_decoder_intertechno_v3_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_intertechno_v3_encoder = {
    .alloc = subghz_protocol_encoder_intertechno_v3_alloc,
    .free = subghz_protocol_encoder_intertechno_v3_free,

    .deserialize = subghz_protocol_encoder_intertechno_v3_deserialize,
    .stop = subghz_protocol_encoder_intertechno_v3_stop,
    .yield = subghz_protocol_encoder_intertechno_v3_yield,
};

const SubGhzProtocol subghz_protocol_intertechno_v3 = {
    .name = SUBGHZ_PROTOCOL_INTERTECHNO_V3_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_intertechno_v3_decoder,
    .encoder = &subghz_protocol_intertechno_v3_encoder,
};

void* subghz_protocol_encoder_intertechno_v3_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderIntertechno_V3* instance =
        malloc(sizeof(SubGhzProtocolEncoderIntertechno_V3));

    instance->base.protocol = &subghz_protocol_intertechno_v3;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_intertechno_v3_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderIntertechno_V3* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderIntertechno_V3 instance
 * @return true On success
 */
static bool subghz_protocol_encoder_intertechno_v3_get_upload(
    SubGhzProtocolEncoderIntertechno_V3* instance) {
    furi_assert(instance);
    size_t index = 0;

    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short * 38);
    //Send sync
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short * 10);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if((instance->generic.data_count_bit == INTERTECHNO_V3_DIMMING_COUNT_BIT) && (i == 9)) {
            //send bit dimm
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
        } else if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_intertechno_v3_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_intertechno_v3_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_intertechno_v3_const.te_long);
        }
    }
    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus subghz_protocol_encoder_intertechno_v3_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderIntertechno_V3* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_intertechno_v3_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit != INTERTECHNO_V3_DIMMING_COUNT_BIT)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            ret = SubGhzProtocolStatusErrorValueBitCount;
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_intertechno_v3_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_intertechno_v3_stop(void* context) {
    SubGhzProtocolEncoderIntertechno_V3* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_intertechno_v3_yield(void* context) {
    SubGhzProtocolEncoderIntertechno_V3* instance = context;

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

void* subghz_protocol_decoder_intertechno_v3_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderIntertechno_V3* instance =
        malloc(sizeof(SubGhzProtocolDecoderIntertechno_V3));
    instance->base.protocol = &subghz_protocol_intertechno_v3;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_intertechno_v3_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    free(instance);
}

void subghz_protocol_decoder_intertechno_v3_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
}

void subghz_protocol_decoder_intertechno_v3_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    switch(instance->decoder.parser_step) {
    case IntertechnoV3DecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short * 37) <
            subghz_protocol_intertechno_v3_const.te_delta * 15)) {
            instance->decoder.parser_step = IntertechnoV3DecoderStepStartSync;
        }
        break;
    case IntertechnoV3DecoderStepStartSync:
        if(level && (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                     subghz_protocol_intertechno_v3_const.te_delta)) {
            instance->decoder.parser_step = IntertechnoV3DecoderStepFoundSync;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;

    case IntertechnoV3DecoderStepFoundSync:
        if(!level && (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short * 10) <
                      subghz_protocol_intertechno_v3_const.te_delta * 3)) {
            instance->decoder.parser_step = IntertechnoV3DecoderStepStartDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;

    case IntertechnoV3DecoderStepStartDuration:
        if(level && (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                     subghz_protocol_intertechno_v3_const.te_delta)) {
            instance->decoder.parser_step = IntertechnoV3DecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;

    case IntertechnoV3DecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_intertechno_v3_const.te_short * 11)) {
                instance->decoder.parser_step = IntertechnoV3DecoderStepStartSync;
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_intertechno_v3_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit == INTERTECHNO_V3_DIMMING_COUNT_BIT)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = IntertechnoV3DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;
    case IntertechnoV3DecoderStepCheckDuration:
        if(level) {
            //Add 0 bit
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_intertechno_v3_const.te_short) <
                subghz_protocol_intertechno_v3_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                subghz_protocol_intertechno_v3_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = IntertechnoV3DecoderStepEndDuration;
            } else if(
                //Add 1 bit
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_intertechno_v3_const.te_long) <
                 subghz_protocol_intertechno_v3_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                 subghz_protocol_intertechno_v3_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = IntertechnoV3DecoderStepEndDuration;

            } else if(
                //Add dimm_state
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_intertechno_v3_const.te_short) <
                 subghz_protocol_intertechno_v3_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                 subghz_protocol_intertechno_v3_const.te_delta) &&
                (instance->decoder.decode_count_bit == 27)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = IntertechnoV3DecoderStepEndDuration;

            } else
                instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;

    case IntertechnoV3DecoderStepEndDuration:
        if(!level && ((DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_short) <
                       subghz_protocol_intertechno_v3_const.te_delta) ||
                      (DURATION_DIFF(duration, subghz_protocol_intertechno_v3_const.te_long) <
                       subghz_protocol_intertechno_v3_const.te_delta * 2))) {
            instance->decoder.parser_step = IntertechnoV3DecoderStepStartDuration;
        } else {
            instance->decoder.parser_step = IntertechnoV3DecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_intertechno_v3_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
 *  A frame is either 32 or 36 bits:
 *     
 *               _
 *   start bit: | |__________ (T,10T)
 *          _   _
 *   '0':  | |_| |_____  (T,T,T,5T)
 *          _       _
 *   '1':  | |_____| |_  (T,5T,T,T)
 *          _   _
 *   dimm: | |_| |_     (T,T,T,T)
 * 
 *              _
 *   stop bit: | |____...____ (T,38T)
 * 
 *  if frame 32 bits
 *                     SSSSSSSSSSSSSSSSSSSSSSSSSS all_ch  on/off  ~ch
 *  Key:0x3F86C59F  => 00111111100001101100010110   0       1     1111
 * 
 *  if frame 36 bits
 *                     SSSSSSSSSSSSSSSSSSSSSSSSSS  all_ch dimm  ~ch   dimm_level
 *  Key:0x42D2E8856 => 01000010110100101110100010   0      X    0101  0110
 * 
 */

    if(instance->data_count_bit == subghz_protocol_intertechno_v3_const.min_count_bit_for_found) {
        instance->serial = (instance->data >> 6) & 0x3FFFFFF;
        if((instance->data >> 5) & 0x1) {
            instance->cnt = 1 << 5;
        } else {
            instance->cnt = (~instance->data & 0xF);
        }
        instance->btn = (instance->data >> 4) & 0x1;
    } else if(instance->data_count_bit == INTERTECHNO_V3_DIMMING_COUNT_BIT) {
        instance->serial = (instance->data >> 10) & 0x3FFFFFF;
        if((instance->data >> 9) & 0x1) {
            instance->cnt = 1 << 5;
        } else {
            instance->cnt = (~(instance->data >> 4) & 0xF);
        }
        instance->btn = (instance->data) & 0xF;
    } else {
        instance->serial = 0;
        instance->cnt = 0;
        instance->btn = 0;
    }
}

uint8_t subghz_protocol_decoder_intertechno_v3_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_intertechno_v3_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus subghz_protocol_decoder_intertechno_v3_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_intertechno_v3_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit != INTERTECHNO_V3_DIMMING_COUNT_BIT)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            ret = SubGhzProtocolStatusErrorValueBitCount;
            break;
        }
    } while(false);
    return ret;
}

void subghz_protocol_decoder_intertechno_v3_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderIntertechno_V3* instance = context;

    subghz_protocol_intertechno_v3_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%.11s %db\r\n"
        "Key:0x%08llX\r\n"
        "Sn:%07lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.data,
        instance->generic.serial);

    if(instance->generic.data_count_bit ==
       subghz_protocol_intertechno_v3_const.min_count_bit_for_found) {
        if(instance->generic.cnt >> 5) {
            furi_string_cat_printf(
                output, "Ch: All Btn:%s\r\n", (instance->generic.btn ? "On" : "Off"));
        } else {
            furi_string_cat_printf(
                output,
                "Ch:" CH_PATTERN " Btn:%s\r\n",
                CNT_TO_CH(instance->generic.cnt),
                (instance->generic.btn ? "On" : "Off"));
        }
    } else if(instance->generic.data_count_bit == INTERTECHNO_V3_DIMMING_COUNT_BIT) {
        furi_string_cat_printf(
            output,
            "Ch:" CH_PATTERN " Dimm:%d%%\r\n",
            CNT_TO_CH(instance->generic.cnt),
            (int)(6.67f * (float)instance->generic.btn));
    }
}
