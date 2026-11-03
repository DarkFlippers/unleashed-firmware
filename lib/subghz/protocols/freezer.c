#include "freezer.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolFreezer"

static const SubGhzBlockConst subghz_protocol_freezer_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 120,
    .min_count_bit_for_found = 24,
};

/*
 * 433.92MHz OOK. One short high that belongs to no cell, then 24 (low, high) cells, closed by
 * a carrier-off gap: short low with long high is a 1, long low with short high a 0.
 *
 * The symbols are classified by a split rather than by DURATION_DIFF windows because the
 * measured classes, 158..359 and 392..594, are only 33us apart while each spreads over 200us -
 * a symmetric delta wide enough for the spread would have to overlap.
 *
 * FREEZER_SYMBOL_MAX is what keeps this clear of Princeton: Princeton's long symbol is 1170us
 * nominal and 1509..1729 measured, so refusing everything over 700us means a Princeton frame
 * can never complete a Freezer one however the gaps line up. The reverse cannot happen either,
 * since no Freezer pulse reaches Princeton's long window at 870us.
 */
#define FREEZER_SYMBOL_MIN   120
#define FREEZER_SYMBOL_SPLIT 375 // short at or under, long above
#define FREEZER_SYMBOL_MAX   700

/* The first frame of a press follows a much longer idle than the 8.9ms between frames, so the
 * end of a frame is a minimum rather than a window. */
#define FREEZER_GAP_MIN_TE   30
#define FREEZER_GAP_MIN      ((uint32_t)subghz_protocol_freezer_const.te_short * FREEZER_GAP_MIN_TE)
#define FREEZER_GAP_DURATION ((uint32_t)subghz_protocol_freezer_const.te_short * 36)

struct SubGhzProtocolDecoderFreezer {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint64_t last_data;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderFreezer);

struct SubGhzProtocolEncoderFreezer {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderFreezer);

typedef enum {
    FreezerDecoderStepReset = 0,
    FreezerDecoderStepStart,
    FreezerDecoderStepSaveDuration,
    FreezerDecoderStepCheckDuration,
} FreezerDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_freezer_decoder = {
    .alloc = subghz_protocol_decoder_freezer_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_freezer_feed,
    .reset = subghz_protocol_decoder_freezer_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize,
    .deserialize = subghz_protocol_decoder_freezer_deserialize,
    .get_string = subghz_protocol_decoder_freezer_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_freezer_encoder = {
    .alloc = subghz_protocol_encoder_freezer_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_freezer_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_freezer = {
    .name = SUBGHZ_PROTOCOL_FREEZER_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_freezer_decoder,
    .encoder = &subghz_protocol_freezer_encoder,
};

void* subghz_protocol_encoder_freezer_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderFreezer), &subghz_protocol_freezer, 3, 64);
}

static void subghz_protocol_freezer_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = (instance->data >> 4) & 0xFFFFF;
    instance->btn = instance->data & 0xF;
}

/**
 * Generating an upload from data.
 * @param context Pointer to a SubGhzProtocolEncoderFreezer instance
 * @return true Always; this encoder has no failure path
 */
static bool subghz_protocol_encoder_freezer_get_upload(void* context) {
    SubGhzProtocolEncoderFreezer* instance = context;
    furi_assert(instance);

    subghz_protocol_freezer_check_remote_controller(&instance->generic);
    size_t index = 0;

    // The lone short high the decoder steps over before the first cell
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_freezer_const.te_short);

    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Bit 1: short low then long high
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_freezer_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_freezer_const.te_long);
        } else {
            // Bit 0: long low then short high
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_freezer_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_freezer_const.te_short);
        }
    }

    instance->encoder.upload[index++] = level_duration_make(false, FREEZER_GAP_DURATION);

    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_freezer_deserialize(void* context, FlipperFormat* flipper_format) {
    return subghz_protocol_encoder_common_deserialize(
        context,
        flipper_format,
        subghz_protocol_freezer_const.min_count_bit_for_found,
        subghz_protocol_encoder_freezer_get_upload);
}

void* subghz_protocol_decoder_freezer_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderFreezer* instance = subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderFreezer), &subghz_protocol_freezer);
    instance->last_data = 0; // common_alloc does not zero

    return instance;
}

void subghz_protocol_decoder_freezer_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderFreezer* instance = context;
    instance->decoder.parser_step = FreezerDecoderStepReset;
    instance->last_data = 0;
}

static bool subghz_protocol_freezer_is_short(uint32_t duration) {
    return (duration >= FREEZER_SYMBOL_MIN) && (duration <= FREEZER_SYMBOL_SPLIT);
}

static bool subghz_protocol_freezer_is_long(uint32_t duration) {
    return (duration > FREEZER_SYMBOL_SPLIT) && (duration <= FREEZER_SYMBOL_MAX);
}

void subghz_protocol_decoder_freezer_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFreezer* instance = context;

    switch(instance->decoder.parser_step) {
    case FreezerDecoderStepReset:
        if((!level) && (duration >= FREEZER_GAP_MIN)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = FreezerDecoderStepStart;
        }
        break;

    case FreezerDecoderStepStart:
        if(level && subghz_protocol_freezer_is_short(duration)) {
            instance->decoder.parser_step = FreezerDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = FreezerDecoderStepReset;
        }
        break;

    case FreezerDecoderStepSaveDuration: // a cell opens on the low
        if(level) {
            instance->decoder.parser_step = FreezerDecoderStepReset;
            break;
        }
        if(duration >= FREEZER_GAP_MIN) {
            /* Only a code that arrived twice running is reported: a remote repeats its frame
             * for as long as the button is held, a stray run of plausible cells does not. */
            if((instance->decoder.decode_count_bit ==
                subghz_protocol_freezer_const.min_count_bit_for_found) &&
               (instance->decoder.decode_data == instance->last_data) && instance->last_data) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            if(instance->decoder.decode_count_bit ==
               subghz_protocol_freezer_const.min_count_bit_for_found) {
                instance->last_data = instance->decoder.decode_data;
            } else {
                instance->last_data = 0;
            }
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = FreezerDecoderStepStart;
            break;
        }
        instance->decoder.te_last = duration;
        instance->decoder.parser_step = FreezerDecoderStepCheckDuration;
        break;

    case FreezerDecoderStepCheckDuration: // and closes on the high
        if(!level) {
            instance->decoder.parser_step = FreezerDecoderStepReset;
            break;
        }
        if(subghz_protocol_freezer_is_short(instance->decoder.te_last) &&
           subghz_protocol_freezer_is_long(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            instance->decoder.parser_step = FreezerDecoderStepSaveDuration;
        } else if(
            subghz_protocol_freezer_is_long(instance->decoder.te_last) &&
            subghz_protocol_freezer_is_short(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            instance->decoder.parser_step = FreezerDecoderStepSaveDuration;
        } else {
            instance->last_data = 0;
            instance->decoder.parser_step = FreezerDecoderStepReset;
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_freezer_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderFreezer* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_freezer_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_freezer_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderFreezer* instance = context;

    subghz_protocol_freezer_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%06lX\r\n"
        "Sn:0x%05lX Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
