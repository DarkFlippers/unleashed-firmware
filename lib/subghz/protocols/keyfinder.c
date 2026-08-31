#include "keyfinder.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolKeyFinder"

static const SubGhzBlockConst subghz_protocol_keyfinder_const = {
    .te_short = 400,
    .te_long = 1200,
    .te_delta = 150,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderKeyFinder {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint8_t end_count;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderKeyFinder);

struct SubGhzProtocolEncoderKeyFinder {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderKeyFinder);

typedef enum {
    KeyFinderDecoderStepReset = 0,
    KeyFinderDecoderStepSaveDuration,
    KeyFinderDecoderStepCheckDuration,
    KeyFinderDecoderStepFinish,
} KeyFinderDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_keyfinder_decoder = {
    .alloc = subghz_protocol_decoder_keyfinder_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_keyfinder_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize,
    .deserialize = subghz_protocol_decoder_keyfinder_deserialize,
    .get_string = subghz_protocol_decoder_keyfinder_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_keyfinder_encoder = {
    .alloc = subghz_protocol_encoder_keyfinder_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_keyfinder_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_keyfinder = {
    .name = SUBGHZ_PROTOCOL_KEYFINDER_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_keyfinder_decoder,
    .encoder = &subghz_protocol_keyfinder_encoder,
};

void* subghz_protocol_encoder_keyfinder_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderKeyFinder), &subghz_protocol_keyfinder, 5, 60);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderKeyFinder instance
 */
static void
    subghz_protocol_encoder_keyfinder_get_upload(SubGhzProtocolEncoderKeyFinder* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key data 24 bit first
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_keyfinder_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_keyfinder_const.te_long);

        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_keyfinder_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_keyfinder_const.te_short);
        }
    }
    // End bits (3 times then 1 more with gap 4k us)
    for(uint8_t i = 0; i < 3; i++) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_keyfinder_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_keyfinder_const.te_short);
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_keyfinder_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_keyfinder_const.te_short * 10);

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_keyfinder_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = instance->data >> 4;
    instance->btn = instance->data & 0xF;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_keyfinder_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderKeyFinder* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_keyfinder_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_keyfinder_check_remote_controller(&instance->generic);
        subghz_protocol_encoder_keyfinder_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void* subghz_protocol_decoder_keyfinder_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderKeyFinder), &subghz_protocol_keyfinder);
}

void subghz_protocol_decoder_keyfinder_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderKeyFinder* instance = context;

    // KeyFinder Decoder
    // 2026.03 - @xMasterX (MMX)

    // Key samples
    //
    // 433.92 MHz AM650
    //         Serial ID      Serial           ID
    // RED -    C396F E = 11000011100101101111 1110
    // PURPLE - C396F B = 11000011100101101111 1011
    // GREEN -  C396F D = 11000011100101101111 1101
    // BLUE -   C396F C = 11000011100101101111 1100

    switch(instance->decoder.parser_step) {
    case KeyFinderDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_keyfinder_const.te_short * 10) <
                        subghz_protocol_keyfinder_const.te_delta * 5)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = KeyFinderDecoderStepSaveDuration;
        }
        break;
    case KeyFinderDecoderStepSaveDuration:
        if(instance->decoder.decode_count_bit ==
           subghz_protocol_keyfinder_const.min_count_bit_for_found) {
            if((level) && (DURATION_DIFF(duration, subghz_protocol_keyfinder_const.te_short) <
                           subghz_protocol_keyfinder_const.te_delta)) {
                instance->end_count++;
                if(instance->end_count == 4) {
                    instance->decoder.parser_step = KeyFinderDecoderStepFinish;
                    instance->end_count = 0;
                }
            } else if(
                (!level) && (DURATION_DIFF(duration, subghz_protocol_keyfinder_const.te_short) <
                             subghz_protocol_keyfinder_const.te_delta)) {
                break;
            } else {
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->end_count = 0;
                instance->decoder.parser_step = KeyFinderDecoderStepReset;
            }
            break;
        }

        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = KeyFinderDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = KeyFinderDecoderStepReset;
        }
        break;
    case KeyFinderDecoderStepCheckDuration:
        if(!level) {
            // Bit 1 is short and long timing = 400us HIGH (te_last) and 1200us LOW
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_keyfinder_const.te_short) <
                subghz_protocol_keyfinder_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_keyfinder_const.te_long) <
                subghz_protocol_keyfinder_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = KeyFinderDecoderStepSaveDuration;
                // Bit 0 is long and short timing = 1200us HIGH (te_last) and 400us LOW
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_keyfinder_const.te_long) <
                 subghz_protocol_keyfinder_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_keyfinder_const.te_short) <
                 subghz_protocol_keyfinder_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = KeyFinderDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = KeyFinderDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = KeyFinderDecoderStepReset;
        }
        break;
    case KeyFinderDecoderStepFinish:
        // If got 24 bits key reading is finished
        if(instance->decoder.decode_count_bit ==
           subghz_protocol_keyfinder_const.min_count_bit_for_found) {
            instance->generic.data = instance->decoder.decode_data;
            instance->generic.data_count_bit = instance->decoder.decode_count_bit;
            if(instance->base.callback)
                instance->base.callback(&instance->base, instance->base.context);
        }
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        instance->end_count = 0;
        instance->decoder.parser_step = KeyFinderDecoderStepReset;
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_keyfinder_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderKeyFinder* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_keyfinder_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_keyfinder_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderKeyFinder* instance = context;

    subghz_protocol_keyfinder_check_remote_controller(&instance->generic);

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    // for future use
    // // push protocol data to global variable
    // subghz_block_generic_global.btn_is_available = false;
    // subghz_block_generic_global.current_btn = instance->generic.btn;
    // subghz_block_generic_global.btn_length_bit = 4;
    // //

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%06lX\r\n"
        "Yek: 0x%06lX\r\n"
        "Serial: 0x%05lX\r\n"
        "ID: 0x%0X",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        (uint32_t)(code_found_reverse & 0xFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
