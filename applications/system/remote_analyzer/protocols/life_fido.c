#include "life_fido.h"

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

#define TAG "RaLifeFido"

/* Bits 0-11, and bits 45-54 further in: the two runs that are identical in every frame of
 * both remotes seen so far. Bits 12-13 are not among them - one remote sends 01 there and
 * the other 10 - which is why the leading check stops at bit 11 instead of taking a whole
 * 16-bit header. Together they are 22 bits, and they are what keeps a Manchester decoder
 * from reporting noise, since any 1T/2T stream is valid Manchester. */
#define LIFE_FIDO_HEADER     0xC6u
#define LIFE_FIDO_HEADER_NIB 0x1u
#define LIFE_FIDO_MARK_SHIFT 17u
#define LIFE_FIDO_MARK_MASK  0x3FFu
#define LIFE_FIDO_MARK       0x02Bu

/* Widths are split at the midpoint between 1 and 2 half-bits rather than matched against a
 * band each, so no width can land in a gap between the two and kill a frame. The sync low is
 * nominally 6.5*te_short; the window is wide because it is the gate the whole frame depends
 * on, and it stays far below the shortest gap between frames (15*te_short). */
#define LIFE_FIDO_PULSE_MIN (404u / 2u) // 202
#define LIFE_FIDO_PULSE_MID ((404u * 3u) / 2u) // 606
#define LIFE_FIDO_PULSE_MAX ((404u * 5u) / 2u) // 1010
#define LIFE_FIDO_SYNC_MIN  (404u * 4u) // 1616
#define LIFE_FIDO_SYNC_MAX  (404u * 10u) // 4040

#define LIFE_FIDO_PREAMBLE_MIN 10u

static const SubGhzBlockConst ra_protocol_life_fido_const = {
    .te_short = 404,
    .te_long = 808,
    .te_delta = 120,
    .min_count_bit_for_found = 72,
};

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    uint8_t data_high; // bits 0..7 of the frame; the rest live in decoder.decode_data
    bool lead_skipped;
    bool half_pending;
    bool half_level;
} RaProtocolDecoderLifeFido;

/** Half-bits a pulse carries, or 0 when its width is not one of them. */
static uint8_t ra_protocol_life_fido_halves(uint32_t duration) {
    if(duration < LIFE_FIDO_PULSE_MIN) return 0;
    if(duration < LIFE_FIDO_PULSE_MID) return 1;
    if(duration <= LIFE_FIDO_PULSE_MAX) return 2;
    return 0;
}

typedef enum {
    LifeFidoDecoderStepReset = 0,
    LifeFidoDecoderStepPreamble,
    LifeFidoDecoderStepData,
} LifeFidoDecoderStep;

static void* ra_protocol_decoder_life_fido_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderLifeFido* instance = malloc(sizeof(RaProtocolDecoderLifeFido));
    instance->base.protocol = &ra_protocol_life_fido;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_life_fido_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_life_fido_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderLifeFido* instance = context;
    instance->decoder.parser_step = LifeFidoDecoderStepReset;
    instance->header_count = 0;
}

/** Clears only the frame in progress, so a failed one cannot overwrite the last good read. */
static void ra_protocol_life_fido_frame_reset(RaProtocolDecoderLifeFido* instance) {
    instance->decoder.decode_data = 0;
    instance->decoder.decode_count_bit = 0;
    instance->data_high = 0;
    instance->lead_skipped = false;
    instance->half_pending = false;
}

/**
 * Appends one half-bit to the frame in progress.
 * @return false when the pair did not transition, which is not Manchester
 */
static bool ra_protocol_life_fido_add_half(RaProtocolDecoderLifeFido* instance, bool level) {
    // The first half-bit after the sync low is part of the sync, not of bit 0
    if(!instance->lead_skipped) {
        instance->lead_skipped = true;
        return true;
    }
    if(!instance->half_pending) {
        instance->half_pending = true;
        instance->half_level = level;
        return true;
    }
    instance->half_pending = false;
    if(instance->half_level == level) {
        return false;
    }

    // High then low is a one, and the pair's first half already carries that value
    instance->data_high = (uint8_t)((instance->data_high << 1) |
                                    (uint8_t)((instance->decoder.decode_data >> 63) & 1));
    instance->decoder.decode_data = (instance->decoder.decode_data << 1) |
                                    (instance->half_level ? 1u : 0u);
    instance->decoder.decode_count_bit++;
    return true;
}

static void ra_protocol_life_fido_report(RaProtocolDecoderLifeFido* instance) {
    if((instance->data_high != LIFE_FIDO_HEADER) ||
       (((instance->decoder.decode_data >> 60) & 0xFu) != LIFE_FIDO_HEADER_NIB) ||
       (((instance->decoder.decode_data >> LIFE_FIDO_MARK_SHIFT) & LIFE_FIDO_MARK_MASK) !=
        LIFE_FIDO_MARK)) {
        return;
    }
    instance->generic.data = instance->decoder.decode_data;
    instance->generic.data_2 = instance->data_high;
    instance->generic.data_count_bit = ra_protocol_life_fido_const.min_count_bit_for_found;
    if(instance->base.callback) {
        instance->base.callback(&instance->base, instance->base.context);
    }
}

static void ra_protocol_decoder_life_fido_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderLifeFido* instance = context;

    switch(instance->decoder.parser_step) {
    case LifeFidoDecoderStepReset:
        if(ra_protocol_life_fido_halves(duration) == 1) {
            instance->header_count = 1;
            instance->decoder.parser_step = LifeFidoDecoderStepPreamble;
        }
        break;

    case LifeFidoDecoderStepPreamble:
        if(ra_protocol_life_fido_halves(duration) == 1) {
            if(instance->header_count < UINT16_MAX) {
                instance->header_count++;
            }
            break;
        }
        if(!level && (instance->header_count >= LIFE_FIDO_PREAMBLE_MIN) &&
           (duration >= LIFE_FIDO_SYNC_MIN) && (duration <= LIFE_FIDO_SYNC_MAX)) {
            ra_protocol_life_fido_frame_reset(instance);
            instance->decoder.parser_step = LifeFidoDecoderStepData;
            break;
        }
        ra_protocol_decoder_life_fido_reset(instance);
        break;

    case LifeFidoDecoderStepData: {
        uint8_t halves = ra_protocol_life_fido_halves(duration);
        if(halves == 0) {
            ra_protocol_decoder_life_fido_reset(instance);
            break;
        }
        while(halves--) {
            if(!ra_protocol_life_fido_add_half(instance, level)) {
                ra_protocol_decoder_life_fido_reset(instance);
                return;
            }
            if(instance->decoder.decode_count_bit ==
               ra_protocol_life_fido_const.min_count_bit_for_found) {
                ra_protocol_life_fido_report(instance);
                ra_protocol_decoder_life_fido_reset(instance);
                return;
            }
        }
        break;
    }
    }
}

static void ra_protocol_decoder_life_fido_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderLifeFido* instance = context;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:%02lX%08lX%08lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data_2 & 0xFF),
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF));
}

static const SubGhzProtocolDecoder ra_protocol_life_fido_decoder = {
    .alloc = ra_protocol_decoder_life_fido_alloc,
    .free = ra_protocol_decoder_life_fido_free,

    .feed = ra_protocol_decoder_life_fido_feed,
    .reset = ra_protocol_decoder_life_fido_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_life_fido_get_string,
};

const SubGhzProtocol ra_protocol_life_fido = {
    .name = SUBGHZ_PROTOCOL_LIFE_FIDO_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_life_fido_decoder,
    .encoder = NULL,
};
