#include "hay21.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolHay21"

static const SubGhzBlockConst subghz_protocol_hay21_const = {
    .te_short = 300,
    .te_long = 700,
    .te_delta = 150,
    .min_count_bit_for_found = 21,
};

struct SubGhzProtocolDecoderHay21 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderHay21 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Hay21DecoderStepReset = 0,
    Hay21DecoderStepSaveDuration,
    Hay21DecoderStepCheckDuration,
} Hay21DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_hay21_decoder = {
    .alloc = subghz_protocol_decoder_hay21_alloc,
    .free = subghz_protocol_decoder_hay21_free,

    .feed = subghz_protocol_decoder_hay21_feed,
    .reset = subghz_protocol_decoder_hay21_reset,

    .get_hash_data = subghz_protocol_decoder_hay21_get_hash_data,
    .serialize = subghz_protocol_decoder_hay21_serialize,
    .deserialize = subghz_protocol_decoder_hay21_deserialize,
    .get_string = subghz_protocol_decoder_hay21_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_hay21_encoder = {
    .alloc = subghz_protocol_encoder_hay21_alloc,
    .free = subghz_protocol_encoder_hay21_free,

    .deserialize = subghz_protocol_encoder_hay21_deserialize,
    .stop = subghz_protocol_encoder_hay21_stop,
    .yield = subghz_protocol_encoder_hay21_yield,
};

const SubGhzProtocol subghz_protocol_hay21 = {
    .name = SUBGHZ_PROTOCOL_HAY21_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_hay21_decoder,
    .encoder = &subghz_protocol_hay21_encoder,
};

void* subghz_protocol_encoder_hay21_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHay21* instance = malloc(sizeof(SubGhzProtocolEncoderHay21));

    instance->base.protocol = &subghz_protocol_hay21;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_hay21_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHay21* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

// Get custom button code
static uint8_t subghz_protocol_hay21_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x5A:
            btn = 0xC3;
            break;
        case 0xC3:
            btn = 0x5A;
            break;
        case 0x88:
            btn = 0x5A;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x5A:
            btn = 0x88;
            break;
        case 0xC3:
            btn = 0x88;
            break;
        case 0x88:
            btn = 0xC3;
            break;

        default:
            break;
        }
    }

    return btn;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHay21 instance
 */
static void subghz_protocol_encoder_hay21_get_upload(SubGhzProtocolEncoderHay21* instance) {
    furi_assert(instance);

    // Generate new key using custom or default button
    instance->generic.btn = subghz_protocol_hay21_get_btn_code();

    // Counter increment
    if(instance->generic.cnt < 0xF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xF) {
        instance->generic.cnt = 0;
    }

    // Reconstruction of the data
    instance->generic.data =
        ((uint64_t)instance->generic.btn << 13 | (uint64_t)instance->generic.serial << 5 |
         instance->generic.cnt << 1) |
        0b1;

    size_t index = 0;

    // Send key and GAP between parcels
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_hay21_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hay21_const.te_long * 6);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hay21_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_hay21_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hay21_const.te_long * 6);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_hay21_const.te_long);
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
static void subghz_protocol_hay21_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = (instance->data >> 13) & 0xFF;
    instance->serial = (instance->data >> 5) & 0xFF;
    instance->cnt = (instance->data >> 1) & 0xF;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(2);

    // Hay21 Decoder
    // 09.2024 - @xMasterX (MMX)

    // Key samples (inverted)
    //              button   serial     CNT (goes lower since 0/1 are inverted)
    //14A84A = 000 10100101 01000010 0101 0 (cnt 5)
    //14A848 = 000 10100101 01000010 0100 0 (cnt 4)
    //14A846 = 000 10100101 01000010 0011 0 (cnt 3)
    //14A844 = 000 10100101 01000010 0010 0 (cnt 2)
    //14A842 = 000 10100101 01000010 0001 0 (cnt 1)
    //14A840 = 000 10100101 01000010 0000 0 (cnt 0)
    //14A85E = 000 10100101 01000010 1111 0 (cnt F)
    //14A85C = 000 10100101 01000010 1110 0 (cnt E)
    //14A85A = 000 10100101 01000010 1101 0 (cnt D)
    //14A858 = 000 10100101 01000010 1100 0 (cnt C)
    //14A856 = 000 10100101 01000010 1011 0 (cnt B)
    //          0xA5 (Labeled as On/Off on the remote board)
    //          0x3C (Labeled as Mode on the remote board)
    //          0x42 (Serial)
    //                BTN   Serial   CNT
    //078854 = 000 00111100 01000010 1010 0 (cnt A)
    //078852 = 000 00111100 01000010 1001 0 (cnt 9)
    //078850 = 000 00111100 01000010 1000 0 (cnt 8)
    //07884E = 000 00111100 01000010 0111 0 (cnt 7)
    // Inverted back
    //1877B9 = 000 11000011 10111101 1100 1
    //1877BB = 000 11000011 10111101 1101 1
    //1877BD = 000 11000011 10111101 1110 1
    //0B57BF = 000 01011010 10111101 1111 1
}

SubGhzProtocolStatus
    subghz_protocol_encoder_hay21_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHay21* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_hay21_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_hay21_remote_controller(&instance->generic);
        subghz_protocol_encoder_hay21_get_upload(instance);

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

void subghz_protocol_encoder_hay21_stop(void* context) {
    SubGhzProtocolEncoderHay21* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_hay21_yield(void* context) {
    SubGhzProtocolEncoderHay21* instance = context;

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

void* subghz_protocol_decoder_hay21_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHay21* instance = malloc(sizeof(SubGhzProtocolDecoderHay21));
    instance->base.protocol = &subghz_protocol_hay21;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_hay21_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;
    free(instance);
}

void subghz_protocol_decoder_hay21_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;
    instance->decoder.parser_step = Hay21DecoderStepReset;
}

void subghz_protocol_decoder_hay21_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;

    switch(instance->decoder.parser_step) {
    case Hay21DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_hay21_const.te_long * 6) <
                        subghz_protocol_hay21_const.te_delta * 3)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = Hay21DecoderStepSaveDuration;
        }
        break;
    case Hay21DecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Hay21DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Hay21DecoderStepReset;
        }
        break;
    case Hay21DecoderStepCheckDuration:
        if(!level) {
            // Bit 1 is long + short timing
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hay21_const.te_long) <
                subghz_protocol_hay21_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_hay21_const.te_short) <
                subghz_protocol_hay21_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Hay21DecoderStepSaveDuration;
                // Bit 0 is short + long timing
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hay21_const.te_short) <
                 subghz_protocol_hay21_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_hay21_const.te_long) <
                 subghz_protocol_hay21_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Hay21DecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_hay21_const.te_long * 6) <
                subghz_protocol_hay21_const.te_delta * 2) {
                //Found next GAP and add bit 0 or 1
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hay21_const.te_long) <
                    subghz_protocol_hay21_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_hay21_const.te_short) <
                    subghz_protocol_hay21_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                // If got 21 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_hay21_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = Hay21DecoderStepReset;
            } else {
                instance->decoder.parser_step = Hay21DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Hay21DecoderStepReset;
        }
        break;
    }
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_hay21_get_button_name(uint8_t btn) {
    const char* btn_name;
    switch(btn) {
    case 0x5A:
        btn_name = "On/Off";
        break;
    case 0xC3:
        btn_name = "Mode";
        break;
    case 0x88:
        btn_name = "Hold";
        break;
    default:
        btn_name = "Unknown";
        break;
    }
    return btn_name;
}

uint8_t subghz_protocol_decoder_hay21_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_hay21_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_hay21_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_hay21_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_hay21_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderHay21* instance = context;

    // Parse serial, button, counter
    subghz_protocol_hay21_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s - %dbit\r\n"
        "Key: 0x%06lX\r\n"
        "Serial: 0x%02X\r\n"
        "Btn: 0x%01X - %s\r\n"
        "Cnt: 0x%01X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        (uint8_t)(instance->generic.serial & 0xFF),
        instance->generic.btn,
        subghz_protocol_hay21_get_button_name(instance->generic.btn),
        (uint8_t)(instance->generic.cnt & 0xF));
}
