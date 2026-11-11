#include "belgate.h"

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

static const SubGhzBlockConst ra_protocol_belgate_const = {
    .te_short = 480,
    .te_long = 960,
    .te_delta = 200, // observed 410..620 and 780..1120, still disjoint

    .min_count_bit_for_found = 119,
};

/* A header is 7.7ms, but the first frame of a press follows a doubled one and the last is
 * followed by seconds of silence, so any low this long or longer opens a frame. */
#define BELGATE_HEADER_MIN_US 6500

// Field boundaries, as bit counts from the first pulse
#define BELGATE_PREAMBLE_BITS 16
#define BELGATE_SERIAL_END    48
#define BELGATE_PAYLOAD_END   112
#define BELGATE_BTN_END       118
#define BELGATE_PREAMBLE      0x8017

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t preamble; // bits 0..15
    uint32_t serial; // bits 16..47
    uint64_t payload; // bits 48..111
    uint8_t btn_mask; // bits 112..117, one-hot
    uint8_t count; // pulses taken into the frame so far
} RaProtocolDecoderBelGate;

typedef enum {
    BelGateDecoderStepReset = 0,
    BelGateDecoderStepData,
} BelGateDecoderStep;

static void* ra_protocol_decoder_belgate_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderBelGate* instance = malloc(sizeof(RaProtocolDecoderBelGate));
    instance->base.protocol = &ra_protocol_belgate;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_belgate_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_belgate_frame_reset(RaProtocolDecoderBelGate* instance) {
    instance->preamble = 0;
    instance->serial = 0;
    instance->payload = 0;
    instance->btn_mask = 0;
    instance->count = 0;
}

static void ra_protocol_decoder_belgate_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderBelGate* instance = context;
    instance->decoder.parser_step = BelGateDecoderStepReset;
    ra_protocol_decoder_belgate_frame_reset(instance);
}

static bool ra_protocol_belgate_is_short(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_belgate_const.te_short) <
           ra_protocol_belgate_const.te_delta;
}

static bool ra_protocol_belgate_is_long(uint32_t duration) {
    return DURATION_DIFF(duration, ra_protocol_belgate_const.te_long) <
           ra_protocol_belgate_const.te_delta;
}

static void ra_protocol_decoder_belgate_add_bit(RaProtocolDecoderBelGate* instance, bool bit) {
    const uint8_t i = instance->count;

    if(i < BELGATE_PREAMBLE_BITS) {
        instance->preamble = (uint16_t)((instance->preamble << 1) | (bit ? 1U : 0U));
    } else if(i < BELGATE_SERIAL_END) {
        instance->serial = (instance->serial << 1) | (bit ? 1UL : 0UL);
    } else if(i < BELGATE_PAYLOAD_END) {
        instance->payload = (instance->payload << 1) | (bit ? 1ULL : 0ULL);
    } else if(i < BELGATE_BTN_END) {
        instance->btn_mask = (uint8_t)((instance->btn_mask << 1) | (bit ? 1U : 0U));
    }
    // Bit 118 reads 1 in about half the frames of one press, so it is counted but not kept.

    if(instance->count < ra_protocol_belgate_const.min_count_bit_for_found) instance->count++;
}

/** One-hot: the four-button capture gave four single-bit values and nothing else. */
static bool ra_protocol_belgate_btn_valid(uint8_t mask) {
    return (mask != 0) && ((mask & (uint8_t)(mask - 1U)) == 0);
}

static void ra_protocol_decoder_belgate_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderBelGate* instance = context;

    switch(instance->decoder.parser_step) {
    case BelGateDecoderStepReset:
        if((!level) && (duration >= BELGATE_HEADER_MIN_US)) {
            ra_protocol_decoder_belgate_frame_reset(instance);
            instance->decoder.parser_step = BelGateDecoderStepData;
        }
        break;

    case BelGateDecoderStepData:
        /* The header opening the next frame closes this one. The trailing high is already
         * taken, so a whole frame is exactly 119 pulses by now. */
        if((!level) && (duration >= BELGATE_HEADER_MIN_US)) {
            if((instance->count == ra_protocol_belgate_const.min_count_bit_for_found) &&
               (instance->preamble == BELGATE_PREAMBLE) &&
               ra_protocol_belgate_btn_valid(instance->btn_mask)) {
                instance->generic.data = instance->payload;
                instance->generic.data_count_bit =
                    ra_protocol_belgate_const.min_count_bit_for_found;
                instance->generic.serial = instance->serial;
                instance->generic.btn = instance->btn_mask;

                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            ra_protocol_decoder_belgate_frame_reset(instance);
            break;
        }

        if(ra_protocol_belgate_is_short(duration)) {
            ra_protocol_decoder_belgate_add_bit(instance, false);
        } else if(ra_protocol_belgate_is_long(duration)) {
            ra_protocol_decoder_belgate_add_bit(instance, true);
        } else {
            instance->decoder.parser_step = BelGateDecoderStepReset;
            ra_protocol_decoder_belgate_frame_reset(instance);
        }
        break;
    }
}

static void ra_protocol_decoder_belgate_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderBelGate* instance = context;

    // Reported as the index of the set bit, so it reads 1..6 and not the raw mask
    uint8_t btn = 0;
    for(uint8_t mask = instance->generic.btn; mask; mask >>= 1)
        btn++;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Sn:0x%08lX Btn:%u\r\n"
        "Rolling:%08lX%08lX",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.serial,
        (unsigned int)btn,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF));
}

static const SubGhzProtocolDecoder ra_protocol_belgate_decoder = {
    .alloc = ra_protocol_decoder_belgate_alloc,
    .free = ra_protocol_decoder_belgate_free,

    .feed = ra_protocol_decoder_belgate_feed,
    .reset = ra_protocol_decoder_belgate_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_belgate_get_string,
};

const SubGhzProtocol ra_protocol_belgate = {
    .name = SUBGHZ_PROTOCOL_BELGATE_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_belgate_decoder,
    .encoder = NULL,
};
