#include "crypt.h"

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

#define TAG "RaCrypt"

#define CRYPT_PREAMBLE_TE    1025u // 5 * Te, the rate the preamble and sync cell run at
#define CRYPT_SYNC_LONG      2050u // 10 * Te
#define CRYPT_PREAMBLE_DELTA 250u
#define CRYPT_SYNC_DELTA     350u
#define CRYPT_PREAMBLE_MIN   8u
#define CRYPT_GAP_MIN        6000u

#define CRYPT_BITS_SHORT_SYNC 56u // sync high 5*Te, low 10*Te
#define CRYPT_BITS_LONG_SYNC  64u // sync high 10*Te, low 5*Te

static const SubGhzBlockConst ra_protocol_crypt_const = {
    .te_short = 210,
    .te_long = 615,
    .te_delta = 110,
    .min_count_bit_for_found = CRYPT_BITS_SHORT_SYNC,
};

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    bool lite; // 64 bits behind a 10*Te sync high, rather than 56 behind a 5*Te one
} RaProtocolDecoderCrypt;

/** The bit count this variant's sync cell promises. */
static uint8_t ra_protocol_crypt_bits(const RaProtocolDecoderCrypt* instance) {
    return instance->lite ? CRYPT_BITS_LONG_SYNC : CRYPT_BITS_SHORT_SYNC;
}

typedef enum {
    CryptDecoderStepReset = 0,
    CryptDecoderStepPreamble,
    CryptDecoderStepSyncHigh,
    CryptDecoderStepSyncLow,
    CryptDecoderStepSaveDuration,
    CryptDecoderStepCheckDuration,
} CryptDecoderStep;

static void* ra_protocol_decoder_crypt_alloc_variant(const SubGhzProtocol* protocol, bool lite) {
    RaProtocolDecoderCrypt* instance = malloc(sizeof(RaProtocolDecoderCrypt));
    instance->base.protocol = protocol;
    instance->generic.protocol_name = protocol->name;
    instance->lite = lite;
    return instance;
}

static void* ra_protocol_decoder_crypt_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return ra_protocol_decoder_crypt_alloc_variant(&ra_protocol_crypt, false);
}

static void* ra_protocol_decoder_crypt_lite_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return ra_protocol_decoder_crypt_alloc_variant(&ra_protocol_crypt_lite, true);
}

static void ra_protocol_decoder_crypt_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_crypt_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderCrypt* instance = context;
    instance->decoder.parser_step = CryptDecoderStepReset;
    instance->header_count = 0;
}

static void ra_protocol_crypt_add_bit(RaProtocolDecoderCrypt* instance, bool bit) {
    if(instance->decoder.decode_count_bit >= ra_protocol_crypt_bits(instance)) {
        // One bit past the length this sync cell promised, so the frame is not one of ours
        instance->decoder.decode_count_bit = UINT8_MAX;
        return;
    }
    instance->decoder.decode_data = (instance->decoder.decode_data << 1) | (bit ? 1u : 0u);
    instance->decoder.decode_count_bit++;
}

/** Reports the frame if it ran to exactly the length the sync cell promised. */
static void ra_protocol_crypt_finish(RaProtocolDecoderCrypt* instance) {
    if(instance->decoder.decode_count_bit != ra_protocol_crypt_bits(instance)) {
        return;
    }
    instance->generic.data = instance->decoder.decode_data;
    instance->generic.data_count_bit = ra_protocol_crypt_bits(instance);
    if(instance->base.callback) {
        instance->base.callback(&instance->base, instance->base.context);
    }
}

static void ra_protocol_decoder_crypt_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderCrypt* instance = context;
    const uint32_t te_short = (uint32_t)ra_protocol_crypt_const.te_short;
    const uint32_t te_long = (uint32_t)ra_protocol_crypt_const.te_long;
    const uint32_t te_delta = (uint32_t)ra_protocol_crypt_const.te_delta;

    switch(instance->decoder.parser_step) {
    case CryptDecoderStepReset:
        if(DURATION_DIFF(duration, CRYPT_PREAMBLE_TE) < CRYPT_PREAMBLE_DELTA) {
            instance->header_count = 1;
            instance->decoder.parser_step = CryptDecoderStepPreamble;
        }
        break;

    case CryptDecoderStepPreamble:
        if(DURATION_DIFF(duration, CRYPT_PREAMBLE_TE) < CRYPT_PREAMBLE_DELTA) {
            if(instance->header_count < UINT16_MAX) {
                instance->header_count++;
            }
            break;
        }
        // The preamble is separated from its own frame by the inter-frame gap
        if(!level && (instance->header_count >= CRYPT_PREAMBLE_MIN) &&
           (duration >= CRYPT_GAP_MIN)) {
            instance->decoder.parser_step = CryptDecoderStepSyncHigh;
            break;
        }
        ra_protocol_decoder_crypt_reset(instance);
        break;

    case CryptDecoderStepSyncHigh: {
        const uint32_t want = instance->lite ? CRYPT_SYNC_LONG : CRYPT_PREAMBLE_TE;
        const uint32_t want_delta = instance->lite ? CRYPT_SYNC_DELTA : CRYPT_PREAMBLE_DELTA;
        if(level && (DURATION_DIFF(duration, want) < want_delta)) {
            instance->decoder.parser_step = CryptDecoderStepSyncLow;
            break;
        }
        ra_protocol_decoder_crypt_reset(instance);
        break;
    }

    case CryptDecoderStepSyncLow: {
        // The cell is one direction or the other, never two halves of the same length
        const uint32_t want = instance->lite ? CRYPT_PREAMBLE_TE : CRYPT_SYNC_LONG;
        const uint32_t want_delta = instance->lite ? CRYPT_PREAMBLE_DELTA : CRYPT_SYNC_DELTA;
        if(!level && (DURATION_DIFF(duration, want) < want_delta)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = CryptDecoderStepSaveDuration;
            break;
        }
        ra_protocol_decoder_crypt_reset(instance);
        break;
    }

    case CryptDecoderStepSaveDuration:
        if(!level) {
            ra_protocol_decoder_crypt_reset(instance);
            break;
        }
        /* Data runs straight into the next preamble, so a high that is neither symbol is the
         * end of the frame rather than an error. */
        if((DURATION_DIFF(duration, te_short) >= te_delta) &&
           (DURATION_DIFF(duration, te_long) >= te_delta * 2)) {
            ra_protocol_crypt_finish(instance);
            ra_protocol_decoder_crypt_reset(instance);
            break;
        }
        instance->decoder.te_last = duration;
        instance->decoder.parser_step = CryptDecoderStepCheckDuration;
        break;

    case CryptDecoderStepCheckDuration:
        if(level) {
            ra_protocol_decoder_crypt_reset(instance);
            break;
        }
        if((DURATION_DIFF(instance->decoder.te_last, te_long) < te_delta * 2) &&
           (DURATION_DIFF(duration, te_short) < te_delta)) {
            ra_protocol_crypt_add_bit(instance, true);
        } else if(
            (DURATION_DIFF(instance->decoder.te_last, te_short) < te_delta) &&
            (DURATION_DIFF(duration, te_long) < te_delta * 2)) {
            ra_protocol_crypt_add_bit(instance, false);
        } else {
            ra_protocol_crypt_finish(instance);
            ra_protocol_decoder_crypt_reset(instance);
            break;
        }
        instance->decoder.parser_step = CryptDecoderStepSaveDuration;
        break;
    }
}

static void ra_protocol_decoder_crypt_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderCrypt* instance = context;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:%08lX%08lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF));
}

static const SubGhzProtocolDecoder ra_protocol_crypt_decoder = {
    .alloc = ra_protocol_decoder_crypt_alloc,
    .free = ra_protocol_decoder_crypt_free,

    .feed = ra_protocol_decoder_crypt_feed,
    .reset = ra_protocol_decoder_crypt_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_crypt_get_string,
};

static const SubGhzProtocolDecoder ra_protocol_crypt_lite_decoder = {
    .alloc = ra_protocol_decoder_crypt_lite_alloc,
    .free = ra_protocol_decoder_crypt_free,

    .feed = ra_protocol_decoder_crypt_feed,
    .reset = ra_protocol_decoder_crypt_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_crypt_get_string,
};

const SubGhzProtocol ra_protocol_crypt = {
    .name = SUBGHZ_PROTOCOL_CRYPT_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_crypt_decoder,
    .encoder = NULL,
};

const SubGhzProtocol ra_protocol_crypt_lite = {
    .name = SUBGHZ_PROTOCOL_CRYPT_LITE_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_crypt_lite_decoder,
    .encoder = NULL,
};
