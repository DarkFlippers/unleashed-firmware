#include "telcoma_edge.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolTelcomaEdge"

/* Upload buffer capacity (LevelDuration entries). Worst case is a 33-bit
 * channel frame: 33*2 half-bits + 1 stop + 1 guard gap = 68, well under this. */
#define TELCOMA_EDGE_UPLOAD_SIZE 128

/*
 * Timing + framing constants, taken from real captures and validated in a
 * Python reference decoder before this port (9/9 frames -> 0xFF309FC0).
 * te_delta is deliberately wide because the half-bit is long and the cheap
 * remote's timing jitters a fair bit. Tune against your own .sub if needed.
 */
static const SubGhzBlockConst subghz_protocol_telcoma_edge_const = {
    .te_short = 1270,
    .te_long = 2540,
    .te_delta = 520,
    .min_count_bit_for_found = 32,
};

struct SubGhzProtocolDecoderTelcomaEdge {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    bool half_pending; /* a half-bit sample is buffered, awaiting its pair */
    bool half_level; /* level of the buffered half-bit */
};

struct SubGhzProtocolEncoderTelcomaEdge {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

const SubGhzProtocolDecoder subghz_protocol_telcoma_edge_decoder = {
    .alloc = subghz_protocol_decoder_telcoma_edge_alloc,
    .free = subghz_protocol_decoder_telcoma_edge_free,
    .feed = subghz_protocol_decoder_telcoma_edge_feed,
    .reset = subghz_protocol_decoder_telcoma_edge_reset,
    .get_hash_data = subghz_protocol_decoder_telcoma_edge_get_hash_data,
    .serialize = subghz_protocol_decoder_telcoma_edge_serialize,
    .deserialize = subghz_protocol_decoder_telcoma_edge_deserialize,
    .get_string = subghz_protocol_decoder_telcoma_edge_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_telcoma_edge_encoder = {
    .alloc = subghz_protocol_encoder_telcoma_edge_alloc,
    .free = subghz_protocol_encoder_telcoma_edge_free,
    .deserialize = subghz_protocol_encoder_telcoma_edge_deserialize,
    .stop = subghz_protocol_encoder_telcoma_edge_stop,
    .yield = subghz_protocol_encoder_telcoma_edge_yield,
};

const SubGhzProtocol subghz_protocol_telcoma_edge = {
    .name = SUBGHZ_PROTOCOL_TELCOMA_EDGE_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,
    .decoder = &subghz_protocol_telcoma_edge_decoder,
    .encoder = &subghz_protocol_telcoma_edge_encoder,
};

/* ------------------------------- decoder -------------------------------- */

void* subghz_protocol_decoder_telcoma_edge_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderTelcomaEdge* instance = malloc(sizeof(SubGhzProtocolDecoderTelcomaEdge));
    instance->base.protocol = &subghz_protocol_telcoma_edge;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->half_pending = false;
    return instance;
}

void subghz_protocol_decoder_telcoma_edge_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    free(instance);
}

void subghz_protocol_decoder_telcoma_edge_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    instance->decoder.decode_data = 0;
    instance->decoder.decode_count_bit = 0;
    instance->half_pending = false;
}

/*
 * Manchester decode without the firmware's manchester_advance state machine:
 * expand each pulse into half-bit samples (TE -> one sample, 2*TE -> two
 * samples of the same level) and pair consecutive samples. A high->low pair
 * is a 1, low->high is a 0; a same-level pair is a phase violation and is
 * dropped. Pairing re-aligns at every inter-frame gap. This mirrors the Python
 * reference decoder that was validated against real captures (-> 0xFF309FC0),
 * which the firmware's manchester_advance did NOT reproduce here.
 *
 * POLARITY: high->low = 1 yields the validated 0xFF309FC0. If a build ever
 * decodes the bitwise complement (0x00CF603F), swap the two bit assignments
 * below.
 */
static void subghz_protocol_decoder_telcoma_edge_add_half(
    SubGhzProtocolDecoderTelcomaEdge* instance,
    bool level) {
    if(!instance->half_pending) {
        instance->half_pending = true;
        instance->half_level = level;
        return;
    }
    instance->half_pending = false;
    if(instance->half_level && !level) {
        subghz_protocol_blocks_add_bit(&instance->decoder, 1); /* high -> low = 1 */
    } else if(!instance->half_level && level) {
        subghz_protocol_blocks_add_bit(&instance->decoder, 0); /* low -> high = 0 */
    }
    /* same-level pair: phase violation, emit no bit (matches the reference) */
}

void subghz_protocol_decoder_telcoma_edge_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;

    if(DURATION_DIFF(duration, subghz_protocol_telcoma_edge_const.te_short) <
       subghz_protocol_telcoma_edge_const.te_delta) {
        subghz_protocol_decoder_telcoma_edge_add_half(instance, level);
    } else if(
        DURATION_DIFF(duration, subghz_protocol_telcoma_edge_const.te_long) <
        subghz_protocol_telcoma_edge_const.te_delta) {
        subghz_protocol_decoder_telcoma_edge_add_half(instance, level);
        subghz_protocol_decoder_telcoma_edge_add_half(instance, level);
    } else {
        /* End of a burst (inter-frame gap): publish a complete frame.
         *
         * Channel framing (derived from clean multi-channel captures): the gate
         * channel decodes as 32 Manchester bits; every other channel carries a
         * one-hot marker that adds one bit, so it decodes as 33 bits. Normalise
         * both to a canonical 32-bit key (drop the extra trailing bit on 33-bit
         * frames) so data/count stay uniform; the channel then lives in the low
         * bits. The encoder reverses this exactly (see get_upload). */
        uint32_t count = instance->decoder.decode_count_bit;
        if(count == 32 || count == 33) {
            uint32_t key = (count == 33) ? (uint32_t)(instance->decoder.decode_data >> 1) :
                                           (uint32_t)instance->decoder.decode_data;
            /* Reject noise/partial frames: a valid frame starts with 0xFF. */
            if((key >> 24) == 0xFF) {
                instance->generic.data = key;
                instance->generic.data_count_bit =
                    subghz_protocol_telcoma_edge_const.min_count_bit_for_found;
                if(instance->base.callback) {
                    instance->base.callback(&instance->base, instance->base.context);
                }
            }
        }
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        instance->half_pending = false;
    }
}

uint8_t subghz_protocol_decoder_telcoma_edge_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_telcoma_edge_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_telcoma_edge_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_telcoma_edge_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_telcoma_edge_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderTelcomaEdge* instance = context;
    uint32_t data = (uint32_t)(instance->generic.data & 0xFFFFFFFF);
    uint32_t payload = data & 0xFFFFFF; /* 24-bit, 0xFF preamble stripped */
    /* Canonical 32-bit key layout: payload[23:3] = per-remote serial/address,
     * payload[2:0] = one-hot channel select (gate = 0, others = 0x1/0x2/0x4).
     * The channel is a position-encoded one-hot marker on the wire that the
     * decoder folds into these low bits (and the encoder restores). */
    uint32_t serial = (payload >> 3) & 0x1FFFFF;
    uint8_t channel = payload & 0x07;
    furi_string_cat_printf(
        output,
        "Telcoma/Cardin\nEDGE %db\r\n"
        "Key:0x%08lX\r\n"
        "Serial:0x%05lX\r\n"
        "Ch:0x%01X\r\n",
        instance->generic.data_count_bit,
        (unsigned long)data,
        (unsigned long)serial,
        channel);
}

/* ------------------------------- encoder -------------------------------- */

void* subghz_protocol_encoder_telcoma_edge_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderTelcomaEdge* instance = malloc(sizeof(SubGhzProtocolEncoderTelcomaEdge));
    instance->base.protocol = &subghz_protocol_telcoma_edge;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->encoder.repeat = 6;
    instance->encoder.size_upload = TELCOMA_EDGE_UPLOAD_SIZE;
    instance->encoder.upload = malloc(TELCOMA_EDGE_UPLOAD_SIZE * sizeof(LevelDuration));
    instance->encoder.front = 0;
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_telcoma_edge_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderTelcomaEdge* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static bool
    subghz_protocol_encoder_telcoma_edge_get_upload(SubGhzProtocolEncoderTelcomaEdge* instance) {
    furi_assert(instance);
    size_t index = 0;
    uint32_t te = subghz_protocol_telcoma_edge_const.te_short;

    /* Channel-aware reconstruction (validated to reproduce every captured
     * channel pulse-for-pulse):
     *   - gate (channel field 0): 32 Manchester bits + 1 HIGH stop half-bit.
     *   - any other channel:      33 Manchester bits (key << 1), NO stop.
     * The << 1 restores the on-wire one-hot channel marker that the decoder
     * folded into the canonical key's low bits. */
    uint32_t key = (uint32_t)(instance->generic.data & 0xFFFFFFFF);
    uint8_t channel = key & 0x07;
    uint64_t value;
    uint8_t nbits;
    bool add_stop;
    if(channel == 0) {
        value = key;
        nbits = 32;
        add_stop = true;
    } else {
        value = (uint64_t)key << 1;
        nbits = 33;
        add_stop = false;
    }

    /* Worst case: 2 half-bits per bit + 1 stop half-bit + 1 guard gap.
     * Coalescing (below) only reduces this, so this is a safe upper bound. */
    if((size_t)(nbits * 2 + 2) > TELCOMA_EDGE_UPLOAD_SIZE) {
        FURI_LOG_E(TAG, "Upload buffer too small");
        return false;
    }

    /* Emit a half-bit at the given level, COALESCING with the previous entry
     * when the level matches. Manchester naturally produces 2*TE runs at
     * same-bit boundaries; merging them reproduces the exact waveform that
     * was verified on hardware (a strictly level-alternating upload). */
#define TELCOMA_EDGE_PUSH(lvl)                                                                    \
    do {                                                                                          \
        if(index > 0 && level_duration_get_level(instance->encoder.upload[index - 1]) == (lvl)) { \
            instance->encoder.upload[index - 1] = level_duration_make(                            \
                (lvl), level_duration_get_duration(instance->encoder.upload[index - 1]) + te);    \
        } else {                                                                                  \
            instance->encoder.upload[index++] = level_duration_make((lvl), te);                   \
        }                                                                                         \
    } while(0)

    for(int i = nbits - 1; i >= 0; i--) {
        uint8_t bit = (value >> i) & 1;
        if(bit) {
            /* 1 -> high, then low (matches the validated decoder convention) */
            TELCOMA_EDGE_PUSH(true);
            TELCOMA_EDGE_PUSH(false);
        } else {
            /* 0 -> low, then high */
            TELCOMA_EDGE_PUSH(false);
            TELCOMA_EDGE_PUSH(true);
        }
    }
    if(add_stop) {
        /* Trailing HIGH stop half-bit — only the gate channel carries it; it
         * completes the gate's 65-half-bit frame and is REQUIRED there (a frame
         * without it was rejected by the gate, verified on hardware). */
        TELCOMA_EDGE_PUSH(true);
    }
#undef TELCOMA_EDGE_PUSH

    /* inter-frame guard (~4 * TE low), matches the gap seen between bursts */
    instance->encoder.upload[index++] = level_duration_make(false, te * 4);

    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_telcoma_edge_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderTelcomaEdge* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_telcoma_edge_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) break;

        if(!subghz_protocol_encoder_telcoma_edge_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.front = 0;
        instance->encoder.is_running = true;
    } while(false);
    return ret;
}

void subghz_protocol_encoder_telcoma_edge_stop(void* context) {
    SubGhzProtocolEncoderTelcomaEdge* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_telcoma_edge_yield(void* context) {
    SubGhzProtocolEncoderTelcomaEdge* instance = context;

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
