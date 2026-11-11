#include "nero_intro_ii.h"
#include "nero_intro_ii_code.h"

#include <lib/subghz/blocks/generic.h>

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockGeneric generic;
    NeroIntroIICode code;
    uint8_t corrections;
} RaProtocolDecoderNeroIntroII;

static void* ra_protocol_decoder_nero_intro_ii_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderNeroIntroII* instance = malloc(sizeof(RaProtocolDecoderNeroIntroII));
    memset(instance, 0, sizeof(RaProtocolDecoderNeroIntroII));
    instance->base.protocol = &ra_protocol_nero_intro_ii;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_nero_intro_ii_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_nero_intro_ii_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderNeroIntroII* instance = context;
    nero_intro_ii_code_reset(&instance->code);
}

static void ra_protocol_decoder_nero_intro_ii_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderNeroIntroII* instance = context;
    NeroIntroIIFrame frame;

    if(!nero_intro_ii_code_feed(&instance->code, level, duration, &frame)) return;

    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    for(unsigned i = 0; i < NERO_INTRO_II_BITS; i++) {
        const bool bit = (frame.symbols[i / 7] >> (6 - (i % 7))) & 1;
        if(i < 64) {
            instance->generic.data = (instance->generic.data << 1) | bit;
        } else {
            instance->generic.data_2 = (instance->generic.data_2 << 1) | bit;
        }
    }
    instance->generic.data_count_bit = NERO_INTRO_II_BITS;
    instance->corrections = frame.corrections;

    /* The app resets every decoder from its receive callback, and the idle just consumed can
     * already hold the next frame's first sync chip - carry that tail across the callback. */
    const uint64_t sync = instance->code.sync;
    const uint8_t sync_count = instance->code.sync_count;
    if(instance->base.callback) instance->base.callback(&instance->base, instance->base.context);
    instance->code.sync = sync;
    instance->code.sync_count = sync_count;
}

static void ra_protocol_decoder_nero_intro_ii_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderNeroIntroII* instance = context;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:%08lX%08lX\r\n"
        "Data:%03lX%08lX\r\n"
        "Line code, edits:%u",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)instance->generic.data_2,
        instance->corrections);
}

static const SubGhzProtocolDecoder ra_protocol_nero_intro_ii_decoder = {
    .alloc = ra_protocol_decoder_nero_intro_ii_alloc,
    .free = ra_protocol_decoder_nero_intro_ii_free,

    .feed = ra_protocol_decoder_nero_intro_ii_feed,
    .reset = ra_protocol_decoder_nero_intro_ii_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_nero_intro_ii_get_string,
};

const SubGhzProtocol ra_protocol_nero_intro_ii = {
    .name = SUBGHZ_PROTOCOL_NERO_INTRO_II_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_nero_intro_ii_decoder,
    .encoder = NULL,
};
