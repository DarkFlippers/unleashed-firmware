#include "ditec_gol4.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolDitecGOL4"

#define GOL4_RAW_BYTES 7

static const SubGhzBlockConst subghz_protocol_ditec_gol4_const = {
    .te_short = 400,
    .te_long = 1100,
    .te_delta = 200,
    .min_count_bit_for_found = 54,
};

struct SubGhzProtocolDecoderDitecGOL4 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderDitecGOL4 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    DitecGOL4DecoderStepReset = 0,
    DitecGOL4DecoderStepStartBit,
    DitecGOL4DecoderStepSaveDuration,
    DitecGOL4DecoderStepCheckDuration,
} DitecGOL4DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_ditec_gol4_decoder = {
    .alloc = subghz_protocol_decoder_ditec_gol4_alloc,
    .free = subghz_protocol_decoder_ditec_gol4_free,

    .feed = subghz_protocol_decoder_ditec_gol4_feed,
    .reset = subghz_protocol_decoder_ditec_gol4_reset,

    .get_hash_data = subghz_protocol_decoder_ditec_gol4_get_hash_data,
    .serialize = subghz_protocol_decoder_ditec_gol4_serialize,
    .deserialize = subghz_protocol_decoder_ditec_gol4_deserialize,
    .get_string = subghz_protocol_decoder_ditec_gol4_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_ditec_gol4_encoder = {
    .alloc = subghz_protocol_encoder_ditec_gol4_alloc,
    .free = subghz_protocol_encoder_ditec_gol4_free,

    .deserialize = subghz_protocol_encoder_ditec_gol4_deserialize,
    .stop = subghz_protocol_encoder_ditec_gol4_stop,
    .yield = subghz_protocol_encoder_ditec_gol4_yield,
};

const SubGhzProtocol subghz_protocol_ditec_gol4 = {
    .name = SUBGHZ_PROTOCOL_DITEC_GOL4_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_ditec_gol4_decoder,
    .encoder = &subghz_protocol_ditec_gol4_encoder,
};

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x1 | 0x2 | 0x4 | 0x8 | 0x0 PROG
 * @return Button code
 */
static uint8_t subghz_protocol_ditec_gol4_get_btn_code(void);

static uint8_t gol4_bit_reverse(uint8_t b) {
    b &= 0xFF;
    uint8_t result = 0;
    for(uint8_t i = 0; i < 8; i++) {
        result = (uint8_t)((result << 1) | (b & 1));
        b >>= 1;
    }
    return result;
}

static uint8_t gol4_bit_parity(uint8_t b) {
    uint8_t p = 0;
    for(uint8_t i = 0; i < 8; i++) {
        if((b >> i) & 1u) p ^= 1u;
    }
    return p;
}

static uint8_t gol4_lcg_step(uint8_t seed, uint8_t steps) {
    uint8_t x = seed & 0xFF;
    steps &= 0xFF;
    for(uint8_t i = 0; i < steps; i++) {
        x = (uint8_t)((21 * x + 1) & 0xFF);
    }
    return x;
}

static uint8_t gol4_lcg_inverse(uint8_t target, uint8_t steps) {
    steps &= 0xFF;
    if(steps == 0) return target & 0xFF;
    return gol4_lcg_step(target, (uint8_t)(256 - steps));
}

static void gol4_decode_rotate_and_bitrev(uint8_t* raw) {
    uint8_t carry = 0;
    for(uint8_t r = 0; r < 3; r++) {
        for(uint8_t i = 2; i < 7; i++) {
            uint8_t new_carry = raw[i] & 1;
            raw[i] = (uint8_t)(((raw[i] >> 1) | (carry << 7)) & 0xFF);
            carry = new_carry;
        }
    }

    raw[0] = gol4_bit_reverse(raw[0]);
    raw[1] = gol4_bit_reverse(raw[1]);
    raw[3] = gol4_bit_reverse(raw[3]);
    raw[4] = gol4_bit_reverse(raw[4]);

    uint8_t b2 = raw[2] & 0xDF;
    b2 = (uint8_t)(((b2 << 4) | (b2 >> 4)) & 0xFF);
    b2 = (uint8_t)((~b2) & 0xFF);
    raw[2] = gol4_bit_reverse(b2);

    raw[5] = gol4_bit_reverse(raw[5]);
    raw[6] = gol4_bit_reverse(raw[6]);
}

static bool gol4_decode_lcg_xor(uint8_t* raw) {
    if(raw[6] & 0x80) raw[5] ^= 1;

    uint8_t out5 = gol4_lcg_inverse(raw[5], 0xFE);
    raw[5] = out5;

    uint8_t out6 = gol4_lcg_inverse(raw[6], raw[5]);
    raw[6] = out6;

    raw[5] ^= 0xA7;
    raw[6] ^= 0x69;
    return true;
}

static bool gol4_rolling_decode(uint8_t* raw) {
    gol4_decode_rotate_and_bitrev(raw);
    return gol4_decode_lcg_xor(raw);
}

static bool gol4_encode_lcg_xor(uint8_t* raw) {
    uint8_t dec5 = (uint8_t)(raw[5] ^ 0xA7);
    uint8_t dec6 = (uint8_t)(raw[6] ^ 0x69);

    uint8_t enc6 = gol4_lcg_step(dec6, dec5);
    uint8_t enc5 = gol4_lcg_step(dec5, 0xFE);

    if(enc6 & 0x80) enc5 ^= 1;

    raw[5] = enc5;
    raw[6] = enc6;
    return true;
}

static void gol4_encode_bitrev_and_rotate(uint8_t* raw) {
    raw[0] = gol4_bit_reverse(raw[0]);
    raw[1] = gol4_bit_reverse(raw[1]);
    raw[3] = gol4_bit_reverse(raw[3]);
    raw[4] = gol4_bit_reverse(raw[4]);

    if(raw[2] == 0x0) {
        raw[2] = 0xF0;
    }
    uint8_t b2 = gol4_bit_reverse(raw[2]);
    b2 = (uint8_t)(~b2);
    b2 = (uint8_t)(((b2 << 4) | (b2 >> 4)) & 0xFF);
    b2 &= 0xDF;
    raw[2] = b2;

    raw[5] = gol4_bit_reverse(raw[5]);
    raw[6] = gol4_bit_reverse(raw[6]);

    uint8_t p5 = gol4_bit_parity(raw[5]);
    uint8_t p6 = gol4_bit_parity(raw[6]);

    uint8_t carry = 0;
    for(uint8_t r = 0; r < 3; r++) {
        for(int8_t i = 6; i >= 2; i--) {
            uint8_t new_carry = (uint8_t)((raw[i] >> 7) & 1);
            raw[i] = (uint8_t)(((raw[i] << 1) | carry) & 0xFF);
            carry = new_carry;
        }
    }

    raw[6] = (p5 == p6) ? (uint8_t)(raw[6] & 0xFBu) : (uint8_t)(raw[6] | 0x04u);
}

static bool gol4_rolling_encode(uint8_t* raw) {
    if(!raw) return false;
    if(!gol4_encode_lcg_xor(raw)) return false;
    gol4_encode_bitrev_and_rotate(raw);
    return true;
}

static void bits_to_raw(const uint8_t* bits, uint8_t* raw) {
    memset(raw, 0, GOL4_RAW_BYTES);
    for(uint8_t i = 0; i < subghz_protocol_ditec_gol4_const.min_count_bit_for_found; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = 7 - (i % 8);
        if(bits[i]) raw[byte_idx] |= (1 << bit_idx);
    }
}

static void raw_to_bits(const uint8_t* raw, uint8_t* bits) {
    for(uint8_t i = 0; i < subghz_protocol_ditec_gol4_const.min_count_bit_for_found; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = 7 - (i % 8);
        bits[i] = (uint8_t)((raw[byte_idx] >> bit_idx) & 1);
    }
}

static uint64_t bits_to_data(const uint8_t* bits) {
    uint64_t data = 0;
    for(uint8_t i = 0; i < subghz_protocol_ditec_gol4_const.min_count_bit_for_found; i++) {
        data = (data << 1) | (uint64_t)(bits[i] & 1);
    }
    return data;
}

static uint32_t serial_to_display(const uint8_t* s) {
    if(!s) return 0;
    return (uint32_t)((s[0] << 24) | (s[4] << 16) | (s[1] << 8) | s[3]);
}

void* subghz_protocol_encoder_ditec_gol4_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderDitecGOL4* instance = malloc(sizeof(SubGhzProtocolEncoderDitecGOL4));

    instance->base.protocol = &subghz_protocol_ditec_gol4;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 4;
    instance->encoder.size_upload = 128; // 110 actual
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_ditec_gol4_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderDitecGOL4* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 */
static void
    subghz_protocol_encoder_ditec_gol4_get_upload(SubGhzProtocolEncoderDitecGOL4* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP between repeats
    //Send gap before data
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_ditec_gol4_const.te_long * 22);
    // Start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_ditec_gol4_const.te_short * 2);

    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_ditec_gol4_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_ditec_gol4_const.te_long);
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_ditec_gol4_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_ditec_gol4_const.te_short);
        }
    }

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_ditec_gol4_decode_key(SubGhzBlockGeneric* instance) {
    // Ditec GOL4 Decoder
    // 2025 - 2026.02 - @xMasterX (MMX) & @zero-mega
    //
    // RAW Samples
    // 0xCCB2F83208122 - btn 1 = 0011001100101100 101111 100000110010000 01000000100100010
    //
    // Programming mode:
    // 0xCCB1F832103B9 - btn 0 = 0011001100101100 011111 100000110010000 10000001110111001
    // Regular buttons:
    // 0xCCB2F8320ED66 - btn 1 = 0011001100101100 101111 100000110010000 01110110101100110
    // 0xCCB37832104A6 - btn 2 = 0011001100101100 110111 100000110010000 10000010010100110
    // 0xCCB3B8320DB4E - btn 4 = 0011001100101100 111011 100000110010000 01101101101001110
    // 0xCCB3D8320E855 - btn 8 = 0011001100101100 111101 100000110010000 01110100001010101
    //
    // Regular buttons:
    // Decoded array: CC 34 71 83 09 F8 C1
    // Decoded array: CC 34 71 83 09 F9 C1
    // Decoded array: CC 34 72 83 09 FA C1
    // Decoded array: CC 34 74 83 09 FB C1
    // Decoded array: CC 34 78 83 09 FC C1
    // Programming mode
    // Decoded array: CC 34 F0 83 09 FD C1
    // Decoded array: CC 34 F0 83 09 FE C1
    //
    uint8_t bits[subghz_protocol_ditec_gol4_const.min_count_bit_for_found];
    uint64_t data = instance->data;
    for(int i = subghz_protocol_ditec_gol4_const.min_count_bit_for_found - 1; i >= 0; i--) {
        bits[i] = (uint8_t)(data & 1);
        data >>= 1;
    }
    uint8_t decrypted[GOL4_RAW_BYTES];
    bits_to_raw(bits, decrypted);

    if(gol4_rolling_decode(decrypted)) {
        uint8_t temp_serial[5];
        memcpy(temp_serial, decrypted, 5);
        instance->serial = serial_to_display(temp_serial);
        instance->btn = decrypted[2] & 0x0F;
        instance->cnt = (uint16_t)((decrypted[5] | (decrypted[6] << 8)) & 0xFFFF);
        // Save original button for later use
        if(subghz_custom_btn_get_original() == 0) {
            subghz_custom_btn_set_original(instance->btn);
        }
        subghz_custom_btn_set_max(4);
    }
}

static void subghz_protocol_ditec_gol4_encode_key(SubGhzBlockGeneric* instance) {
    // Encoder crypto part:
    //
    // TODO: Current issue - last bit at original remote sometimes 0 but we encode as 1, or vice versa.
    // This does not affect decoding but may have issue on real receiver
    //
    uint8_t decrypted[GOL4_RAW_BYTES];

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }

    instance->btn = subghz_protocol_ditec_gol4_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&instance->btn))
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", instance->btn);

    // Check for OFEX (overflow experimental) mode
    if(furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF) {
        // standart counter mode. PULL data from subghz_block_generic_global variables
        if(!subghz_block_generic_global_counter_override_get(&instance->cnt)) {
            // if counter_override_get return FALSE then counter was not changed and we increase counter by standart mult value
            if((instance->cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
                instance->cnt = 0;
            } else {
                instance->cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        }
    } else {
        if((instance->cnt + 0x1) > 0xFFFF) {
            instance->cnt = 0;
        } else if(instance->cnt >= 0x1 && instance->cnt != 0xFFFE) {
            instance->cnt = 0xFFFE;
        } else {
            instance->cnt++;
        }
    }

    decrypted[0] = (uint8_t)((instance->serial >> 24) & 0xFF);
    decrypted[4] = (uint8_t)((instance->serial >> 16) & 0xFF);
    decrypted[1] = (uint8_t)((instance->serial >> 8) & 0xFF);
    decrypted[3] = (uint8_t)(instance->serial & 0xFF);
    decrypted[2] = (uint8_t)(instance->btn & 0x0F);

    uint16_t counter = (uint16_t)(instance->cnt & 0xFFFF);
    decrypted[5] = (uint8_t)(counter & 0xFF);
    decrypted[6] = (uint8_t)((counter >> 8) & 0xFF);

    gol4_rolling_encode(decrypted);

    uint8_t bits[subghz_protocol_ditec_gol4_const.min_count_bit_for_found];
    raw_to_bits(decrypted, bits);
    instance->data = bits_to_data(bits);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_ditec_gol4_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderDitecGOL4* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_ditec_gol4_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_ditec_gol4_decode_key(&instance->generic);
        subghz_protocol_ditec_gol4_encode_key(&instance->generic);
        subghz_protocol_encoder_ditec_gol4_get_upload(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }

        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_ditec_gol4_stop(void* context) {
    SubGhzProtocolEncoderDitecGOL4* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_ditec_gol4_yield(void* context) {
    SubGhzProtocolEncoderDitecGOL4* instance = context;

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

void* subghz_protocol_decoder_ditec_gol4_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderDitecGOL4* instance = malloc(sizeof(SubGhzProtocolDecoderDitecGOL4));
    instance->base.protocol = &subghz_protocol_ditec_gol4;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_ditec_gol4_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;
    free(instance);
}

void subghz_protocol_decoder_ditec_gol4_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;
    instance->decoder.parser_step = DitecGOL4DecoderStepReset;
}

void subghz_protocol_decoder_ditec_gol4_feed(void* context, bool level, uint32_t duration) {
    furi_check(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;

    switch(instance->decoder.parser_step) {
    case DitecGOL4DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_ditec_gol4_const.te_long * 22) <
                        (subghz_protocol_ditec_gol4_const.te_long * 4))) {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = DitecGOL4DecoderStepStartBit;
        }
        break;

    case DitecGOL4DecoderStepStartBit:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_ditec_gol4_const.te_short * 2) <
                       subghz_protocol_ditec_gol4_const.te_delta)) {
            instance->decoder.parser_step = DitecGOL4DecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = DitecGOL4DecoderStepReset;
        }
        break;

    case DitecGOL4DecoderStepSaveDuration:
        if(!level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = DitecGOL4DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = DitecGOL4DecoderStepReset;
        }
        break;

    case DitecGOL4DecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_ditec_gol4_const.te_short) <
                subghz_protocol_ditec_gol4_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_ditec_gol4_const.te_long) <
                subghz_protocol_ditec_gol4_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = DitecGOL4DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_ditec_gol4_const.te_long) <
                 subghz_protocol_ditec_gol4_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_ditec_gol4_const.te_short) <
                 subghz_protocol_ditec_gol4_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = DitecGOL4DecoderStepSaveDuration;
            }
        } else {
            if(DURATION_DIFF(
                   instance->decoder.te_last, subghz_protocol_ditec_gol4_const.te_long * 20) <
               (subghz_protocol_ditec_gol4_const.te_long * 3)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_ditec_gol4_const.min_count_bit_for_found) {
                    // 54 bits received, save and continue
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit =
                        subghz_protocol_ditec_gol4_const.min_count_bit_for_found;

                    if(instance->base.callback) {
                        instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = DitecGOL4DecoderStepReset;
            } else {
                instance->decoder.parser_step = DitecGOL4DecoderStepReset;
            }
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_ditec_gol4_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_ditec_gol4_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_ditec_gol4_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_ditec_gol4_const.min_count_bit_for_found);
}

bool subghz_protocol_ditec_gol4_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderDitecGOL4* instance = context;
    instance->generic.btn = btn;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.data_count_bit = subghz_protocol_ditec_gol4_const.min_count_bit_for_found;

    subghz_protocol_ditec_gol4_encode_key(&instance->generic);

    return SubGhzProtocolStatusOk ==
           subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

static uint8_t subghz_protocol_ditec_gol4_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x2;
            break;
        case 0x2:
            btn = 0x1;
            break;
        case 0x4:
            btn = 0x1;
            break;
        case 0x8:
            btn = 0x1;
            break;
        case 0x0:
            btn = 0x1;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x4;
            break;
        case 0x2:
            btn = 0x4;
            break;
        case 0x4:
            btn = 0x2;
            break;
        case 0x8:
            btn = 0x4;
            break;
        case 0x0:
            btn = 0x4;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x8;
            break;
        case 0x2:
            btn = 0x8;
            break;
        case 0x4:
            btn = 0x8;
            break;
        case 0x8:
            btn = 0x2;
            break;
        case 0x0:
            btn = 0x2;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_RIGHT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x0;
            break;
        case 0x2:
            btn = 0x0;
            break;
        case 0x4:
            btn = 0x0;
            break;
        case 0x8:
            btn = 0x0;
            break;
        case 0x0:
            btn = 0x8;
            break;

        default:
            break;
        }
    }

    return btn;
}

void subghz_protocol_decoder_ditec_gol4_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderDitecGOL4* instance = context;

    subghz_protocol_ditec_gol4_decode_key(&instance->generic);

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 16;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 4;
    //

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%0lX%08lX\r\n"
        "Serial:0x%08lX\r\n"
        "Btn:%01X %s\r\n"
        "Cnt:%04lX",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn,
        (instance->generic.btn == 0x0) ? "- Prog" : "",
        instance->generic.cnt);
}
