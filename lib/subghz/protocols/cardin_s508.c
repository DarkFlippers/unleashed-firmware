#include "cardin_s508.h"

#include <lib/toolbox/manchester_decoder.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolCardinS508"

/* Fixed 12-bit sync from the capture. Manchester phase/polarity can make the
 * decoder observe either the canonical value or its complement; both are
 * accepted and the payload is canonicalized before it is saved. */
#define CARDIN_S508_SYNC         0x0CF4u
#define CARDIN_S508_SYNC_INV     0x030Bu
#define CARDIN_S508_SYNC_MASK    0x0FFFu
#define CARDIN_S508_PAYLOAD_BITS 128u

#define CARDIN_S508_PULSE_MIN    40u
#define CARDIN_S508_HI_SHORT_MAX 165u
#define CARDIN_S508_HI_LONG_MAX  340u
#define CARDIN_S508_LO_SHORT_MAX 135u
#define CARDIN_S508_LO_LONG_MAX  300u

#define CARDIN_S508_TE            100u
#define CARDIN_S508_PREAMBLE_BITS 400u
#define CARDIN_S508_GAP           50000u
#define CARDIN_S508_UPLOAD_SIZE   1280u

/* These constants and the 8-cycle primitive match CalcKeyS500 in the
 * official Cardin DecCardin.dll (CARDINTX_SW V1.43). This is deliberately
 * kept local to the protocol instead of importing the proprietary DLL. */
#define CARDIN_S508_KEY_BLOCK_1     0xB6C4A8E2u
#define CARDIN_S508_KEY_BLOCK_2     0x6723F4B1u
#define CARDIN_S508_TEA_DELTA       0x9E3779B9u
#define CARDIN_S508_TEA_INITIAL_SUM 0xC6EF3720u

/*
 * The receiver wake-up burst is not part of the decoded Manchester word. It
 * is nevertheless needed for replay: this representative timing shape was
 * measured from the clean raw capture added by PR #1004. It varies slightly
 * between button presses.
 */
static const int16_t cardin_s508_wakeup[] = {
    1101, -102, 293,  -98,  703, -98,  701,  -98,  295,  -98,  319, -72,  733,  -74,
    1111, -102, 295,  -98,  701, -98,  701,  -98,  903,  -102, 695, -100, 1699, -100,
    295,  -100, 705,  -98,  319, -72,  319,  -74,  733,  -74,  317, -74,  319,  -98,
    697,  -102, 295,  -98,  915, -74,  1723, -74,  1119, -102, 505, -78,  499,  -122,
    279,  -126, 1059, -126, 679, -124, 281,  -128, 251,  -150, 448, -154, 227,  -52,
};

static const SubGhzBlockConst subghz_protocol_cardin_s508_const = {
    .te_short = CARDIN_S508_TE,
    .te_long = CARDIN_S508_TE * 2,
    .te_delta = 50,
    .min_count_bit_for_found = CARDIN_S508_PAYLOAD_BITS,
};

/*
 * The FM12K slicer produces different HIGH and LOW durations. A common
 * threshold would confuse a short HIGH with a long LOW, so the decoder keeps
 * separate short/long bands. Durations outside these bands (idle, wake-up
 * burst, or noise) reset Manchester state and let the sync search resume.
 */

struct SubGhzProtocolDecoderCardinS508 {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t sync_shift;
    uint16_t payload_count;
    uint64_t payload_hi;
    uint64_t payload_lo;
    bool sync_locked;
    bool inverted;
    bool rolling;
    uint32_t rolling_counter;
};

struct SubGhzProtocolEncoderCardinS508 {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
    uint64_t rolling_key_hi;
    uint64_t rolling_key_lo;
    uint32_t rolling_counter;
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

const SubGhzProtocolEncoder subghz_protocol_cardin_s508_encoder = {
    .alloc = subghz_protocol_encoder_cardin_s508_alloc,
    .free = subghz_protocol_encoder_cardin_s508_free,
    .deserialize = subghz_protocol_encoder_cardin_s508_deserialize,
    .stop = subghz_protocol_encoder_cardin_s508_stop,
    .yield = subghz_protocol_encoder_cardin_s508_yield,
};

const SubGhzProtocol subghz_protocol_cardin_s508 = {
    .name = SUBGHZ_PROTOCOL_CARDIN_S508_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_868 | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,
    .decoder = &subghz_protocol_cardin_s508_decoder,
    .encoder = &subghz_protocol_cardin_s508_encoder,
};

static void subghz_protocol_cardin_s508_calc_key_block(
    uint32_t* value_hi,
    uint32_t* value_lo,
    const uint32_t key[4]) {
    /* CalcKeyS500 uses the reverse TEA direction. EncryptS500_VB6, which is
     * a separate export in the same DLL, uses the forward direction. */
    uint32_t sum = CARDIN_S508_TEA_INITIAL_SUM;
    uint32_t v0 = *value_hi;
    uint32_t v1 = *value_lo;

    /* This is the exact 8-cycle sequence used by CalcKeyS500: the first
     * half-round uses key[(sum >> 11) & 3], then sum is decremented, and the
     * second half-round uses key[sum & 3]. */
    for(uint8_t cycle = 0; cycle < 8; cycle++) {
        uint32_t mix = (((v0 << 4) ^ (v0 >> 5)) + v0);
        v1 -= mix ^ (sum + key[(sum >> 11) & 3u]);
        sum -= CARDIN_S508_TEA_DELTA;

        mix = (((v1 << 4) ^ (v1 >> 5)) + v1);
        v0 -= mix ^ (sum + key[sum & 3u]);
    }

    *value_hi = v0;
    *value_lo = v1;
}

void subghz_protocol_cardin_s508_generate_payload(
    uint64_t key_hi,
    uint64_t key_lo,
    uint32_t counter,
    uint64_t* payload_hi,
    uint64_t* payload_lo) {
    furi_assert(payload_hi);
    furi_assert(payload_lo);

    const uint32_t key[4] = {
        (uint32_t)(key_hi >> 32),
        (uint32_t)key_hi,
        (uint32_t)(key_lo >> 32),
        (uint32_t)key_lo,
    };

    uint32_t first_hi = CARDIN_S508_KEY_BLOCK_1;
    uint32_t first_lo = counter;
    uint32_t second_hi = CARDIN_S508_KEY_BLOCK_2;
    uint32_t second_lo = counter;
    subghz_protocol_cardin_s508_calc_key_block(&first_hi, &first_lo, key);
    subghz_protocol_cardin_s508_calc_key_block(&second_hi, &second_lo, key);

    /* CalcKeyS500 returns first.v0, first.v1, second.v0, second.v1 through
     * four word pointers. Cardin's VB6 adapter writes the transmitter bytes
     * in reverse word/block order: second.v1, second.v0, first.v1, first.v0. */
    *payload_hi = ((uint64_t)second_lo << 32) | second_hi;
    *payload_lo = ((uint64_t)first_lo << 32) | first_hi;
}

/* ------------------------------- decoder -------------------------------- */

static void subghz_protocol_decoder_cardin_s508_reset_accumulators(
    SubGhzProtocolDecoderCardinS508* instance) {
    instance->sync_shift = 0;
    instance->sync_locked = false;
    instance->inverted = false;
    instance->payload_hi = 0;
    instance->payload_lo = 0;
    instance->payload_count = 0;
    instance->rolling = false;
    instance->rolling_counter = 0;
    instance->decoder.decode_data = 0;
    instance->decoder.decode_count_bit = 0;
    instance->manchester_saved_state = ManchesterStateMid1;
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
    free(context);
}

void subghz_protocol_decoder_cardin_s508_reset(void* context) {
    furi_assert(context);
    subghz_protocol_decoder_cardin_s508_reset_accumulators(context);
}

static void subghz_protocol_decoder_cardin_s508_add_bit(
    SubGhzProtocolDecoderCardinS508* instance,
    bool bit) {
    if(!instance->sync_locked) {
        instance->sync_shift =
            (uint16_t)(((instance->sync_shift << 1) | (bit ? 1u : 0u)) & CARDIN_S508_SYNC_MASK);
        if(instance->sync_shift == CARDIN_S508_SYNC) {
            instance->inverted = false;
        } else if(instance->sync_shift == CARDIN_S508_SYNC_INV) {
            instance->inverted = true;
        } else {
            return;
        }

        instance->sync_locked = true;
        instance->payload_hi = 0;
        instance->payload_lo = 0;
        instance->payload_count = 0;
        return;
    }

    instance->payload_hi = (instance->payload_hi << 1) | (instance->payload_lo >> 63);
    instance->payload_lo = (instance->payload_lo << 1) | (bit ? 1u : 0u);
    instance->payload_count++;

    if(instance->payload_count < CARDIN_S508_PAYLOAD_BITS) return;

    instance->generic.data_2 = instance->inverted ? ~instance->payload_hi : instance->payload_hi;
    instance->generic.data = instance->inverted ? ~instance->payload_lo : instance->payload_lo;
    instance->generic.data_count_bit = CARDIN_S508_PAYLOAD_BITS;
    instance->decoder.decode_data = instance->generic.data;
    instance->decoder.decode_count_bit = CARDIN_S508_PAYLOAD_BITS;

    if(instance->base.callback) instance->base.callback(&instance->base, instance->base.context);

    /* A held button repeats the same codeword. Look for the next sync. */
    instance->sync_locked = false;
    instance->sync_shift = 0;
}

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
    return subghz_protocol_blocks_get_hash_data(&instance->decoder, sizeof(uint64_t));
}

SubGhzProtocolStatus subghz_protocol_decoder_cardin_s508_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (uint8_t)(instance->generic.data_2 >> (i * 8));
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

    /* A manually-created rolling file contains the secret 128-bit key and
     * counter. Rebuild the opaque RF payload so Signal Settings can expose
     * the counter editor for that file. Captured/replay files continue to use
     * the opaque Data field below. */
    instance->rolling = false;
    if(!flipper_format_rewind(flipper_format)) return SubGhzProtocolStatusErrorParserOthers;
    uint32_t rolling = 0;
    if(flipper_format_read_uint32(flipper_format, "Rolling", &rolling, 1) && rolling) {
        uint32_t bit_count = 0;
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_uint32(flipper_format, "Bit", &bit_count, 1)) {
            FURI_LOG_E(TAG, "Missing Bit in rolling file");
            return SubGhzProtocolStatusErrorParserBitCount;
        }
        if(bit_count != CARDIN_S508_PAYLOAD_BITS) {
            FURI_LOG_E(TAG, "Wrong number of bits in rolling file");
            return SubGhzProtocolStatusErrorValueBitCount;
        }

        uint8_t key_data[sizeof(uint64_t) * 2] = {0};
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_hex(flipper_format, "Key", key_data, sizeof(key_data))) {
            FURI_LOG_E(TAG, "Missing 128-bit Cardin key");
            return SubGhzProtocolStatusErrorParserKey;
        }

        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_uint32(flipper_format, "Counter", &instance->rolling_counter, 1)) {
            FURI_LOG_E(TAG, "Missing Cardin rolling counter");
            return SubGhzProtocolStatusErrorParserOthers;
        }

        uint64_t key_hi = 0;
        uint64_t key_lo = 0;
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_hi = (key_hi << 8) | key_data[i];
            key_lo = (key_lo << 8) | key_data[sizeof(uint64_t) + i];
        }
        subghz_protocol_cardin_s508_generate_payload(
            key_hi,
            key_lo,
            instance->rolling_counter,
            &instance->generic.data_2,
            &instance->generic.data);
        instance->generic.data_count_bit = CARDIN_S508_PAYLOAD_BITS;
        instance->rolling = true;
        return SubGhzProtocolStatusOk;
    }

    SubGhzProtocolStatus ret = subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_cardin_s508_const.min_count_bit_for_found);
    if(ret != SubGhzProtocolStatusOk) return ret;

    if(!flipper_format_rewind(flipper_format)) return SubGhzProtocolStatusErrorParserOthers;

    uint8_t key_data[sizeof(uint64_t)] = {0};
    if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Missing Data");
        return SubGhzProtocolStatusErrorParserOthers;
    }

    instance->generic.data_2 = 0;
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        instance->generic.data_2 = (instance->generic.data_2 << 8) | key_data[i];
    }
    return ret;
}

void subghz_protocol_decoder_cardin_s508_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderCardinS508* instance = context;

    subghz_block_generic_global.cnt_is_available = instance->rolling;
    if(instance->rolling) {
        subghz_block_generic_global.cnt_length_bit = 32;
        subghz_block_generic_global.current_cnt = instance->rolling_counter;
    } else {
        subghz_block_generic_global.cnt_length_bit = 0;
        subghz_block_generic_global.current_cnt = 0;
    }

    furi_string_cat_printf(
        output,
        "%s\r\n"
        "Rolling payload (opaque)\r\n"
        "Sync:110011110100\r\n"
        "Payload:%08lX%08lX%08lX%08lX\r\n",
        instance->generic.protocol_name,
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)instance->generic.data_2,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data);

    if(instance->rolling) {
        furi_string_cat_printf(output, "Counter:%08lX\r\n", instance->rolling_counter);
    }
}

/* ------------------------------- encoder -------------------------------- */

static bool subghz_protocol_encoder_cardin_s508_push(
    SubGhzProtocolEncoderCardinS508* instance,
    size_t* index,
    bool level,
    uint32_t duration) {
    if(*index > 0 && level_duration_get_level(instance->encoder.upload[*index - 1]) == level) {
        instance->encoder.upload[*index - 1] = level_duration_make(
            level, level_duration_get_duration(instance->encoder.upload[*index - 1]) + duration);
        return true;
    }
    if(*index >= CARDIN_S508_UPLOAD_SIZE) return false;
    instance->encoder.upload[*index] = level_duration_make(level, duration);
    (*index)++;
    return true;
}

/* A wire 0 is low-high, and a wire 1 is high-low. Adjacent equal half-bits
 * are coalesced into a 2*TE pulse by subghz_protocol_encoder_cardin_s508_push(). */
static bool subghz_protocol_encoder_cardin_s508_add_wire_bit(
    SubGhzProtocolEncoderCardinS508* instance,
    size_t* index,
    bool wire_bit) {
    bool first_level = wire_bit;
    return subghz_protocol_encoder_cardin_s508_push(
               instance, index, first_level, CARDIN_S508_TE) &&
           subghz_protocol_encoder_cardin_s508_push(instance, index, !first_level, CARDIN_S508_TE);
}

static bool subghz_protocol_encoder_cardin_s508_get_payload_bit(
    const SubGhzProtocolEncoderCardinS508* instance,
    size_t index) {
    if(index < 64) return (instance->generic.data_2 >> (63 - index)) & 1u;
    return (instance->generic.data >> (127 - index)) & 1u;
}

static bool
    subghz_protocol_encoder_cardin_s508_get_upload(SubGhzProtocolEncoderCardinS508* instance) {
    size_t index = 0;

    /* A long idle establishes a new Manchester frame boundary. */
    if(!subghz_protocol_encoder_cardin_s508_push(instance, &index, true, CARDIN_S508_GAP)) {
        return false;
    }

    /* The capture has a short low separator before the wake-up burst. */
    if(!subghz_protocol_encoder_cardin_s508_push(instance, &index, false, CARDIN_S508_TE)) {
        return false;
    }

    for(size_t i = 0; i < COUNT_OF(cardin_s508_wakeup); i++) {
        int32_t pulse = cardin_s508_wakeup[i];
        bool level = pulse >= 0;
        uint32_t duration = (uint32_t)(level ? pulse : -pulse);
        if(!subghz_protocol_encoder_cardin_s508_push(instance, &index, level, duration)) {
            return false;
        }
    }

    /* Constant high-low Manchester preamble for receiver clock/AGC settling. */
    for(size_t i = 0; i < CARDIN_S508_PREAMBLE_BITS; i++) {
        if(!subghz_protocol_encoder_cardin_s508_add_wire_bit(instance, &index, true)) {
            return false;
        }
    }

    /* Send the fixed sync and opaque payload in the physical polarity observed
     * in the capture. The decoder accepts the resulting inverse sync and
     * canonicalizes the payload before presenting/saving it. */
    for(size_t i = 0; i < 12; i++) {
        bool canonical_bit = (CARDIN_S508_SYNC >> (11 - i)) & 1u;
        if(!subghz_protocol_encoder_cardin_s508_add_wire_bit(instance, &index, canonical_bit)) {
            return false;
        }
    }
    for(size_t i = 0; i < CARDIN_S508_PAYLOAD_BITS; i++) {
        bool canonical_bit = subghz_protocol_encoder_cardin_s508_get_payload_bit(instance, i);
        if(!subghz_protocol_encoder_cardin_s508_add_wire_bit(instance, &index, canonical_bit)) {
            return false;
        }
    }

    if(!subghz_protocol_encoder_cardin_s508_push(instance, &index, true, CARDIN_S508_GAP)) {
        return false;
    }
    instance->encoder.front = 0;
    instance->encoder.size_upload = index;
    return true;
}

void* subghz_protocol_encoder_cardin_s508_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderCardinS508* instance = malloc(sizeof(SubGhzProtocolEncoderCardinS508));
    instance->base.protocol = &subghz_protocol_cardin_s508;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->encoder.repeat = 3;
    instance->encoder.front = 0;
    instance->encoder.size_upload = CARDIN_S508_UPLOAD_SIZE;
    instance->encoder.upload = malloc(CARDIN_S508_UPLOAD_SIZE * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    instance->rolling_key_hi = 0;
    instance->rolling_key_lo = 0;
    instance->rolling_counter = 0;
    return instance;
}

void subghz_protocol_encoder_cardin_s508_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCardinS508* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_cardin_s508_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderCardinS508* instance = context;

    /* Rolling files carry the secret key in a 16-byte Key field. The generic
     * deserializer intentionally only accepts the 8-byte replay format, so
     * detect and parse the rolling extension before calling it. */
    if(!flipper_format_rewind(flipper_format)) return SubGhzProtocolStatusErrorParserOthers;
    uint32_t rolling = 0;
    if(flipper_format_read_uint32(flipper_format, "Rolling", &rolling, 1) && rolling) {
        uint32_t bit_count = 0;
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_uint32(flipper_format, "Bit", &bit_count, 1)) {
            FURI_LOG_E(TAG, "Missing Bit in rolling file");
            return SubGhzProtocolStatusErrorParserBitCount;
        }
        if(bit_count != CARDIN_S508_PAYLOAD_BITS) {
            FURI_LOG_E(TAG, "Wrong number of bits in rolling file");
            return SubGhzProtocolStatusErrorValueBitCount;
        }

        uint8_t key_data[sizeof(uint64_t) * 2] = {0};
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_hex(flipper_format, "Key", key_data, sizeof(key_data))) {
            FURI_LOG_E(TAG, "Missing 128-bit Cardin key");
            return SubGhzProtocolStatusErrorParserKey;
        }

        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_read_uint32(flipper_format, "Counter", &instance->rolling_counter, 1)) {
            FURI_LOG_E(TAG, "Missing Cardin rolling counter");
            return SubGhzProtocolStatusErrorParserOthers;
        }

        /* Cardin always uses the full 32-bit counter. Set the width here as
         * well as in get_string() so an override cannot inherit a stale width
         * from another protocol. */
        subghz_block_generic_global.cnt_length_bit = 32;
        bool counter_overridden =
            subghz_block_generic_global_counter_override_get(&instance->rolling_counter);

        if(!counter_overridden) {
            int32_t counter_step = furi_hal_subghz_get_rolling_counter_mult();
            /* Cardin has a plain 32-bit counter; use the normal next-code
             * step when the global OFEX marker is selected. Unsigned addition
             * intentionally wraps from 0xFFFFFFFF to zero. */
            if(counter_step == -0x7FFFFFFF) counter_step = 1;
            instance->rolling_counter += (uint32_t)counter_step;
        }

        instance->rolling_key_hi = 0;
        instance->rolling_key_lo = 0;
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            instance->rolling_key_hi = (instance->rolling_key_hi << 8) | key_data[i];
            instance->rolling_key_lo = (instance->rolling_key_lo << 8) |
                                       key_data[sizeof(uint64_t) + i];
        }

        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);
        subghz_protocol_cardin_s508_generate_payload(
            instance->rolling_key_hi,
            instance->rolling_key_lo,
            instance->rolling_counter,
            &instance->generic.data_2,
            &instance->generic.data);
        instance->generic.data_count_bit = CARDIN_S508_PAYLOAD_BITS;
        uint8_t payload_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            payload_data[sizeof(uint64_t) - i - 1] =
                (uint8_t)(instance->generic.data_2 >> (i * 8));
        }
        if(!flipper_format_rewind(flipper_format) ||
           !flipper_format_insert_or_update_hex(
               flipper_format, "Data", payload_data, sizeof(payload_data)) ||
           !flipper_format_insert_or_update_uint32(
               flipper_format, "Counter", &instance->rolling_counter, 1)) {
            FURI_LOG_E(TAG, "Unable to update Cardin rolling counter");
            return SubGhzProtocolStatusErrorParserOthers;
        }
    } else {
        SubGhzProtocolStatus ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_cardin_s508_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) return ret;

        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);
        if(!flipper_format_rewind(flipper_format)) return SubGhzProtocolStatusErrorParserOthers;

        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            return SubGhzProtocolStatusErrorParserOthers;
        }
        instance->generic.data_2 = 0;
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = (instance->generic.data_2 << 8) | key_data[i];
        }
    }

    if(!subghz_protocol_encoder_cardin_s508_get_upload(instance)) {
        FURI_LOG_E(TAG, "Unable to generate upload");
        return SubGhzProtocolStatusErrorEncoderGetUpload;
    }
    instance->encoder.is_running = true;
    return SubGhzProtocolStatusOk;
}

void subghz_protocol_encoder_cardin_s508_stop(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCardinS508* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_cardin_s508_yield(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCardinS508* instance = context;
    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];
    if(++instance->encoder.front == instance->encoder.size_upload) {
        if(!subghz_block_generic_global.endless_tx) instance->encoder.repeat--;
        instance->encoder.front = 0;
    }
    return ret;
}
