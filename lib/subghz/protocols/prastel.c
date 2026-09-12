#include "prastel.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

/*
 * Prastel 42 bit rolling code.
 *
 * Shares the CAME line coding (320/640 us, 24 ms header) but the 42 bit payload is
 * scrambled and carries a serial, a button and a 16 bit counter, so it lives in its
 * own dynamic protocol. The 25 bit Prastel and 18 bit Airforce frames stay in came.c
 * where they belong - those are static.
 *
 * Payload layout, 7 bytes taken from (data << 2), MSB first:
 *   p[0..1]  fixed / high nibble of the serial
 *   p[2..3]  serial
 *   p[4]     serial nibble | button index
 *   p[5..6]  counter, little endian, plus a parity bit
 */

#define TAG "SubGhzProtocolPrastel"

#define PRASTEL_42_COUNT_BIT 42

static const SubGhzBlockConst subghz_protocol_prastel_const = {
    .te_short = 320,
    .te_long = 640,
    .te_delta = 150,
    .min_count_bit_for_found = PRASTEL_42_COUNT_BIT,
};

struct SubGhzProtocolDecoderPrastel {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderPrastel);

struct SubGhzProtocolEncoderPrastel {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderPrastel);

typedef enum {
    PrastelDecoderStepReset = 0,
    PrastelDecoderStepFoundStartBit,
    PrastelDecoderStepSaveDuration,
    PrastelDecoderStepCheckDuration,
} PrastelDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_prastel_decoder = {
    .alloc = subghz_protocol_decoder_prastel_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_prastel_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize,
    .deserialize = subghz_protocol_decoder_prastel_deserialize,
    .get_string = subghz_protocol_decoder_prastel_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_prastel_encoder = {
    .alloc = subghz_protocol_encoder_prastel_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_prastel_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_prastel = {
    .name = SUBGHZ_PROTOCOL_PRASTEL_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_prastel_decoder,
    .encoder = &subghz_protocol_prastel_encoder,
};

/* ------------------------------------------------------------------------- */
/* Scrambling                                                                 */
/* ------------------------------------------------------------------------- */

static uint8_t subghz_protocol_prastel_reverse_byte(uint8_t v) {
    v = (uint8_t)((v >> 4) | (v << 4));
    v = (uint8_t)(((v & 0xCC) >> 2) | ((v & 0x33) << 2));
    v = (uint8_t)(((v & 0xAA) >> 1) | ((v & 0x55) << 1));
    return v;
}

static uint8_t subghz_protocol_prastel_parity(uint8_t v) {
    v ^= (uint8_t)(v >> 4);
    return (uint8_t)((0x6996u >> (v & 0x0F)) & 1u);
}

/**
 * Undo the Prastel 42 bit scrambling in place.
 * @param p 7 byte buffer holding the 44 bit payload, MSB first
 */
static void subghz_protocol_prastel_unscramble(uint8_t p[7]) {
    // three passes of a 40 bit right shift over p[2..6], carrying the bit that falls
    // out of p[6] into the top of p[2] on the next pass
    uint8_t carry = 0;
    for(uint8_t round = 0; round < 3; round++) {
        for(uint8_t i = 2; i <= 6; i++) {
            uint8_t cur = p[i];
            p[i] = (uint8_t)((carry << 7) | (cur >> 1));
            carry = cur & 1;
        }
    }

    p[0] = subghz_protocol_prastel_reverse_byte(p[0]);
    uint8_t t2 = p[2] & 0xDF;
    p[1] = subghz_protocol_prastel_reverse_byte(p[1]);
    p[3] = subghz_protocol_prastel_reverse_byte(p[3]);
    p[4] = subghz_protocol_prastel_reverse_byte(p[4]);
    uint8_t swapped = (uint8_t)((t2 << 4) | (t2 >> 4));
    p[2] = subghz_protocol_prastel_reverse_byte((uint8_t)~swapped);

    uint8_t p6 = p[6];
    p[5] = subghz_protocol_prastel_reverse_byte(p[5]);
    if(p6 & 1) p[5] ^= 1;

    // counter: p[5] selects how many times the LCG x = x*21+1 is iterated over p[6]
    uint8_t t = (uint8_t)(((uint8_t)(p[5] * 21) + 1) * 21);
    uint8_t x = subghz_protocol_prastel_reverse_byte(p6);
    uint8_t n = (uint8_t)(t + 1);
    if(n != 0) {
        uint8_t limit = (uint8_t)~t;
        uint8_t i = 0;
        do {
            x = (uint8_t)(x * 21 + 1);
            i++;
        } while(limit > i);
    }
    p[5] = n ^ 0xA7;
    p[6] = x ^ 0x69;
}

/**
 * Exact inverse of subghz_protocol_prastel_unscramble().
 * The counter LCG x = x*21 + 1 has period 256, so both directions are done by
 * iterating it forwards: n extra steps on the counter byte and 254 on the length
 * byte bring each of them back round.
 * @param p 7 byte buffer, modified in place
 */
static void subghz_protocol_prastel_scramble(uint8_t p[7]) {
    uint8_t n = (uint8_t)(p[5] ^ 0xA7);
    uint8_t x = (uint8_t)(p[6] ^ 0x69);

    for(uint8_t i = 0; n > i; i++) {
        x = (uint8_t)(x * 21 + 1);
    }
    for(uint8_t i = 0; i < 254; i++) {
        n = (uint8_t)(n * 21 + 1);
    }

    const bool flip = (x & 0x80) != 0;

    p[0] = subghz_protocol_prastel_reverse_byte(p[0]);
    p[1] = subghz_protocol_prastel_reverse_byte(p[1]);
    p[3] = subghz_protocol_prastel_reverse_byte(p[3]);
    p[4] = subghz_protocol_prastel_reverse_byte(p[4]);

    if(flip) n ^= 1;
    if(p[2] == 0) p[2] = 0xF0;
    uint8_t w = (uint8_t)~subghz_protocol_prastel_reverse_byte(p[2]);
    p[2] = (uint8_t)(((w << 4) & 0xD0) | ((w >> 4) & 0x0F));
    p[5] = subghz_protocol_prastel_reverse_byte(n);
    p[6] = subghz_protocol_prastel_reverse_byte(x);

    const uint8_t parity_5 = subghz_protocol_prastel_parity(p[5]);
    const uint8_t parity_6 = subghz_protocol_prastel_parity(p[6]);

    uint8_t carry = 0;
    for(uint8_t round = 0; round < 3; round++) {
        for(int8_t i = 6; i >= 2; i--) {
            uint8_t cur = p[i];
            p[i] = (uint8_t)((cur << 1) | carry);
            carry = (uint8_t)(cur >> 7);
        }
    }

    // bit 2 of the last byte is a parity check over the two counter bytes
    if(parity_5 == parity_6) {
        p[6] &= (uint8_t)~0x04;
    } else {
        p[6] |= 0x04;
    }
}

static void subghz_protocol_prastel_unpack(uint64_t data, uint8_t p[7]) {
    uint64_t payload = ((uint64_t)((data >> 30) & 0xFFF) << 32) | (uint32_t)(data << 2);
    for(uint8_t i = 0; i < 7; i++) {
        p[i] = (uint8_t)(payload >> (0x30u - 8u * i));
    }
}

static uint64_t subghz_protocol_prastel_pack(const uint8_t p[7]) {
    uint64_t payload = 0;
    for(uint8_t i = 0; i < 7; i++) {
        payload |= (uint64_t)p[i] << (0x30u - 8u * i);
    }
    return (payload >> 2) & 0x3FFFFFFFFFFULL;
}

/**
 * Analysis of received data.
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_prastel_remote_controller(SubGhzBlockGeneric* instance) {
    static const uint8_t btn_map[8] = {0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x01};

    uint8_t p[7];
    subghz_protocol_prastel_unpack(instance->data, p);
    subghz_protocol_prastel_unscramble(p);

    instance->serial = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 8) | p[3] |
                       (((uint32_t)p[4] << 12) & 0xF0000);
    uint8_t idx = (uint8_t)((p[4] & 0x0F) - 7);
    instance->btn = (idx <= 7) ? btn_map[idx] : 0;
    instance->cnt = (uint16_t)(p[5] | (p[6] << 8));

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original((p[4] & 0x0F) | 0x10);
    }
    subghz_custom_btn_set_max(2);
}

/**
 * Rebuild a key with a new button and counter.
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param btn Raw low nibble of the button field
 * @param cnt New counter value
 */
static void
    subghz_protocol_prastel_gen_data(SubGhzBlockGeneric* instance, uint8_t btn, uint16_t cnt) {
    uint8_t p[7];
    subghz_protocol_prastel_unpack(instance->data, p);
    subghz_protocol_prastel_unscramble(p);

    p[4] = (uint8_t)((p[4] & 0xF0) | (btn & 0x0F));
    p[5] = (uint8_t)cnt;
    p[6] = (uint8_t)(cnt >> 8);

    subghz_protocol_prastel_scramble(p);

    instance->data = subghz_protocol_prastel_pack(p);
    instance->cnt = cnt;
}

/**
 * Defines the button value for the current btn_id.
 * Prastel only has two positions, so any custom position flips to the other one.
 * @return Raw low nibble of the button field
 */
static uint8_t subghz_protocol_prastel_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original() & 0x0F;
    if(original_btn_code == 0) return 0;

    if(custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) return original_btn_code;
    return (uint8_t)((original_btn_code == 0x0B) ? 0x0C : 0x0B);
}

/* ------------------------------------------------------------------------- */
/* Encoder                                                                    */
/* ------------------------------------------------------------------------- */

void* subghz_protocol_encoder_prastel_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderPrastel), &subghz_protocol_prastel, 3, 128);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderPrastel instance
 * @return true On success
 */
static bool subghz_protocol_encoder_prastel_get_upload(SubGhzProtocolEncoderPrastel* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    // Header, same 24320 us as CAME 24 bit
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_prastel_const.te_short * 76);
    // Start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_prastel_const.te_short);
    // Key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_prastel_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_prastel_const.te_short);
        } else {
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_prastel_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_prastel_const.te_long);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_prastel_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderPrastel* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_prastel_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_prastel_remote_controller(&instance->generic);

        uint8_t btn = subghz_protocol_prastel_get_btn_code();
        if(subghz_block_generic_global_button_override_get(&btn)) {
            FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", btn);
            btn &= 0x0F;
        }

        if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
            int32_t mult = furi_hal_subghz_get_rolling_counter_mult();
            if(mult == -0x7FFFFFFF) mult = 1;
            if((instance->generic.cnt + mult) > 0xFFFF) {
                instance->generic.cnt = 0;
            } else {
                instance->generic.cnt += mult;
            }
        }

        subghz_protocol_prastel_gen_data(&instance->generic, btn, (uint16_t)instance->generic.cnt);

        if(!subghz_protocol_encoder_prastel_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            ret = SubGhzProtocolStatusErrorParserKey;
            break;
        }

        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

/* ------------------------------------------------------------------------- */
/* Decoder                                                                    */
/* ------------------------------------------------------------------------- */

void* subghz_protocol_decoder_prastel_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderPrastel), &subghz_protocol_prastel);
}

void subghz_protocol_decoder_prastel_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPrastel* instance = context;
    switch(instance->decoder.parser_step) {
    case PrastelDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_prastel_const.te_short * 56) <
                        subghz_protocol_prastel_const.te_delta * 63)) {
            instance->decoder.parser_step = PrastelDecoderStepFoundStartBit;
        }
        break;
    case PrastelDecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_prastel_const.te_short) <
            subghz_protocol_prastel_const.te_delta) {
            instance->decoder.parser_step = PrastelDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = PrastelDecoderStepReset;
        }
        break;
    case PrastelDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_prastel_const.te_short * 4)) {
                instance->decoder.parser_step = PrastelDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_prastel_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = PrastelDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = PrastelDecoderStepReset;
        }
        break;
    case PrastelDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_prastel_const.te_short) <
                subghz_protocol_prastel_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_prastel_const.te_long) <
                subghz_protocol_prastel_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = PrastelDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_prastel_const.te_long) <
                 subghz_protocol_prastel_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_prastel_const.te_short) <
                 subghz_protocol_prastel_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = PrastelDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = PrastelDecoderStepReset;
        } else {
            instance->decoder.parser_step = PrastelDecoderStepReset;
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_prastel_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPrastel* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_prastel_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_prastel_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderPrastel* instance = context;

    subghz_protocol_prastel_remote_controller(&instance->generic);

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 16;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 4;

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%011llX\r\n"
        "Sn:%08lX\r\n"
        "Btn:%01X\r\n"
        "Cnt:%04lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.data & 0x3FFFFFFFFFFULL,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
