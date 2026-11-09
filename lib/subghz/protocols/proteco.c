#include "proteco.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolProteco"

/*
 * Proteco MOVER gate remotes (Italy), 433.92 MHz AM650, 24 bit fixed code.
 * https://github.com/flipperdevices/flipperzero-firmware/issues/2345
 *
 * Same line coding as CAME, but at Te ~440us instead of 320us, so the CAME decoder
 * accepts the guard and start bit and then rejects every long pulse:
 *   guard  : LOW  20.5*Te (~9.0ms)
 *   start  : HIGH Te
 *   bit 1  : LOW 2*Te, HIGH Te
 *   bit 0  : LOW Te,   HIGH 2*Te
 * MSB first, frame repeated while the button is held, low 2 bits are the button.
 */

/* The guard is measured against the frame's own Te rather than a fixed us window: it is
 * 20.4-20.6 Te on the fob and comes from the same oscillator, so this both admits a remote
 * off clock and keeps out a foreign 24 bit frame that merely happens to sit near 9 ms.
 * In half Te, because the fob's gap is not a whole number of them. */
#define PROTECO_GUARD_TE_X2     41u
#define PROTECO_GUARD_TE_X2_MIN 37u
#define PROTECO_GUARD_TE_X2_MAX 45u

/* Low 2 bits of the key, reported as buttons 1..4 so that 0 keeps its "no button seen yet"
 * meaning in subghz_custom_btn. A, B and C are a TX3; the fourth code is unverified. */
#define PROTECO_BTN_MASK 0x3u
static const uint8_t subghz_protocol_proteco_btn[4] = {0x2, 0x3, 0x1, 0x4};
static const uint8_t subghz_protocol_proteco_btn_raw[5] = {0x0, 0x2, 0x0, 0x1, 0x3};

static const SubGhzBlockConst subghz_protocol_proteco_const = {
    .te_short = 440,
    .te_long = 880,
    .te_delta = 150,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderProteco {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te; // sum of the frame's pulses, then the mean Te
    uint32_t guard; // the LOW that opened the frame
};
SUBGHZ_ASSERT_DECODER_TE_LAYOUT(SubGhzProtocolDecoderProteco);

struct SubGhzProtocolEncoderProteco {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderProteco);

typedef enum {
    ProtecoDecoderStepReset = 0,
    ProtecoDecoderStepFoundStartBit,
    ProtecoDecoderStepSaveDuration,
    ProtecoDecoderStepCheckDuration,
} ProtecoDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_proteco_decoder = {
    .alloc = subghz_protocol_decoder_proteco_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_proteco_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize,
    .deserialize = subghz_protocol_decoder_proteco_deserialize,
    .get_string = subghz_protocol_decoder_proteco_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_proteco_encoder = {
    .alloc = subghz_protocol_encoder_proteco_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_proteco_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_proteco = {
    .name = SUBGHZ_PROTOCOL_PROTECO_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_proteco_decoder,
    .encoder = &subghz_protocol_proteco_encoder,
};

void* subghz_protocol_encoder_proteco_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    // guard + start bit + 24 bits * 2 = 50 entries
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderProteco), &subghz_protocol_proteco, 10, 50);
}

/**
 * Defines the button value for the current btn_id.
 * @return Button code 1..4, or 0 while no original button is known
 */
static uint8_t subghz_protocol_proteco_get_btn_code(void) {
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t custom_btn_id = subghz_custom_btn_get();

    if((original_btn_code == 0) || (custom_btn_id == SUBGHZ_CUSTOM_BTN_OK)) {
        return original_btn_code;
    }
    // one of the three other fob buttons per arrow key
    return (uint8_t)(((original_btn_code - 1u + custom_btn_id) & PROTECO_BTN_MASK) + 1u);
}

/**
 * Generating an upload from data.
 * @param context Pointer to a SubGhzProtocolEncoderProteco instance
 * @return true Always; the bit count is checked on deserialize
 */
static bool subghz_protocol_encoder_proteco_get_upload(void* context) {
    SubGhzProtocolEncoderProteco* instance = context;
    const SubGhzBlockConst* c = &subghz_protocol_proteco_const;
    size_t index = 0;

    uint8_t btn = subghz_protocol_proteco_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&btn)) {
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", btn);
    }
    if((btn >= 1) && (btn <= 4)) {
        instance->generic.data = (instance->generic.data & ~(uint64_t)PROTECO_BTN_MASK) |
                                 subghz_protocol_proteco_btn_raw[btn];
    }

    instance->encoder.upload[index++] =
        level_duration_make(false, c->te_short * PROTECO_GUARD_TE_X2 / 2);
    instance->encoder.upload[index++] = level_duration_make(true, c->te_short);
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            instance->encoder.upload[index++] = level_duration_make(false, c->te_long);
            instance->encoder.upload[index++] = level_duration_make(true, c->te_short);
        } else {
            instance->encoder.upload[index++] = level_duration_make(false, c->te_short);
            instance->encoder.upload[index++] = level_duration_make(true, c->te_long);
        }
    }
    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_proteco_deserialize(void* context, FlipperFormat* flipper_format) {
    return subghz_protocol_encoder_common_deserialize(
        context,
        flipper_format,
        subghz_protocol_proteco_const.min_count_bit_for_found,
        subghz_protocol_encoder_proteco_get_upload);
}

void* subghz_protocol_decoder_proteco_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderProteco), &subghz_protocol_proteco);
}

/** Is the LOW that opened this frame 20.5 Te of the frame's own clock?
 * @param instance Pointer to a SubGhzProtocolDecoderProteco instance
 * @return true if the gap belongs to this frame's oscillator
 */
static bool subghz_protocol_proteco_guard_ok(SubGhzProtocolDecoderProteco* instance) {
    // start bit plus 3 Te per bit
    uint32_t te = instance->te / (instance->decoder.decode_count_bit * 3 + 1);
    return (instance->guard >= te * PROTECO_GUARD_TE_X2_MIN / 2) &&
           (instance->guard <= te * PROTECO_GUARD_TE_X2_MAX / 2);
}

void subghz_protocol_decoder_proteco_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderProteco* instance = context;
    const SubGhzBlockConst* c = &subghz_protocol_proteco_const;

    switch(instance->decoder.parser_step) {
    case ProtecoDecoderStepReset:
        if((!level) && (duration >= c->te_short * 4)) {
            instance->guard = duration;
            instance->decoder.parser_step = ProtecoDecoderStepFoundStartBit;
        }
        break;
    case ProtecoDecoderStepFoundStartBit:
        if(level && (DURATION_DIFF(duration, c->te_short) < c->te_delta)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = duration;
            instance->decoder.parser_step = ProtecoDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = ProtecoDecoderStepReset;
        }
        break;
    case ProtecoDecoderStepSaveDuration:
        if(level) {
            instance->decoder.parser_step = ProtecoDecoderStepReset;
            break;
        }
        if(duration >= c->te_short * 4) {
            // this gap ends the frame and is the next frame's guard
            if((instance->decoder.decode_count_bit == c->min_count_bit_for_found) &&
               subghz_protocol_proteco_guard_ok(instance)) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->guard = duration;
            instance->decoder.parser_step = ProtecoDecoderStepFoundStartBit;
            break;
        }
        instance->decoder.te_last = duration;
        instance->te += duration;
        instance->decoder.parser_step = ProtecoDecoderStepCheckDuration;
        break;
    case ProtecoDecoderStepCheckDuration:
        if(level && (DURATION_DIFF(instance->decoder.te_last, c->te_short) < c->te_delta) &&
           (DURATION_DIFF(duration, c->te_long) < c->te_delta)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            instance->te += duration;
            instance->decoder.parser_step = ProtecoDecoderStepSaveDuration;
        } else if(
            level && (DURATION_DIFF(instance->decoder.te_last, c->te_long) < c->te_delta) &&
            (DURATION_DIFF(duration, c->te_short) < c->te_delta)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            instance->te += duration;
            instance->decoder.parser_step = ProtecoDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = ProtecoDecoderStepReset;
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_proteco_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderProteco* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_proteco_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_proteco_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderProteco* instance = context;

    instance->generic.serial = (uint32_t)(instance->generic.data >> 2) & 0x3FFFFF;
    instance->generic.btn = subghz_protocol_proteco_btn[instance->generic.data & PROTECO_BTN_MASK];

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->generic.btn);
    }
    subghz_custom_btn_set_max(3);

    // push protocol data to global variable
    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 3;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%06lX\r\n"
        "Sn:0x%06lX  Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
