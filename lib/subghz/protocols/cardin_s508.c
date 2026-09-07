#include "cardin_s508.h"

#include <lib/toolbox/manchester_decoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolCardinS508"

/*
 * Only min_count_bit_for_found (the stored payload length) is used from here;
 * the nominal half-bit is ~100us (bit cell ~200us) but the on-air pulse widths
 * are asymmetric (see the per-level bands below), so classification does NOT go
 * through te_short/te_long/te_delta.
 */
static const SubGhzBlockConst subghz_protocol_cardin_s508_const = {
    .te_short = 100,
    .te_long = 200,
    .te_delta = 50,
    .min_count_bit_for_found = 128,
};

/*
 * Per-level Manchester timing bands, measured from a real FM12K capture.
 * The 2-FSK slicer is duty-cycle biased: a half-bit reads ~110-150us HIGH but
 * only ~75-120us LOW, and a full cell ~225us HIGH / ~175us LOW. A single
 * symmetric threshold can't separate a short HIGH (~150) from a long LOW (~175),
 * so HIGH and LOW get their own short/long boundaries. Anything outside these
 * bands (noise spike, inter-burst gap, the wakeup burst) re-syncs the decoder.
 */
#define CARDIN_S508_PULSE_MIN    40
#define CARDIN_S508_HI_SHORT_MAX 165
#define CARDIN_S508_HI_LONG_MAX  340
#define CARDIN_S508_LO_SHORT_MAX 135
#define CARDIN_S508_LO_LONG_MAX  300

/* 12-bit fixed sync 110011110100 and its bitwise complement. We accept either:
 * the repo's manchester_advance has a fixed polarity that (as the real capture
 * confirms) decodes Cardin's "1 = high-then-low" convention as the COMPLEMENT,
 * so we lock whichever form appears and, when it was the complement, invert the
 * 128 payload bits so the reported codeword is always canonical (begins with
 * 110011110100). The long constant-bit preamble before the sync establishes
 * Manchester phase. */
#define CARDIN_S508_SYNC         0x0CF4u
#define CARDIN_S508_SYNC_INV     0x030Bu
#define CARDIN_S508_SYNC_MASK    0x0FFFu
#define CARDIN_S508_PAYLOAD_BITS 128u

struct SubGhzProtocolDecoderCardinS508 {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t sync_shift; /* rolling 12-bit window while hunting for the sync */
    uint16_t payload_count; /* payload bits collected since the sync locked */
    uint64_t payload_hi; /* payload bits [127:64] */
    uint64_t payload_lo; /* payload bits [63:0] */
    bool sync_locked; /* sync seen, now collecting the 128-bit payload */
    bool inverted; /* sync matched the complement -> invert the payload */
};

/* Unused: this protocol is decode-only. The struct exists only to mirror the
 * repo's protocol layout; the encoder vtable below is all-NULL. */
struct SubGhzProtocolEncoderCardinS508 {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

const SubGhzProtocolDecoder subghz_protocol_cardin_s508_decoder = {
    .alloc = subghz_protocol_decoder_cardin_s508_alloc,
    .free = subghz_protocol_decoder_cardin_s508_free,
    .feed = subghz_protocol_decoder_cardin_s508_feed,
    .reset = subghz_protocol_decoder_cardin_s508_reset,
    .get_hash_data = subghz_protocol_decoder_cardin_s508_get_hash_data,
    .serialize = subghz_protocol_decoder_cardin_s508_serialize,
    .deserialize = subghz_protocol_decoder_cardin_s508_deserialize,
    .get_string = subghz_protocol_decoder_cardin_s508_get_string,
};

/* Decode-only: no encoder. Mirrors the iDo 117/111 decode-only protocol. */
const SubGhzProtocolEncoder subghz_protocol_cardin_s508_encoder = {
    .alloc = NULL,
    .free = NULL,
    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_cardin_s508 = {
    .name = SUBGHZ_PROTOCOL_CARDIN_S508_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_868 | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Save,
    .decoder = &subghz_protocol_cardin_s508_decoder,
    .encoder = &subghz_protocol_cardin_s508_encoder,
};

/* ------------------------------- decoder -------------------------------- */

static void subghz_protocol_decoder_cardin_s508_reset_accumulators(
    SubGhzProtocolDecoderCardinS508* instance) {
    instance->sync_shift = 0;
    instance->sync_locked = false;
    instance->inverted = false;
    instance->payload_hi = 0;
    instance->payload_lo = 0;
    instance->payload_count = 0;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void* subghz_protocol_decoder_cardin_s508_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderCardinS508* instance = malloc(sizeof(SubGhzProtocolDecoderCardinS508));
    instance->base.protocol = &subghz_protocol_cardin_s508;
    instance->generic.protocol_name = instance->base.protocol->name;
    subghz_protocol_decoder_cardin_s508_reset_accumulators(instance);
    return instance;
}

void subghz_protocol_decoder_cardin_s508_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    free(instance);
}

void subghz_protocol_decoder_cardin_s508_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    subghz_protocol_decoder_cardin_s508_reset_accumulators(instance);
}

/* One decoded Manchester bit: while hunting, slide it into the 12-bit window and
 * watch for the sync; once locked, shift it into the 128-bit payload and publish
 * a frame when 128 bits are in. */
static void
    subghz_protocol_decoder_cardin_s508_add_bit(SubGhzProtocolDecoderCardinS508* instance, bool bit) {
    if(!instance->sync_locked) {
        instance->sync_shift = ((instance->sync_shift << 1) | (bit ? 1u : 0u)) &
                               CARDIN_S508_SYNC_MASK;
        if(instance->sync_shift == CARDIN_S508_SYNC) {
            instance->sync_locked = true;
            instance->inverted = false;
        } else if(instance->sync_shift == CARDIN_S508_SYNC_INV) {
            instance->sync_locked = true;
            instance->inverted = true;
        } else {
            return;
        }
        instance->payload_hi = 0;
        instance->payload_lo = 0;
        instance->payload_count = 0;
        return;
    }

    instance->payload_hi = (instance->payload_hi << 1) | (instance->payload_lo >> 63);
    instance->payload_lo = (instance->payload_lo << 1) | (bit ? 1u : 0u);
    if(++instance->payload_count < CARDIN_S508_PAYLOAD_BITS) {
        return;
    }

    uint64_t hi = instance->inverted ? ~instance->payload_hi : instance->payload_hi;
    uint64_t lo = instance->inverted ? ~instance->payload_lo : instance->payload_lo;
    instance->generic.data = lo;
    instance->generic.data_2 = hi;
    instance->generic.data_count_bit = CARDIN_S508_PAYLOAD_BITS;
    /* keep the decoder block in sync for hashing */
    instance->decoder.decode_data = lo;
    instance->decoder.decode_count_bit = CARDIN_S508_PAYLOAD_BITS;
    if(instance->base.callback) {
        instance->base.callback(&instance->base, instance->base.context);
    }
    /* ready to catch the next repeat of the same press */
    instance->sync_locked = false;
    instance->sync_shift = 0;
}

/* Classify one pulse into a Manchester short/long event using per-level bands.
 * Returns false (caller re-syncs) for anything outside the bit bands. */
static bool
    subghz_protocol_decoder_cardin_s508_classify(bool level, uint32_t duration, bool* is_short) {
    if(duration < CARDIN_S508_PULSE_MIN) return false;
    uint32_t short_max = level ? CARDIN_S508_HI_SHORT_MAX : CARDIN_S508_LO_SHORT_MAX;
    uint32_t long_max = level ? CARDIN_S508_HI_LONG_MAX : CARDIN_S508_LO_LONG_MAX;
    if(duration < short_max) {
        *is_short = true;
        return true;
    }
    if(duration < long_max) {
        *is_short = false;
        return true;
    }
    return false;
}

void subghz_protocol_decoder_cardin_s508_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;

    bool is_short;
    if(!subghz_protocol_decoder_cardin_s508_classify(level, duration, &is_short)) {
        /* gap / wakeup burst / noise: re-sync and keep scanning for the sync */
        subghz_protocol_decoder_cardin_s508_reset_accumulators(instance);
        return;
    }

    ManchesterEvent event;
    if(is_short) {
        event = level ? ManchesterEventShortHigh : ManchesterEventShortLow;
    } else {
        event = level ? ManchesterEventLongHigh : ManchesterEventLongLow;
    }
    bool data;
    if(manchester_advance(
           instance->manchester_saved_state, event, &instance->manchester_saved_state, &data)) {
        subghz_protocol_decoder_cardin_s508_add_bit(instance, data);
    }
}

uint8_t subghz_protocol_decoder_cardin_s508_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    /* hash the low 64 payload bits; every press rolls all 128 bits, so this is
     * unique per press while deduping the repeats of a single press */
    return subghz_protocol_blocks_get_hash_data(&instance->decoder, sizeof(uint64_t));
}

SubGhzProtocolStatus subghz_protocol_decoder_cardin_s508_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    /* generic_serialize writes Bit + the low 64 bits as "Key"; the high 64 bits
     * (data_2) need an extra field, same pattern as kinggates_stylo_4k. */
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> (i * 8)) & 0xFF;
    }
    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Data");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_cardin_s508_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_cardin_s508_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        instance->generic.data_2 = 0;
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }
    } while(false);
    return ret;
}

void subghz_protocol_decoder_cardin_s508_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    /* Opaque, rolling AES-128 payload: show the constant sync and the 128-bit
     * codeword hex; deliberately no serial/button/counter (none exist in the
     * clear) and no TX. */
    furi_string_cat_printf(
        output,
        "%s\r\n"
        "Rolling (AES-128)\r\n"
        "Sync:110011110100\r\n"
        "Key:%08lX%08lX%08lX%08lX\r\n",
        instance->generic.protocol_name,
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)(instance->generic.data_2 & 0xFFFFFFFF),
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF));
}
