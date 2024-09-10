#include "hollarm.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolHollarm"

static const SubGhzBlockConst subghz_protocol_hollarm_const = {
    .te_short = 200,
    .te_long = 1000,
    .te_delta = 200,
    .min_count_bit_for_found = 42,
};

struct SubGhzProtocolDecoderHollarm {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderHollarm {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    HollarmDecoderStepReset = 0,
    HollarmDecoderStepSaveDuration,
    HollarmDecoderStepCheckDuration,
} HollarmDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_hollarm_decoder = {
    .alloc = subghz_protocol_decoder_hollarm_alloc,
    .free = subghz_protocol_decoder_hollarm_free,

    .feed = subghz_protocol_decoder_hollarm_feed,
    .reset = subghz_protocol_decoder_hollarm_reset,

    .get_hash_data = subghz_protocol_decoder_hollarm_get_hash_data,
    .serialize = subghz_protocol_decoder_hollarm_serialize,
    .deserialize = subghz_protocol_decoder_hollarm_deserialize,
    .get_string = subghz_protocol_decoder_hollarm_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_hollarm_encoder = {
    .alloc = subghz_protocol_encoder_hollarm_alloc,
    .free = subghz_protocol_encoder_hollarm_free,

    .deserialize = subghz_protocol_encoder_hollarm_deserialize,
    .stop = subghz_protocol_encoder_hollarm_stop,
    .yield = subghz_protocol_encoder_hollarm_yield,
};

const SubGhzProtocol subghz_protocol_hollarm = {
    .name = SUBGHZ_PROTOCOL_HOLLARM_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_hollarm_decoder,
    .encoder = &subghz_protocol_hollarm_encoder,
};

void* subghz_protocol_encoder_hollarm_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHollarm* instance = malloc(sizeof(SubGhzProtocolEncoderHollarm));

    instance->base.protocol = &subghz_protocol_hollarm;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_hollarm_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHollarm* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

// Get custom button code
static uint8_t subghz_protocol_hollarm_get_btn_code(void) {
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

        default:
            break;
        }
    }

    return btn;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHollarm instance
 */
static void subghz_protocol_encoder_hollarm_get_upload(SubGhzProtocolEncoderHollarm* instance) {
    furi_assert(instance);

    // Generate new key using custom or default button
    instance->generic.btn = subghz_protocol_hollarm_get_btn_code();

    uint64_t new_key = (instance->generic.data >> 12) << 12 | (instance->generic.btn << 8);

    uint8_t crc = ((new_key >> 32) & 0xFF) + ((new_key >> 24) & 0xFF) + ((new_key >> 16) & 0xFF) +
                  ((new_key >> 8) & 0xFF);

    instance->generic.data = (new_key | crc);

    size_t index = 0;

    // Send key and GAP between parcels
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        // Read and prepare levels with 2 bit (was saved for better parsing) to the left offset to fit with the original remote transmission
        if(bit_read((instance->generic.data << 2), i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_hollarm_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_hollarm_const.te_short * 12);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_hollarm_const.te_short * 8);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_hollarm_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_hollarm_const.te_short * 12);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hollarm_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data and parsing serial number
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_hollarm_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = (instance->data >> 8) & 0xF;
    instance->serial = (instance->data & 0xFFFFFFF0000) >> 16;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(3);

    // Hollarm Decoder
    // 09.2024 - @xMasterX (MMX)
    // Thanks @Skorpionm for support!

    // F0B93422FF = FF 8bit Sum
    // F0B93421FE = FE 8bit Sum
    // F0B9342401 = 01 8bit Sum
    // F0B9342805 = 05 8bit Sum

    // Serial (moved 2bit to right)    | Btn | 8b CRC (previous 4 bytes sum)
    // 00001111000010111001001101000010 0010  11111111 btn = (0x2)
    // 00001111000010111001001101000010 0001  11111110 btn = (0x1)
    // 00001111000010111001001101000010 0100  00000001 btn = (0x4)
    // 00001111000010111001001101000010 1000  00000101 btn = (0x8)
}

SubGhzProtocolStatus
    subghz_protocol_encoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHollarm* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_hollarm_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_hollarm_remote_controller(&instance->generic);
        subghz_protocol_encoder_hollarm_get_upload(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
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

void subghz_protocol_encoder_hollarm_stop(void* context) {
    SubGhzProtocolEncoderHollarm* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_hollarm_yield(void* context) {
    SubGhzProtocolEncoderHollarm* instance = context;

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

void* subghz_protocol_decoder_hollarm_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHollarm* instance = malloc(sizeof(SubGhzProtocolDecoderHollarm));
    instance->base.protocol = &subghz_protocol_hollarm;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_hollarm_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;
    free(instance);
}

void subghz_protocol_decoder_hollarm_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;
    instance->decoder.parser_step = HollarmDecoderStepReset;
}

void subghz_protocol_decoder_hollarm_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;

    switch(instance->decoder.parser_step) {
    case HollarmDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_hollarm_const.te_short * 12) <
                        subghz_protocol_hollarm_const.te_delta * 2)) {
            //Found GAP between parcels
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = HollarmDecoderStepSaveDuration;
        }
        break;
    case HollarmDecoderStepSaveDuration:
        // Save HIGH level timing for next step
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = HollarmDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = HollarmDecoderStepReset;
        }
        break;
    case HollarmDecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short 200us HIGH + long 1000us LOW timing
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hollarm_const.te_short) <
                subghz_protocol_hollarm_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_hollarm_const.te_long) <
                subghz_protocol_hollarm_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = HollarmDecoderStepSaveDuration;
                // Bit 1 is short 200us HIGH + short x8 = 1600us LOW timing
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hollarm_const.te_short) <
                 subghz_protocol_hollarm_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_hollarm_const.te_short * 8) <
                 subghz_protocol_hollarm_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = HollarmDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_hollarm_const.te_short * 12) <
                subghz_protocol_hollarm_const.te_delta) {
                // When next GAP is found add bit 0 and do check for read finish
                // (we have 42 high level pulses, last or first one may be a stop/start bit but we will parse it as zero)
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);

                // If got 42 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_hollarm_const.min_count_bit_for_found) {
                    // Saving with 2bit to the right offset for proper parsing
                    instance->generic.data = (instance->decoder.decode_data >> 2);
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = HollarmDecoderStepReset;
            } else {
                instance->decoder.parser_step = HollarmDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = HollarmDecoderStepReset;
        }
        break;
    }
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_hollarm_get_button_name(uint8_t btn) {
    const char* name_btn[16] = {
        "Unknown",
        "Disarm", // B (2)
        "Arm", // A (1)
        "0x3",
        "Alarm", // C (3)
        "0x5",
        "0x6",
        "0x7",
        "Ring", // D (4)
        "0x9",
        "0xA",
        "0xB",
        "0xC",
        "0xD",
        "0xE",
        "0xF"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

uint8_t subghz_protocol_decoder_hollarm_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_hollarm_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_hollarm_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_hollarm_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderHollarm* instance = context;

    // Parse serial
    subghz_protocol_hollarm_remote_controller(&instance->generic);
    // Get CRC
    uint8_t crc = ((instance->generic.data >> 32) & 0xFF) +
                  ((instance->generic.data >> 24) & 0xFF) +
                  ((instance->generic.data >> 16) & 0xFF) + ((instance->generic.data >> 8) & 0xFF);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%02lX%08lX\r\n"
        "Serial: 0x%06lX  CRC: %02X\r\n"
        "Btn: 0x%01X - %s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->generic.serial,
        crc,
        instance->generic.btn,
        subghz_protocol_hollarm_get_button_name(instance->generic.btn));
}
