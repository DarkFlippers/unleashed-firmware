#include "proteco.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolProteco"

/*
 * Proteco MOVER gate remotes (Italy), 433.92 MHz AM650, 24 bit fixed code.
 * https://github.com/flipperdevices/flipperzero-firmware/issues/2345
 *
 * Same line coding as CAME, but at Te ~440us instead of 320us, so the CAME decoder
 * accepts the guard and start bit and then rejects every long pulse:
 *   guard  : LOW  ~21*Te (~9.1ms)
 *   start  : HIGH Te
 *   bit 1  : LOW 2*Te, HIGH Te
 *   bit 0  : LOW Te,   HIGH 2*Te
 * MSB first, frame repeated while the button is held.
 */
#define PROTECO_GUARD_TE 21u

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
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderProteco);

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
 * Generating an upload from data.
 * @param context Pointer to a SubGhzProtocolEncoderProteco instance
 * @return true Always; the bit count is checked on deserialize
 */
static bool subghz_protocol_encoder_proteco_get_upload(void* context) {
    SubGhzProtocolEncoderProteco* instance = context;
    const SubGhzBlockConst* c = &subghz_protocol_proteco_const;
    size_t index = 0;

    instance->encoder.upload[index++] = level_duration_make(false, c->te_short * PROTECO_GUARD_TE);
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

void subghz_protocol_decoder_proteco_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderProteco* instance = context;
    const SubGhzBlockConst* c = &subghz_protocol_proteco_const;

    switch(instance->decoder.parser_step) {
    case ProtecoDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, c->te_short * PROTECO_GUARD_TE) <
                        c->te_delta * PROTECO_GUARD_TE / 2)) {
            instance->decoder.parser_step = ProtecoDecoderStepFoundStartBit;
        }
        break;
    case ProtecoDecoderStepFoundStartBit:
        if(level && (DURATION_DIFF(duration, c->te_short) < c->te_delta)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
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
            // guard reached, the frame ends here and the next one may follow
            if(instance->decoder.decode_count_bit == c->min_count_bit_for_found) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->decoder.parser_step =
                (DURATION_DIFF(duration, c->te_short * PROTECO_GUARD_TE) <
                 c->te_delta * PROTECO_GUARD_TE / 2) ?
                    ProtecoDecoderStepFoundStartBit :
                    ProtecoDecoderStepReset;
            break;
        }
        instance->decoder.te_last = duration;
        instance->decoder.parser_step = ProtecoDecoderStepCheckDuration;
        break;
    case ProtecoDecoderStepCheckDuration:
        if(level && (DURATION_DIFF(instance->decoder.te_last, c->te_short) < c->te_delta) &&
           (DURATION_DIFF(duration, c->te_long) < c->te_delta)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            instance->decoder.parser_step = ProtecoDecoderStepSaveDuration;
        } else if(
            level && (DURATION_DIFF(instance->decoder.te_last, c->te_long) < c->te_delta) &&
            (DURATION_DIFF(duration, c->te_short) < c->te_delta)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
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

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%06lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF));
}
