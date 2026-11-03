#include "kce6xxx.h"

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

static const SubGhzBlockConst ra_protocol_kce6xxx_const = {
    .te_short = 175,
    .te_long = 500,
    .te_delta = 120,
    .min_count_bit_for_found = 32,
};

/* Measured 3250-3500us. The window stays clear of te_long + te_delta so a data low can
 * never be taken for a header. */
#define KCE6XXX_HEADER_US    3350
#define KCE6XXX_HEADER_DELTA 500

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
} RaProtocolDecoderKce6xxx;

typedef enum {
    Kce6xxxDecoderStepReset = 0,
    Kce6xxxDecoderStepSaveDuration,
    Kce6xxxDecoderStepCheckDuration,
} Kce6xxxDecoderStep;

static void* ra_protocol_decoder_kce6xxx_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderKce6xxx* instance = malloc(sizeof(RaProtocolDecoderKce6xxx));
    instance->base.protocol = &ra_protocol_kce6xxx;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_kce6xxx_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_kce6xxx_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderKce6xxx* instance = context;
    instance->decoder.parser_step = Kce6xxxDecoderStepReset;
}

static bool ra_protocol_kce6xxx_is_short(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_kce6xxx_const.te_short) <
           ra_protocol_kce6xxx_const.te_delta;
}

static bool ra_protocol_kce6xxx_is_long(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_kce6xxx_const.te_long) <
           ra_protocol_kce6xxx_const.te_delta;
}

static bool ra_protocol_kce6xxx_is_header(uint32_t duration) {
    return DURATION_DIFF(duration, KCE6XXX_HEADER_US) < KCE6XXX_HEADER_DELTA;
}

static void ra_protocol_decoder_kce6xxx_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderKce6xxx* instance = context;

    switch(instance->decoder.parser_step) {
    case Kce6xxxDecoderStepReset:
        if((!level) && ra_protocol_kce6xxx_is_header(duration)) {
            instance->decoder.parser_step = Kce6xxxDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;

    case Kce6xxxDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Kce6xxxDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Kce6xxxDecoderStepReset;
        }
        break;

    case Kce6xxxDecoderStepCheckDuration:
        if(level) {
            instance->decoder.parser_step = Kce6xxxDecoderStepReset;
            break;
        }

        /* The low closing the frame is the next header. The trailing high is already taken
         * as te_last, so a full frame is exactly 32 bits by now. */
        if(ra_protocol_kce6xxx_is_header(duration)) {
            if((instance->decoder.decode_count_bit ==
                ra_protocol_kce6xxx_const.min_count_bit_for_found) &&
               ra_protocol_kce6xxx_is_short(instance->decoder.te_last)) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = Kce6xxxDecoderStepSaveDuration;
            break;
        }

        if(ra_protocol_kce6xxx_is_short(instance->decoder.te_last) &&
           ra_protocol_kce6xxx_is_long(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            instance->decoder.parser_step = Kce6xxxDecoderStepSaveDuration;
        } else if(
            ra_protocol_kce6xxx_is_long(instance->decoder.te_last) &&
            ra_protocol_kce6xxx_is_short(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            instance->decoder.parser_step = Kce6xxxDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = Kce6xxxDecoderStepReset;
        }
        break;
    }
}

static void ra_protocol_decoder_kce6xxx_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderKce6xxx* instance = context;

    // No serial, button or counter field was identified in the 32 bits, so nothing is split out
    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%08lX\r\n"
        "Fields unknown",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF));
}

static const SubGhzProtocolDecoder ra_protocol_kce6xxx_decoder = {
    .alloc = ra_protocol_decoder_kce6xxx_alloc,
    .free = ra_protocol_decoder_kce6xxx_free,

    .feed = ra_protocol_decoder_kce6xxx_feed,
    .reset = ra_protocol_decoder_kce6xxx_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_kce6xxx_get_string,
};

const SubGhzProtocol ra_protocol_kce6xxx = {
    .name = SUBGHZ_PROTOCOL_KCE6XXX_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_kce6xxx_decoder,
    .encoder = NULL,
};
