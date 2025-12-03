#include "phoenix_v2.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolPhoenixV2"

static const SubGhzBlockConst subghz_protocol_phoenix_v2_const = {
    .te_short = 427,
    .te_long = 853,
    .te_delta = 100,
    .min_count_bit_for_found = 52,
};

struct SubGhzProtocolDecoderPhoenix_V2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderPhoenix_V2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Phoenix_V2DecoderStepReset = 0,
    Phoenix_V2DecoderStepFoundStartBit,
    Phoenix_V2DecoderStepSaveDuration,
    Phoenix_V2DecoderStepCheckDuration,
} Phoenix_V2DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_phoenix_v2_decoder = {
    .alloc = subghz_protocol_decoder_phoenix_v2_alloc,
    .free = subghz_protocol_decoder_phoenix_v2_free,

    .feed = subghz_protocol_decoder_phoenix_v2_feed,
    .reset = subghz_protocol_decoder_phoenix_v2_reset,

    .get_hash_data = subghz_protocol_decoder_phoenix_v2_get_hash_data,
    .serialize = subghz_protocol_decoder_phoenix_v2_serialize,
    .deserialize = subghz_protocol_decoder_phoenix_v2_deserialize,
    .get_string = subghz_protocol_decoder_phoenix_v2_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_phoenix_v2_encoder = {
    .alloc = subghz_protocol_encoder_phoenix_v2_alloc,
    .free = subghz_protocol_encoder_phoenix_v2_free,

    .deserialize = subghz_protocol_encoder_phoenix_v2_deserialize,
    .stop = subghz_protocol_encoder_phoenix_v2_stop,
    .yield = subghz_protocol_encoder_phoenix_v2_yield,
};

const SubGhzProtocol subghz_protocol_phoenix_v2 = {
    .name = SUBGHZ_PROTOCOL_PHOENIX_V2_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_phoenix_v2_decoder,
    .encoder = &subghz_protocol_phoenix_v2_encoder,
};

void* subghz_protocol_encoder_phoenix_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderPhoenix_V2* instance = malloc(sizeof(SubGhzProtocolEncoderPhoenix_V2));

    instance->base.protocol = &subghz_protocol_phoenix_v2;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_phoenix_v2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

// Pre define functions
static uint16_t subghz_protocol_phoenix_v2_encrypt_counter(uint64_t full_key, uint16_t counter);
static void subghz_protocol_phoenix_v2_check_remote_controller(SubGhzBlockGeneric* instance);

bool subghz_protocol_phoenix_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    instance->generic.btn = 0x1;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.data_count_bit = 52;

    uint64_t local_data_rev =
        (uint64_t)(((uint64_t)instance->generic.cnt << 40) |
                   ((uint64_t)instance->generic.btn << 32) | (uint64_t)instance->generic.serial);

    uint16_t encrypted_counter = (uint16_t)subghz_protocol_phoenix_v2_encrypt_counter(
        local_data_rev, instance->generic.cnt);

    instance->generic.data = subghz_protocol_blocks_reverse_key(
        (uint64_t)(((uint64_t)encrypted_counter << 40) | ((uint64_t)instance->generic.btn << 32) |
                   (uint64_t)instance->generic.serial),
        instance->generic.data_count_bit + 4);

    return SubGhzProtocolStatusOk ==
           subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

// Get custom button code
static uint8_t subghz_protocol_phoenix_v2_get_btn_code(void) {
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
        case 0x3:
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
        case 0x3:
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
        case 0x3:
            btn = 0x8;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_RIGHT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x3;
            break;
        case 0x2:
            btn = 0x3;
            break;
        case 0x4:
            btn = 0x3;
            break;
        case 0x8:
            btn = 0x3;
            break;
        case 0x3:
            btn = 0x2;
            break;

        default:
            break;
        }
    }

    return btn;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_phoenix_v2_get_upload(SubGhzProtocolEncoderPhoenix_V2* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    uint8_t btn = instance->generic.btn;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(btn);
    }

    // Get custom button code
    // This will override the btn variable if a custom button is set
    btn = subghz_protocol_phoenix_v2_get_btn_code();

    // Reconstruction of the data
    // Check for OFEX (overflow experimental) mode
    if(furi_hal_subghz_get_rolling_counter_mult() != 0xFFFE) {
        if(instance->generic.cnt < 0xFFFF) {
            if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
                instance->generic.cnt = 0;
            } else {
                instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        } else if(
            (instance->generic.cnt >= 0xFFFF) &&
            (furi_hal_subghz_get_rolling_counter_mult() != 0)) {
            instance->generic.cnt = 0;
        }
    } else {
        if((instance->generic.cnt + 0x1) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else if(instance->generic.cnt >= 0x1 && instance->generic.cnt != 0xFFFE) {
            instance->generic.cnt = furi_hal_subghz_get_rolling_counter_mult();
        } else {
            instance->generic.cnt++;
        }
    }

    uint64_t local_data_rev = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit + 4);

    uint16_t encrypted_counter = (uint16_t)subghz_protocol_phoenix_v2_encrypt_counter(
        local_data_rev, instance->generic.cnt);

    instance->generic.data = subghz_protocol_blocks_reverse_key(
        (uint64_t)(((uint64_t)encrypted_counter << 40) | ((uint64_t)btn << 32) |
                   (uint64_t)instance->generic.serial),
        instance->generic.data_count_bit + 4);

    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_short * 60);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_short * 6);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(!bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_phoenix_v2_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_phoenix_v2_const.te_long);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_phoenix_v2_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_phoenix_v2_check_remote_controller(&instance->generic);

        if(!subghz_protocol_encoder_phoenix_v2_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_phoenix_v2_stop(void* context) {
    SubGhzProtocolEncoderPhoenix_V2* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_phoenix_v2_yield(void* context) {
    SubGhzProtocolEncoderPhoenix_V2* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_phoenix_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderPhoenix_V2* instance = malloc(sizeof(SubGhzProtocolDecoderPhoenix_V2));
    instance->base.protocol = &subghz_protocol_phoenix_v2;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_phoenix_v2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    free(instance);
}

void subghz_protocol_decoder_phoenix_v2_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
}

void subghz_protocol_decoder_phoenix_v2_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;

    switch(instance->decoder.parser_step) {
    case Phoenix_V2DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_short * 60) <
                        subghz_protocol_phoenix_v2_const.te_delta * 30)) {
            //Found Preambula
            instance->decoder.parser_step = Phoenix_V2DecoderStepFoundStartBit;
        }
        break;
    case Phoenix_V2DecoderStepFoundStartBit:
        if(level && (DURATION_DIFF(duration, (subghz_protocol_phoenix_v2_const.te_short * 6)) <
                     subghz_protocol_phoenix_v2_const.te_delta * 4)) {
            //Found start bit
            instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
        }
        break;
    case Phoenix_V2DecoderStepSaveDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_phoenix_v2_const.te_short * 10 +
                            subghz_protocol_phoenix_v2_const.te_delta)) {
                instance->decoder.parser_step = Phoenix_V2DecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_phoenix_v2_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = Phoenix_V2DecoderStepCheckDuration;
            }
        }
        break;
    case Phoenix_V2DecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_phoenix_v2_const.te_short) <
                subghz_protocol_phoenix_v2_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_long) <
                subghz_protocol_phoenix_v2_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_phoenix_v2_const.te_long) <
                 subghz_protocol_phoenix_v2_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_phoenix_v2_const.te_short) <
                 subghz_protocol_phoenix_v2_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Phoenix_V2DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Phoenix_V2DecoderStepReset;
        }
        break;
    }
}

static uint16_t subghz_protocol_phoenix_v2_encrypt_counter(uint64_t full_key, uint16_t counter) {
    uint8_t xor_key1 = (uint8_t)(full_key >> 24); // First byte of serial
    uint8_t xor_key2 = (uint8_t)((full_key >> 16) & 0xFF); // Second byte of serial

    uint8_t byte2 = (uint8_t)(counter >> 8); // First counter byte
    uint8_t byte1 = (uint8_t)(counter & 0xFF); // Second counter byte

    // See decrypt function before reading these comments
    for(int i = 0; i < 16; i++) {
        // The key to reversing the process is that the MSB of the *current* byte2
        // tells us what the MSB of the *previous* byte1 was. This allows us to
        // determine if the conditional XOR was applied before?.
        uint8_t msb_of_prev_byte1 = byte2 & 0x80;

        if(msb_of_prev_byte1 == 0) {
            // reverse the XOR.
            byte2 ^= xor_key2;
            byte1 ^= xor_key1;
        }

        // Perform the bit shuffle in reverse
        // Store the least significant bit (LSB) of the current byte1.
        uint8_t lsb_of_current_byte1 = byte1 & 1;

        byte2 = (byte2 << 1) | lsb_of_current_byte1;
        byte1 = (byte1 >> 1) | msb_of_prev_byte1;
    }

    return (uint16_t)byte1 << 8 | byte2;
}

static uint16_t subghz_protocol_phoenix_v2_decrypt_counter(uint64_t full_key) {
    uint16_t encrypted_value = (uint16_t)((full_key >> 40) & 0xFFFF);

    uint8_t byte1 = (uint8_t)(encrypted_value >> 8); // First encrypted counter byte
    uint8_t byte2 = (uint8_t)(encrypted_value & 0xFF); // Second encrypted counter byte

    uint8_t xor_key1 = (uint8_t)(full_key >> 24); // First byte of serial
    uint8_t xor_key2 = (uint8_t)((full_key >> 16) & 0xFF); // Second byte of serial

    for(int i = 0; i < 16; i++) {
        // Store the most significant bit (MSB) of byte1.
        // The check `(msb_of_byte1 == 0)` will determine if we apply the XOR keys.
        uint8_t msb_of_byte1 = byte1 & 0x80;

        // Store the least significant bit (LSB) of byte2.
        uint8_t lsb_of_byte2 = byte2 & 1;

        // Perform a bit shuffle between the two bytes
        byte2 = (byte2 >> 1) | msb_of_byte1;
        byte1 = (byte1 << 1) | lsb_of_byte2;

        // Conditionally apply the XOR keys based on the original MSB of byte1.
        if(msb_of_byte1 == 0) {
            byte1 ^= xor_key1;
            // The mask `& 0x7F` clears the MSB of byte2 after the XOR.
            byte2 = (byte2 ^ xor_key2) & 0x7F;
        }
    }

    return (uint16_t)byte2 << 8 | byte1;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_phoenix_v2_check_remote_controller(SubGhzBlockGeneric* instance) {
    // 2022.08 - @Skorpionm
    // 2025.07 - @xMasterX & @RocketGod-git
    // Fully supported now, with button switch and add manually
    //
    // Key samples
    // Full key example: 0xC63E01B9615720 - after subghz_protocol_blocks_reverse_key was applied
    // Serial - B9615720
    // Button - 01
    // Encrypted -> Decrypted counters
    // C63E - 025C
    // BCC1 - 025D
    // 3341 - 025E
    // 49BE - 025F
    // 99D3 - 0260
    // E32C - 0261

    uint64_t data_rev =
        subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit + 4);

    instance->serial = data_rev & 0xFFFFFFFF;
    instance->cnt = subghz_protocol_phoenix_v2_decrypt_counter(data_rev);
    instance->btn = (data_rev >> 32) & 0xF;
    // encrypted cnt is (data_rev >> 40) & 0xFFFF

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(4);
}

uint8_t subghz_protocol_decoder_phoenix_v2_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_phoenix_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_phoenix_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_phoenix_v2_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_phoenix_v2_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderPhoenix_V2* instance = context;
    subghz_protocol_phoenix_v2_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "V2 Phoenix %dbit\r\n"
        "Key:%05lX%08lX\r\n"
        "Sn:0x%07lX \r\n"
        "Cnt:0x%04lX\r\n"
        "Btn:%X\r\n",
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32) & 0xFFFFFFFF,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.cnt,
        instance->generic.btn);
}
