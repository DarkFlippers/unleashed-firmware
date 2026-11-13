#include "belkorn.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

#define TAG "BelKorn"

static const SubGhzBlockConst subghz_protocol_belkorn_const = {
    .te_short = 481,
    .te_long = 962,
    .te_delta = 180,
    .min_count_bit_for_found = 118,
};

#define BELKORN_GAP_MIN       2500
#define BELKORN_SYNC          0x801ULL
#define BELKORN_MODEL_KORN    0xD
#define BELKORN_MODEL_BELGATE 0x7

#define BELKORN_L24_MASK   0xFFFFFFUL
#define BELKORN_STEP_A     0x810UL /* L24 per counter pair, format A */
#define BELKORN_STEP_B     0x1028UL /* L24 per counter pair, format B */
#define BELKORN_WRAP_A     0x1C282CUL /* L24 per counter byte wrap, format A */
#define BELKORN_WRAP_B     0x80800UL /* L24 per b6 wrap, format B */
#define BELKORN_B3_STEP_A  0x10 /* b3 per counter pair, format A */
#define BELKORN_B34_WRAP_B 0x108 /* (b3,b4) as 16 bit LE per b6 wrap, format B */
#define BELKORN_B6_BIAS    0xBA /* format B: b6 = counter low byte + bias */
#define BELKORN_B4_BASE    0x5A /* format B: b4 at counter 0, protocol wide */
#define BELKORN_L24_AB     0x0748CCUL /* L24 at counter 0: format B minus format A */
#define BELKORN_B3_AB      0x30 /* b3 at counter 0: format A minus format B */
#define BELKORN_L24_ORIGIN 0x0000E0UL /* low byte of L24 at counter 0, protocol wide */

#define BELKORN_CNT_MASK     0xFFFFU
#define BELKORN_FRAME_LEVELS 120 /* 118 bits, one pulse each, plus trailing mark and gap */
#define BELKORN_FRAME_REPEAT 3 /* x the encoder repeat count, to match the remote's ~9 */
#define BELKORN_UPLOAD_SIZE  (BELKORN_FRAME_REPEAT * BELKORN_FRAME_LEVELS)

static const uint8_t subghz_protocol_belkorn_btn_codes[] = {0x01, 0x02, 0x04, 0x20};

typedef enum {
    BelKornDecoderStepReset = 0,
    BelKornDecoderStepData,
} BelKornDecoderStep;

/** Per-remote constants: the two format A fields at counter 0. */
typedef struct {
    uint8_t model;
    uint32_t l24;
    uint8_t b3;
} BelKornOrigin;

struct SubGhzProtocolDecoderBelKorn {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;

    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderBelKorn);

struct SubGhzProtocolEncoderBelKorn {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;

    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderBelKorn);

const SubGhzProtocolDecoder subghz_protocol_belkorn_decoder = {
    .alloc = subghz_protocol_decoder_belkorn_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_belkorn_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_belkorn_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize_data_2,
    .deserialize = subghz_protocol_decoder_belkorn_deserialize,
    .get_string = subghz_protocol_decoder_belkorn_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_belkorn_encoder = {
    .alloc = subghz_protocol_encoder_belkorn_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_belkorn_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_belkorn = {
    .name = SUBGHZ_PROTOCOL_BELKORN_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_belkorn_decoder,
    .encoder = &subghz_protocol_belkorn_encoder,
};

const char* subghz_protocol_belkorn_get_variant_name(uint8_t model) {
    switch(model & 0xF) {
    case BELKORN_MODEL_KORN:
        return "Korn";
    case BELKORN_MODEL_BELGATE:
        return "BelGate";
    default:
        return SUBGHZ_PROTOCOL_BELKORN_NAME;
    }
}

uint8_t subghz_protocol_belkorn_get_model(uint64_t data) {
    return (data >> 48) & 0xF;
}

/* The 118 frame bits are stored MSB first, bits 0..63 in data and 64..117 in data_2. */
static uint32_t belkorn_get(uint64_t data, uint64_t data_2, uint8_t pos, uint8_t len) {
    const uint64_t mask = ((uint64_t)1 << len) - 1;
    return (pos + len <= 64) ? (uint32_t)((data >> (64 - pos - len)) & mask) :
                               (uint32_t)((data_2 >> (118 - pos - len)) & mask);
}

static void belkorn_put(
    uint8_t model,
    uint32_t serial,
    uint32_t l24,
    uint8_t b3,
    uint8_t b4,
    uint8_t b5,
    uint8_t b6,
    uint8_t c,
    uint8_t btn,
    uint64_t* data,
    uint64_t* data_2) {
    *data = (BELKORN_SYNC << 52) | ((uint64_t)(model & 0xF) << 48) | ((uint64_t)serial << 16) |
            (uint64_t)((l24 & 0xFF) << 8) | (uint64_t)((l24 >> 8) & 0xFF);
    *data_2 = ((uint64_t)((l24 >> 16) & 0xFF) << 46) | ((uint64_t)b3 << 38) |
              ((uint64_t)b4 << 30) | ((uint64_t)b5 << 22) | ((uint64_t)b6 << 14) |
              ((uint64_t)c << 6) | (btn & 0x3F);
}

/* Format B's b6 is the counter low byte plus a bias, so it wraps once per lap plus once more
 * inside the lap already in progress. */
static uint16_t subghz_protocol_belkorn_b6_wraps(uint16_t n) {
    return (n >> 8) + (((n & 0xFF) > 0x45) ? 1 : 0);
}

/* Format B's L24 carries a periodic offset that is additive over the set bits of the counter
 * high byte, each bit contributing a fixed pair of terms gated by one bit of the low byte.
 * The rows for bits 0..2 are measured; bits 3..7 need a remote past 2048 presses and follow
 * the same law. */
static uint32_t subghz_protocol_belkorn_jitter(uint8_t lap, uint8_t c) {
    const uint16_t v = (uint16_t)((c >> 1) - 3);
    uint32_t jitter = (uint32_t)lap << 21;
    for(uint8_t i = 0; i < 8; i++) {
        if(!((lap >> i) & 1)) {
            continue;
        }
        if(!((v >> (i + 1)) & 1)) {
            jitter += 257UL << (5 + i);
        }
        if(i && !((v >> (i - 1)) & 1)) {
            jitter += 1UL << (3 + i);
        }
    }
    return jitter;
}

static void subghz_protocol_belkorn_make_frame(
    const BelKornOrigin* origin,
    uint32_t serial,
    uint16_t n,
    uint8_t btn,
    uint64_t* data,
    uint64_t* data_2) {
    const uint8_t c = n & 0xFF;
    const uint8_t lap = n >> 8;
    const uint16_t k = n >> 1;

    if(c & 1) {
        const uint16_t wraps = subghz_protocol_belkorn_b6_wraps(n);
        const uint32_t l24 =
            (origin->l24 + BELKORN_L24_AB + (uint32_t)k * BELKORN_STEP_B -
             (uint32_t)wraps * BELKORN_WRAP_B + subghz_protocol_belkorn_jitter(lap, c)) &
            BELKORN_L24_MASK;
        const uint16_t b34 =
            (uint16_t)((((uint16_t)BELKORN_B4_BASE << 8) | (uint8_t)(origin->b3 - BELKORN_B3_AB)) +
                       wraps * BELKORN_B34_WRAP_B);
        belkorn_put(
            origin->model,
            serial,
            l24,
            b34 & 0xFF,
            b34 >> 8,
            lap,
            (uint8_t)(c + BELKORN_B6_BIAS),
            c,
            btn,
            data,
            data_2);
    } else {
        const uint32_t l24 =
            (origin->l24 + (uint32_t)k * BELKORN_STEP_A + (uint32_t)lap * BELKORN_WRAP_A) &
            BELKORN_L24_MASK;
        belkorn_put(
            origin->model,
            serial,
            l24,
            (uint8_t)(origin->b3 + k * BELKORN_B3_STEP_A),
            c,
            lap,
            lap,
            c,
            btn,
            data,
            data_2);
    }
}

/* Reads the serial, press counter and button out of the stored frame, and solves back the
 * per-remote origin the generator needs. An odd counter low byte means format B, whose origin
 * differs from format A's by two fixed offsets. */
static void subghz_protocol_belkorn_check_remote_controller(
    SubGhzBlockGeneric* instance,
    BelKornOrigin* origin) {
    const uint64_t data = instance->data;
    const uint64_t data_2 = instance->data_2;

    origin->model = belkorn_get(data, data_2, 12, 4);
    instance->serial = belkorn_get(data, data_2, 16, 32);
    instance->btn = belkorn_get(data, data_2, 112, 6);

    const uint32_t l24 = belkorn_get(data, data_2, 48, 8) |
                         (belkorn_get(data, data_2, 56, 8) << 8) |
                         (belkorn_get(data, data_2, 64, 8) << 16);
    const uint8_t b3 = belkorn_get(data, data_2, 72, 8);
    const uint8_t b4 = belkorn_get(data, data_2, 80, 8);
    const uint8_t b5 = belkorn_get(data, data_2, 88, 8);
    const uint8_t b6 = belkorn_get(data, data_2, 96, 8);
    const uint8_t c = belkorn_get(data, data_2, 104, 8);

    const uint16_t n = (uint16_t)(((uint16_t)((c & 1) ? b5 : b6) << 8) | c);
    instance->cnt = n;

    if(c & 1) {
        const uint16_t wraps = subghz_protocol_belkorn_b6_wraps(n);
        origin->l24 =
            (l24 - (uint32_t)(n >> 1) * BELKORN_STEP_B + (uint32_t)wraps * BELKORN_WRAP_B -
             subghz_protocol_belkorn_jitter(n >> 8, c) - BELKORN_L24_AB) &
            BELKORN_L24_MASK;
        origin->b3 = (uint8_t)((((uint16_t)b3 | (uint16_t)b4 << 8) - wraps * BELKORN_B34_WRAP_B) +
                               BELKORN_B3_AB);
    } else {
        origin->l24 =
            (l24 - (uint32_t)(n >> 1) * BELKORN_STEP_A - (uint32_t)(n >> 8) * BELKORN_WRAP_A) &
            BELKORN_L24_MASK;
        origin->b3 = (uint8_t)(b3 - (n >> 1) * BELKORN_B3_STEP_A);
    }

    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(3);
}

// Get custom button code
static uint8_t subghz_protocol_belkorn_get_btn_code(void) {
    const uint8_t original_btn_code = subghz_custom_btn_get_original();
    const uint8_t custom_btn_id = subghz_custom_btn_get();

    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        return original_btn_code;
    }
    uint8_t index = 0;
    while(index < COUNT_OF(subghz_protocol_belkorn_btn_codes) &&
          subghz_protocol_belkorn_btn_codes[index] != original_btn_code) {
        index++;
    }
    return subghz_protocol_belkorn_btn_codes
        [(index + custom_btn_id) % COUNT_OF(subghz_protocol_belkorn_btn_codes)];
}

void* subghz_protocol_encoder_belkorn_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderBelKorn), &subghz_protocol_belkorn, 3, BELKORN_UPLOAD_SIZE);
}

static void subghz_protocol_belkorn_encoder_get_upload(
    SubGhzProtocolEncoderBelKorn* instance,
    size_t* index,
    uint64_t data,
    uint64_t data_2,
    uint32_t gap) {
    size_t index_local = *index;

    // One bit per pulse, on marks and spaces alike: te_short is 0, te_long is 1
    for(uint8_t i = 0; i < subghz_protocol_belkorn_const.min_count_bit_for_found; i++) {
        const bool set = (i < 64) ? bit_read(data, 63 - i) : bit_read(data_2, 117 - i);
        instance->encoder.upload[index_local++] = level_duration_make(
            (i & 1) == 0,
            set ? (uint32_t)subghz_protocol_belkorn_const.te_long :
                  (uint32_t)subghz_protocol_belkorn_const.te_short);
    }
    // Trailing mark, the one pulse the remote sends at either width
    instance->encoder.upload[index_local++] =
        level_duration_make(true, (uint32_t)subghz_protocol_belkorn_const.te_short);
    instance->encoder.upload[index_local++] = level_duration_make(false, gap);

    *index = index_local;
}

static void
    subghz_protocol_belkorn_encoder_prepare_packets(SubGhzProtocolEncoderBelKorn* instance) {
    furi_assert(instance);

    BelKornOrigin origin;
    subghz_protocol_belkorn_check_remote_controller(&instance->generic, &origin);

    // Counter increment
    // check OFEX mode
    if(furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF) {
        // standart counter mode. PULL data from subghz_block_generic_global variables
        if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else {
        // TODO: OFEX mode
        instance->generic.cnt += 1;
    }
    instance->generic.cnt &= BELKORN_CNT_MASK;

    // Generate new key using custom or default button
    instance->generic.btn = subghz_protocol_belkorn_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&instance->generic.btn)) {
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", instance->generic.btn);
    }

    /* One click is one frame at one counter value, which the remote repeats for as long as
     * it transmits. Whether that frame is format A or B falls out of the counter parity. */
    const uint16_t n = (uint16_t)instance->generic.cnt;
    const uint32_t gap = (uint32_t)subghz_protocol_belkorn_const.te_short * 16;
    size_t index = 0;
    uint64_t data = 0;
    uint64_t data_2 = 0;

    subghz_protocol_belkorn_make_frame(
        &origin, instance->generic.serial, n, instance->generic.btn, &data, &data_2);
    for(uint8_t i = 0; i < BELKORN_FRAME_REPEAT; i++) {
        subghz_protocol_belkorn_encoder_get_upload(instance, &index, data, data_2, gap);
    }

    // Saving after a send keeps the counter that was just sent
    instance->generic.data = data;
    instance->generic.data_2 = data_2;

    instance->encoder.size_upload = index;
}

bool subghz_protocol_belkorn_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint8_t model,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderBelKorn* instance = context;
    instance->generic.serial = serial;
    instance->generic.btn = btn;
    instance->generic.cnt = cnt & BELKORN_CNT_MASK;
    instance->generic.data_count_bit = subghz_protocol_belkorn_const.min_count_bit_for_found;

    /* L24 and b3 are a factory origin with no known tie to the serial, and a receiver in learn
     * mode takes whatever it is handed, so a manual key starts from the protocol wide low byte. */
    const BelKornOrigin origin = {
        .model = model,
        .l24 = BELKORN_L24_ORIGIN,
        .b3 = 0,
    };
    subghz_protocol_belkorn_make_frame(
        &origin,
        serial,
        (uint16_t)instance->generic.cnt,
        btn,
        &instance->generic.data,
        &instance->generic.data_2);

    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    res = subghz_protocol_common_append_data_2(res, &instance->generic, flipper_format);

    return res == SubGhzProtocolStatusOk;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_belkorn_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderBelKorn* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk != subghz_block_generic_deserialize_check_count_bit(
                                         &instance->generic,
                                         flipper_format,
                                         subghz_protocol_belkorn_const.min_count_bit_for_found)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            break;
        }

        instance->generic.data_2 = 0;
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }

        subghz_protocol_belkorn_encoder_prepare_packets(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Data");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void* subghz_protocol_decoder_belkorn_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderBelKorn* instance = subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderBelKorn), &subghz_protocol_belkorn);
    instance->decoder.parser_step = BelKornDecoderStepReset;
    return instance;
}

/* Both formats of one press carry the same press counter, so hashing the decoded fields keeps
 * the pair as a single entry in the history instead of two. */
uint8_t subghz_protocol_decoder_belkorn_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderBelKorn* instance = context;
    BelKornOrigin origin;
    subghz_protocol_belkorn_check_remote_controller(&instance->generic, &origin);
    return subghz_protocol_decoder_common_get_hash_data_serial(context);
}

static bool subghz_protocol_belkorn_frame_valid(uint64_t data, uint64_t data_2) {
    const uint8_t btn = data_2 & 0x3F;
    return ((data >> 52) == BELKORN_SYNC) && (btn != 0) && ((btn & (btn - 1)) == 0);
}

void subghz_protocol_decoder_belkorn_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderBelKorn* instance = context;

    switch(instance->decoder.parser_step) {
    case BelKornDecoderStepReset:
        if((!level) && (duration >= BELKORN_GAP_MIN)) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = BelKornDecoderStepData;
        }
        break;
    case BelKornDecoderStepData:
        // Bits alternate mark and space, so a level out of turn means the frame was broken
        if(level != ((instance->decoder.decode_count_bit & 1) == 0)) {
            instance->decoder.parser_step = BelKornDecoderStepReset;
            break;
        }
        if(DURATION_DIFF(duration, subghz_protocol_belkorn_const.te_short) <
           subghz_protocol_belkorn_const.te_delta) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
        } else if(
            DURATION_DIFF(duration, subghz_protocol_belkorn_const.te_long) <
            subghz_protocol_belkorn_const.te_delta) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
        } else {
            instance->decoder.parser_step = BelKornDecoderStepReset;
            break;
        }

        if(instance->decoder.decode_count_bit == 64) {
            instance->generic.data = instance->decoder.decode_data;
            instance->decoder.decode_data = 0;
        } else if(
            instance->decoder.decode_count_bit ==
            subghz_protocol_belkorn_const.min_count_bit_for_found) {
            instance->generic.data_2 = instance->decoder.decode_data;
            instance->generic.data_count_bit = instance->decoder.decode_count_bit;
            instance->decoder.parser_step = BelKornDecoderStepReset;

            if(subghz_protocol_belkorn_frame_valid(
                   instance->generic.data, instance->generic.data_2) &&
               instance->base.callback) {
                instance->base.callback(&instance->base, instance->base.context);
            }
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_belkorn_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderBelKorn* instance = context;

    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_belkorn_const.min_count_bit_for_found);
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

void subghz_protocol_decoder_belkorn_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderBelKorn* instance = context;

    BelKornOrigin origin;
    subghz_protocol_belkorn_check_remote_controller(&instance->generic, &origin);

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 16;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 6;
    //

    furi_string_printf(
        output,
        "%s %db\r\n",
        subghz_protocol_belkorn_get_variant_name(origin.model),
        instance->generic.data_count_bit);

    if((origin.model != BELKORN_MODEL_KORN) && (origin.model != BELKORN_MODEL_BELGATE)) {
        furi_string_cat_printf(output, "Unknown MF:%01X\r\n", origin.model);
    }

    furi_string_cat_printf(
        output,
        "Key:%lX%08lX\r\n"
        "Key2:%lX%08lX\r\n"
        "Sn:%08lX Btn:%02X\r\n"
        "Cnt:%04lX",
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)(instance->generic.data_2 & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
