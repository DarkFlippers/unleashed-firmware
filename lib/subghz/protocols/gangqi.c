#include "gangqi.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolGangQi"

static const SubGhzBlockConst subghz_protocol_gangqi_const = {
    .te_short = 500,
    .te_long = 1200,
    .te_delta = 200,
    .min_count_bit_for_found = 34,
};

struct SubGhzProtocolDecoderGangQi {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderGangQi {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    GangQiDecoderStepReset = 0,
    GangQiDecoderStepSaveDuration,
    GangQiDecoderStepCheckDuration,
} GangQiDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_gangqi_decoder = {
    .alloc = subghz_protocol_decoder_gangqi_alloc,
    .free = subghz_protocol_decoder_gangqi_free,

    .feed = subghz_protocol_decoder_gangqi_feed,
    .reset = subghz_protocol_decoder_gangqi_reset,

    .get_hash_data = subghz_protocol_decoder_gangqi_get_hash_data,
    .serialize = subghz_protocol_decoder_gangqi_serialize,
    .deserialize = subghz_protocol_decoder_gangqi_deserialize,
    .get_string = subghz_protocol_decoder_gangqi_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_gangqi_encoder = {
    .alloc = subghz_protocol_encoder_gangqi_alloc,
    .free = subghz_protocol_encoder_gangqi_free,

    .deserialize = subghz_protocol_encoder_gangqi_deserialize,
    .stop = subghz_protocol_encoder_gangqi_stop,
    .yield = subghz_protocol_encoder_gangqi_yield,
};

const SubGhzProtocol subghz_protocol_gangqi = {
    .name = SUBGHZ_PROTOCOL_GANGQI_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send |
            SubGhzProtocolFlag_Alarms,

    .decoder = &subghz_protocol_gangqi_decoder,
    .encoder = &subghz_protocol_gangqi_encoder,
};

void* subghz_protocol_encoder_gangqi_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderGangQi* instance = malloc(sizeof(SubGhzProtocolEncoderGangQi));

    instance->base.protocol = &subghz_protocol_gangqi;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_gangqi_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGangQi* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

// Get custom button code
static uint8_t subghz_protocol_gangqi_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0xD:
            btn = 0xE;
            break;
        case 0xE:
            btn = 0xD;
            break;
        case 0xB:
            btn = 0xD;
            break;
        case 0x7:
            btn = 0xD;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0xD:
            btn = 0xB;
            break;
        case 0xE:
            btn = 0xB;
            break;
        case 0xB:
            btn = 0xE;
            break;
        case 0x7:
            btn = 0xE;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0xD:
            btn = 0x7;
            break;
        case 0xE:
            btn = 0x7;
            break;
        case 0xB:
            btn = 0x7;
            break;
        case 0x7:
            btn = 0xB;
            break;

        default:
            break;
        }
    }

    return btn;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderGangQi instance
 */
static void subghz_protocol_encoder_gangqi_get_upload(SubGhzProtocolEncoderGangQi* instance) {
    furi_assert(instance);

    // Generate new key using custom or default button
    instance->generic.btn = subghz_protocol_gangqi_get_btn_code();

    uint16_t serial = (uint16_t)((instance->generic.data >> 18) & 0xFFFF);
    uint8_t const_and_button = (uint8_t)(0xD0 | instance->generic.btn);
    uint8_t serial_high = (uint8_t)(serial >> 8);
    uint8_t serial_low = (uint8_t)(serial & 0xFF);
    uint8_t bytesum = (uint8_t)(0xC8 - serial_high - serial_low - const_and_button);

    instance->generic.data = (instance->generic.data >> 14) << 14 | (instance->generic.btn << 10) |
                             (bytesum << 2);

    size_t index = 0;

    // Send key and GAP between parcels
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gangqi_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_gangqi_const.te_short * 4 +
                        subghz_protocol_gangqi_const.te_delta);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_gangqi_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gangqi_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false,
                    (uint32_t)subghz_protocol_gangqi_const.te_short * 4 +
                        subghz_protocol_gangqi_const.te_delta);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_gangqi_const.te_long);
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
static void subghz_protocol_gangqi_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = (instance->data >> 10) & 0xF;
    instance->serial = (instance->data & 0xFFFFF0000) >> 16;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(3);

    // GangQi Decoder
    // 09.2024 - @xMasterX (MMX) (last update - bytesum calculation at 02.2025)
    // Thanks @Skorpionm for support!
    // Thanks @Drone1950 and @mishamyte (who spent 2 weeks on this) for making this work properly

    // Example of correct bytesum calculation
    // 0xC8 - serial_high - serial_low - constant_and_button
}

SubGhzProtocolStatus
    subghz_protocol_encoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderGangQi* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_gangqi_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_gangqi_remote_controller(&instance->generic);
        subghz_protocol_encoder_gangqi_get_upload(instance);

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

void subghz_protocol_encoder_gangqi_stop(void* context) {
    SubGhzProtocolEncoderGangQi* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_gangqi_yield(void* context) {
    SubGhzProtocolEncoderGangQi* instance = context;

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

void* subghz_protocol_decoder_gangqi_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderGangQi* instance = malloc(sizeof(SubGhzProtocolDecoderGangQi));
    instance->base.protocol = &subghz_protocol_gangqi;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_gangqi_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    free(instance);
}

void subghz_protocol_decoder_gangqi_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    instance->decoder.parser_step = GangQiDecoderStepReset;
}

void subghz_protocol_decoder_gangqi_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;

    switch(instance->decoder.parser_step) {
    case GangQiDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_long * 2) <
                        subghz_protocol_gangqi_const.te_delta * 3)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
        }
        break;
    case GangQiDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = GangQiDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = GangQiDecoderStepReset;
        }
        break;
    case GangQiDecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short and long timing
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_short) <
                subghz_protocol_gangqi_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_long) <
                subghz_protocol_gangqi_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
                // Bit 1 is long and short timing
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_long) <
                 subghz_protocol_gangqi_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short) <
                 subghz_protocol_gangqi_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = GangQiDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                subghz_protocol_gangqi_const.te_delta) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_gangqi_const.te_short) <
                    subghz_protocol_gangqi_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                    subghz_protocol_gangqi_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gangqi_const.te_long) <
                    subghz_protocol_gangqi_const.te_delta) &&
                   (DURATION_DIFF(duration, subghz_protocol_gangqi_const.te_short * 4) <
                    subghz_protocol_gangqi_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 34 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_gangqi_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = GangQiDecoderStepReset;
            } else {
                instance->decoder.parser_step = GangQiDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = GangQiDecoderStepReset;
        }
        break;
    }
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_gangqi_get_button_name(uint8_t btn) {
    const char* name_btn[16] = {
        "Unknown",
        "Exit settings",
        "Volume setting",
        "0x3",
        "Vibro sens. setting",
        "Settings mode",
        "Ringtone setting",
        "Ring", // D
        "0x8",
        "0x9",
        "0xA",
        "Alarm", // C
        "0xC",
        "Arm", // A
        "Disarm", // B
        "0xF"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

uint8_t subghz_protocol_decoder_gangqi_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_gangqi_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_gangqi_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_gangqi_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_gangqi_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderGangQi* instance = context;

    // Parse serial
    subghz_protocol_gangqi_remote_controller(&instance->generic);

    // Get byte sum
    uint16_t serial = (uint16_t)((instance->generic.data >> 18) & 0xFFFF);
    uint8_t const_and_button = (uint8_t)(0xD0 | instance->generic.btn);
    uint8_t serial_high = (uint8_t)(serial >> 8);
    uint8_t serial_low = (uint8_t)(serial & 0xFF);
    // Type 1 is what original remotes use, type 2 is "backdoor" sum that receiver accepts too
    uint8_t sum_type1 = (uint8_t)(0xC8 - serial_high - serial_low - const_and_button);
    uint8_t sum_type2 = (uint8_t)(0x02 + serial_high + serial_low + const_and_button);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%X%08lX\r\n"
        "Serial: 0x%05lX\r\n"
        "Sum: 0x%02X   Sum2: 0x%02X\r\n"
        "Btn: 0x%01X - %s\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint8_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        sum_type1,
        sum_type2,
        instance->generic.btn,
        subghz_protocol_gangqi_get_button_name(instance->generic.btn));
}
