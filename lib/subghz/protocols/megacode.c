#include "megacode.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://wiki.cuvoodoo.info/doku.php?id=megacode
 * https://wiki.cuvoodoo.info/lib/exe/fetch.php?media=megacode:megacode_1.pdf
 * https://fccid.io/EF4ACP00872/Test-Report/Megacode-2-112615.pdf
 * https://github.com/aaronsp777/megadecoder
 * https://github.com/rjmendez/Linear_keyfob
 * https://github.com/j07rdi/Linear_MegaCode_Garage_Remote
 *
 */

#define TAG "SubGhzProtocolMegaCode"

static const SubGhzBlockConst subghz_protocol_megacode_const = {
    .te_short = 1000,
    .te_long = 1000,
    .te_delta = 200,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderMegaCode {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint8_t last_bit;
};

struct SubGhzProtocolEncoderMegaCode {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    MegaCodeDecoderStepReset = 0,
    MegaCodeDecoderStepFoundStartBit,
    MegaCodeDecoderStepSaveDuration,
    MegaCodeDecoderStepCheckDuration,
} MegaCodeDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_megacode_decoder = {
    .alloc = subghz_protocol_decoder_megacode_alloc,
    .free = subghz_protocol_decoder_megacode_free,

    .feed = subghz_protocol_decoder_megacode_feed,
    .reset = subghz_protocol_decoder_megacode_reset,

    .get_hash_data = subghz_protocol_decoder_megacode_get_hash_data,
    .serialize = subghz_protocol_decoder_megacode_serialize,
    .deserialize = subghz_protocol_decoder_megacode_deserialize,
    .get_string = subghz_protocol_decoder_megacode_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_megacode_encoder = {
    .alloc = subghz_protocol_encoder_megacode_alloc,
    .free = subghz_protocol_encoder_megacode_free,

    .deserialize = subghz_protocol_encoder_megacode_deserialize,
    .stop = subghz_protocol_encoder_megacode_stop,
    .yield = subghz_protocol_encoder_megacode_yield,
};

const SubGhzProtocol subghz_protocol_megacode = {
    .name = SUBGHZ_PROTOCOL_MEGACODE_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_megacode_decoder,
    .encoder = &subghz_protocol_megacode_encoder,
};

void* subghz_protocol_encoder_megacode_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderMegaCode* instance = malloc(sizeof(SubGhzProtocolEncoderMegaCode));

    instance->base.protocol = &subghz_protocol_megacode;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_megacode_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderMegaCode* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderMegaCode instance
 * @return true On success
 */
static bool subghz_protocol_encoder_megacode_get_upload(SubGhzProtocolEncoderMegaCode* instance) {
    furi_assert(instance);
    uint8_t last_bit = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    /*
    * Due to the nature of the protocol
    *
    *  00000 1
    *  _____|-| = 1 becomes
    * 
    *  00 1 000
    *  __|-|___ = 0 becomes
    * 
    * it's easier for us to generate an upload backwards
    */

    size_t index = size_upload - 1;

    // Send end level
    instance->encoder.upload[index--] =
        level_duration_make(true, (uint32_t)subghz_protocol_megacode_const.te_short);
    if(bit_read(instance->generic.data, 0)) {
        last_bit = 1;
    } else {
        last_bit = 0;
    }

    //Send key data
    for(uint8_t i = 1; i < instance->generic.data_count_bit; i++) {
        if(bit_read(instance->generic.data, i)) {
            //if bit 1
            instance->encoder.upload[index--] = level_duration_make(
                false,
                last_bit ? (uint32_t)subghz_protocol_megacode_const.te_short * 5 :
                           (uint32_t)subghz_protocol_megacode_const.te_short * 2);
            last_bit = 1;
        } else {
            //if bit 0
            instance->encoder.upload[index--] = level_duration_make(
                false,
                last_bit ? (uint32_t)subghz_protocol_megacode_const.te_short * 8 :
                           (uint32_t)subghz_protocol_megacode_const.te_short * 5);
            last_bit = 0;
        }
        instance->encoder.upload[index--] =
            level_duration_make(true, (uint32_t)subghz_protocol_megacode_const.te_short);
    }

    //Send PT_GUARD
    if(bit_read(instance->generic.data, 0)) {
        //if end bit 1
        instance->encoder.upload[index] =
            level_duration_make(false, (uint32_t)subghz_protocol_megacode_const.te_short * 11);
    } else {
        //if end bit 1
        instance->encoder.upload[index] =
            level_duration_make(false, (uint32_t)subghz_protocol_megacode_const.te_short * 14);
    }

    return true;
}

bool subghz_protocol_encoder_megacode_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderMegaCode* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_megacode_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_megacode_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_megacode_stop(void* context) {
    SubGhzProtocolEncoderMegaCode* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_megacode_yield(void* context) {
    SubGhzProtocolEncoderMegaCode* instance = context;

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

void* subghz_protocol_decoder_megacode_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderMegaCode* instance = malloc(sizeof(SubGhzProtocolDecoderMegaCode));
    instance->base.protocol = &subghz_protocol_megacode;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_megacode_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    free(instance);
}

void subghz_protocol_decoder_megacode_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    instance->decoder.parser_step = MegaCodeDecoderStepReset;
}

void subghz_protocol_decoder_megacode_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    switch(instance->decoder.parser_step) {
    case MegaCodeDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_megacode_const.te_short * 13) <
                        subghz_protocol_megacode_const.te_delta * 17)) { //10..16ms
            //Found header MegaCode
            instance->decoder.parser_step = MegaCodeDecoderStepFoundStartBit;
        }
        break;
    case MegaCodeDecoderStepFoundStartBit:
        if(level && (DURATION_DIFF(duration, subghz_protocol_megacode_const.te_short) <
                     subghz_protocol_megacode_const.te_delta)) {
            //Found start bit MegaCode
            instance->decoder.parser_step = MegaCodeDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            instance->last_bit = 1;

        } else {
            instance->decoder.parser_step = MegaCodeDecoderStepReset;
        }
        break;
    case MegaCodeDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_megacode_const.te_short * 10)) {
                instance->decoder.parser_step = MegaCodeDecoderStepReset;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_megacode_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }

            if(!instance->last_bit) {
                instance->decoder.te_last = duration - subghz_protocol_megacode_const.te_short * 3;
            } else {
                instance->decoder.te_last = duration;
            }
            instance->decoder.parser_step = MegaCodeDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = MegaCodeDecoderStepReset;
        }
        break;
    case MegaCodeDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_megacode_const.te_short * 5) <
                subghz_protocol_megacode_const.te_delta * 5) &&
               (DURATION_DIFF(duration, subghz_protocol_megacode_const.te_short) <
                subghz_protocol_megacode_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->last_bit = 1;
                instance->decoder.parser_step = MegaCodeDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_megacode_const.te_short * 2) <
                 subghz_protocol_megacode_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_megacode_const.te_short) <
                 subghz_protocol_megacode_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->last_bit = 0;
                instance->decoder.parser_step = MegaCodeDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = MegaCodeDecoderStepReset;
        } else {
            instance->decoder.parser_step = MegaCodeDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_megacode_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
    * Short: 1000 µs
    * Long: 1000 µs
    * Gap: 11000 .. 14000 µs
    * A Linear Megacode transmission consists of 24 bit frames starting with 
    * the most significant bit and ending with the least. Each of the 24 bit 
    * frames is 6 milliseconds wide and always contains a single 1 millisecond 
    * pulse. A frame with more than 1 pulse or a frame with no pulse is invalid 
    * and a receiver should reset and begin watching for another start bit. 
    * Start bit is always 1.
    * 
    * 
    * Example (I created with my own remote):
    * Remote “A” has the code “17316”, a Facility Code of “3”, and a single button.
    * Start bit (S) = 1
    * Facility Code 3 (F) = 0011
    * Remote Code (Key) 17316 = 43A4 = 0100001110100100
    * Button (Btn) 1 = 001
    *          S  F        Key         Btn
    * Result = 1|0011|0100001110100100|001
    * 
    *  00000 1
    *  _____|-| = 1 becomes
    * 
    *  00 1 000
    *  __|-|___ = 0 becomes
    * 
    * The device needs to transmit with a 9000 µs gap between retransmissions:
    * 000001 001000 001000 000001 000001 001000 000001 001000 001000 001000 001000 000001
    * 000001 000001 001000 000001 001000 001000 000001 001000 001000 001000 001000 000001
    * wait 9000 µs
    * 000001 001000 001000 000001 000001 001000 000001 001000 001000 001000 001000 000001
    * 000001 000001 001000 000001 001000 001000 000001 001000 001000 001000 001000 000001
    * 
    */
    if((instance->data >> 23) == 1) {
        instance->serial = (instance->data >> 3) & 0xFFFF;
        instance->btn = instance->data & 0b111;
        instance->cnt = (instance->data >> 19) & 0b1111;
    } else {
        instance->serial = 0;
        instance->btn = 0;
        instance->cnt = 0;
    }
}

uint8_t subghz_protocol_decoder_megacode_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_megacode_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_megacode_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_megacode_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_megacode_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderMegaCode* instance = context;
    subghz_protocol_megacode_check_remote_controller(&instance->generic);

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%06lX\r\n"
        "Sn:0x%04lX - %d\r\n"
        "Facility:%X Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)instance->generic.data,
        instance->generic.serial,
        instance->generic.serial,
        instance->generic.cnt,
        instance->generic.btn);
}
