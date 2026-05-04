/**
 * allstar_firefly.c  --  Allstar Firefly 318ALD31K native SubGHz protocol
 *
 * Implements the SubGhzProtocol vtable for the Supertex ED-9 based gate remote.
 * Uses subghz_block_generic_serialize / deserialize for standard-format .sub
 * files, encoding the 9-position trinary DIP code as a uint64 (base-3, MSB
 * first: '+' = 2, '0' = 3, '-' = 0).
 *
 * Saved file format:
 *   Filetype: Flipper SubGhz Key File
 *   Version: 1
 *   Frequency: 318000000
 *   Preset: FuriHalSubGhzPresetOok650Async
 *   Protocol: Allstar Firefly
 *   Key: 00 00 00 00 00 00 34 B9
 *   Bit: 18
 *
 * See allstar_firefly.h for full protocol documentation.
 */

#include "allstar_firefly.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "AllstarFirefly"

#define DIP_P 0b11 //(+)
#define DIP_O 0b10 //(0)
#define DIP_N 0b00 //(-)

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c"

#define SHOW_DIP_P(dip, check_dip)                         \
    ((((dip >> 0x10) & 0x3) == check_dip) ? '*' : '_'),    \
        ((((dip >> 0xE) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0xC) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0xA) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x8) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x6) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x4) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x2) & 0x3) == check_dip) ? '*' : '_'), \
        ((((dip >> 0x0) & 0x3) == check_dip) ? '*' : '_')

static const SubGhzBlockConst subghz_protocol_allstar_firefly_const = {
    .te_short = 600,
    .te_long = 4000,
    .te_delta = 300,
    .min_count_bit_for_found = 18,
};

struct SubGhzProtocolDecoderAllstarFirefly {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderAllstarFirefly {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    AllstarFireflyDecoderStepReset = 0,
    AllstarFireflyDecoderStepSaveDuration,
    AllstarFireflyDecoderStepCheckDuration,
} AllstarFireflyDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_allstar_firefly_decoder = {
    .alloc = subghz_protocol_decoder_allstar_firefly_alloc,
    .free = subghz_protocol_decoder_allstar_firefly_free,

    .feed = subghz_protocol_decoder_allstar_firefly_feed,
    .reset = subghz_protocol_decoder_allstar_firefly_reset,

    .get_hash_data = subghz_protocol_decoder_allstar_firefly_get_hash_data,
    .serialize = subghz_protocol_decoder_allstar_firefly_serialize,
    .deserialize = subghz_protocol_decoder_allstar_firefly_deserialize,
    .get_string = subghz_protocol_decoder_allstar_firefly_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_allstar_firefly_encoder = {
    .alloc = subghz_protocol_encoder_allstar_firefly_alloc,
    .free = subghz_protocol_encoder_allstar_firefly_free,

    .deserialize = subghz_protocol_encoder_allstar_firefly_deserialize,
    .stop = subghz_protocol_encoder_allstar_firefly_stop,
    .yield = subghz_protocol_encoder_allstar_firefly_yield,
};

const SubGhzProtocol subghz_protocol_allstar_firefly = {
    .name = SUBGHZ_PROTOCOL_ALLSTAR_FIREFLY_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_allstar_firefly_decoder,
    .encoder = &subghz_protocol_allstar_firefly_encoder,
};

void* subghz_protocol_encoder_allstar_firefly_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderAllstarFirefly* instance =
        malloc(sizeof(SubGhzProtocolEncoderAllstarFirefly));

    instance->base.protocol = &subghz_protocol_allstar_firefly;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 5;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_allstar_firefly_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderAllstarFirefly* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 */
static void subghz_protocol_encoder_allstar_firefly_get_upload(
    SubGhzProtocolEncoderAllstarFirefly* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_allstar_firefly_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)(subghz_protocol_allstar_firefly_const.te_short * 50) + 400);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_allstar_firefly_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_allstar_firefly_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)(subghz_protocol_allstar_firefly_const.te_short * 50) + 400);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_allstar_firefly_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

SubGhzProtocolStatus subghz_protocol_encoder_allstar_firefly_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderAllstarFirefly* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_allstar_firefly_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_allstar_firefly_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_allstar_firefly_stop(void* context) {
    SubGhzProtocolEncoderAllstarFirefly* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_allstar_firefly_yield(void* context) {
    SubGhzProtocolEncoderAllstarFirefly* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        if(!subghz_block_generic_global.endless_tx) instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_allstar_firefly_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderAllstarFirefly* instance =
        malloc(sizeof(SubGhzProtocolDecoderAllstarFirefly));
    instance->base.protocol = &subghz_protocol_allstar_firefly;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_allstar_firefly_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;
    free(instance);
}

void subghz_protocol_decoder_allstar_firefly_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;
    instance->decoder.parser_step = AllstarFireflyDecoderStepReset;
}

void subghz_protocol_decoder_allstar_firefly_feed(
    void* context,
    bool level,
    volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;

    // Allstar Firefly Decoder
    // 2026 - by @jlaughter
    // Remake to match other protocols code base - @xMasterX (MMX)

    switch(instance->decoder.parser_step) {
    case AllstarFireflyDecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, subghz_protocol_allstar_firefly_const.te_short * 50) <
            subghz_protocol_allstar_firefly_const.te_delta * 5)) {
            // 30k us
            // Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = AllstarFireflyDecoderStepSaveDuration;
        }
        break;
    case AllstarFireflyDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = AllstarFireflyDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = AllstarFireflyDecoderStepReset;
        }
        break;
    case AllstarFireflyDecoderStepCheckDuration:
        if(!level) {
            // Bit 1 is long and short timing = 4000us HIGH (te_last) and 600us LOW
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_allstar_firefly_const.te_long) <
                subghz_protocol_allstar_firefly_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_allstar_firefly_const.te_short) <
                subghz_protocol_allstar_firefly_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = AllstarFireflyDecoderStepSaveDuration;
                // Bit 0 is short and long timing = 600us HIGH (te_last) and 4000us LOW
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_allstar_firefly_const.te_short) <
                 subghz_protocol_allstar_firefly_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_allstar_firefly_const.te_long) <
                 subghz_protocol_allstar_firefly_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = AllstarFireflyDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_allstar_firefly_const.te_short * 50) <
                subghz_protocol_allstar_firefly_const.te_delta * 5) {
                // Found next GAP and add bit 0 or 1
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_allstar_firefly_const.te_long) <
                    subghz_protocol_allstar_firefly_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                if((DURATION_DIFF(
                        instance->decoder.te_last,
                        subghz_protocol_allstar_firefly_const.te_short) <
                    subghz_protocol_allstar_firefly_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                // If got 18 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_allstar_firefly_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = AllstarFireflyDecoderStepReset;
            } else {
                instance->decoder.parser_step = AllstarFireflyDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = AllstarFireflyDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_allstar_firefly_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_allstar_firefly_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus subghz_protocol_decoder_allstar_firefly_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_allstar_firefly_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_allstar_firefly_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderAllstarFirefly* instance = context;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%05lX Yek:0x%05lX\r\n"
        "  +:   " DIP_PATTERN "\r\n"
        "  o:   " DIP_PATTERN "\r\n"
        "  -:   " DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFF),
        (uint32_t)(code_found_reverse & 0xFFFFF),
        SHOW_DIP_P(instance->generic.data, DIP_P),
        SHOW_DIP_P(instance->generic.data, DIP_O),
        SHOW_DIP_P(instance->generic.data, DIP_N));
}
