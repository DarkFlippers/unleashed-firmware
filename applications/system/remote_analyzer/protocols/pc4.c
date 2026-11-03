#include "pc4.h"

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

/* Decoder by @xMasterX (MMX), 2025.04. Header handling and the field split reworked against
 * the three DRX-2M captures - see pc4.h. */

static const SubGhzBlockConst ra_protocol_pc4_const = {
    .te_short = 400,
    .te_long = 800,
    .te_delta = 150, // observed 295..420 and 700..860, still disjoint

    .min_count_bit_for_found = 56,
};

/* Two header cells in two of the captures and three in the third, so any count of at least
 * two is accepted. */
#define PC4_HEADER_US    2000
#define PC4_HEADER_DELTA 600
#define PC4_HEADER_MIN   2

#define PC4_GAP_US    16200 // observed 16086..16652
#define PC4_GAP_DELTA 900

// The only bits that hold still across a capture
#define PC4_ID_SHIFT  36
#define PC4_AUX_SHIFT 24

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint8_t header_count;
} RaProtocolDecoderPc4;

typedef enum {
    Pc4DecoderStepReset = 0,
    Pc4DecoderStepHeader,
    Pc4DecoderStepSaveDuration,
    Pc4DecoderStepCheckDuration,
} Pc4DecoderStep;

static void* ra_protocol_decoder_pc4_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderPc4* instance = malloc(sizeof(RaProtocolDecoderPc4));
    instance->base.protocol = &ra_protocol_pc4;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_pc4_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_pc4_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderPc4* instance = context;
    instance->decoder.parser_step = Pc4DecoderStepReset;
    instance->header_count = 0;
}

static bool ra_protocol_pc4_is_short(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_pc4_const.te_short) <
           ra_protocol_pc4_const.te_delta;
}

static bool ra_protocol_pc4_is_long(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_pc4_const.te_long) < ra_protocol_pc4_const.te_delta;
}

static bool ra_protocol_pc4_is_header(uint32_t duration) {
    return DURATION_DIFF(duration, PC4_HEADER_US) < PC4_HEADER_DELTA;
}

static bool ra_protocol_pc4_is_gap(uint32_t duration) {
    return DURATION_DIFF(duration, PC4_GAP_US) < PC4_GAP_DELTA;
}

static void ra_protocol_decoder_pc4_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderPc4* instance = context;

    switch(instance->decoder.parser_step) {
    case Pc4DecoderStepReset:
        if((!level) && ra_protocol_pc4_is_gap(duration)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->header_count = 0;
            instance->decoder.parser_step = Pc4DecoderStepHeader;
        }
        break;

    case Pc4DecoderStepHeader:
        if(ra_protocol_pc4_is_header(duration)) {
            if(!level) instance->header_count++; // one cell is a high and a low
            break;
        }
        /* The first high that is not a header cell belongs to the first bit, so taking it here
         * is what tells the two- and three-cell headers apart. */
        if(level && (instance->header_count >= PC4_HEADER_MIN)) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Pc4DecoderStepCheckDuration;
            break;
        }
        instance->header_count = 0;
        instance->decoder.parser_step = Pc4DecoderStepReset;
        break;

    case Pc4DecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Pc4DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Pc4DecoderStepReset;
        }
        break;

    case Pc4DecoderStepCheckDuration:
        if(level) {
            instance->decoder.parser_step = Pc4DecoderStepReset;
            break;
        }

        /* te_last is the short trailing high after the last cell and carries no bit, so a
         * full frame is exactly 56 bits by the time the gap arrives. */
        if(ra_protocol_pc4_is_gap(duration)) {
            if(instance->decoder.decode_count_bit ==
               ra_protocol_pc4_const.min_count_bit_for_found) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->header_count = 0;
            instance->decoder.parser_step = Pc4DecoderStepHeader;
            break;
        }

        if(ra_protocol_pc4_is_short(instance->decoder.te_last) &&
           ra_protocol_pc4_is_long(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
            instance->decoder.parser_step = Pc4DecoderStepSaveDuration;
        } else if(
            ra_protocol_pc4_is_long(instance->decoder.te_last) &&
            ra_protocol_pc4_is_short(duration)) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
            instance->decoder.parser_step = Pc4DecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = Pc4DecoderStepReset;
        }
        break;
    }
}

static void ra_protocol_decoder_pc4_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderPc4* instance = context;

    /* Everything but these two nibbles rolls. Id is the one field stable per remote; Aux is
     * nearly stable and is the next thing worth watching if more captures turn up. */
    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%06lX%08lX\r\n"
        "Id:%X Aux:%X\r\n"
        "Rolling, other fields unknown",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        (unsigned int)((instance->generic.data >> PC4_ID_SHIFT) & 0xF),
        (unsigned int)((instance->generic.data >> PC4_AUX_SHIFT) & 0xF));
}

static const SubGhzProtocolDecoder ra_protocol_pc4_decoder = {
    .alloc = ra_protocol_decoder_pc4_alloc,
    .free = ra_protocol_decoder_pc4_free,

    .feed = ra_protocol_decoder_pc4_feed,
    .reset = ra_protocol_decoder_pc4_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_pc4_get_string,
};

const SubGhzProtocol ra_protocol_pc4 = {
    .name = SUBGHZ_PROTOCOL_PC4_NAME,
    // Every bit but one nibble changes frame to frame, whatever the cipher turns out to be
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_pc4_decoder,
    .encoder = NULL,
};
