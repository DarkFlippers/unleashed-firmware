#include "nord_ice.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolNord_Ice"

static const SubGhzBlockConst subghz_protocol_nord_ice_const = {
    .te_short = 300,
    .te_long = 800,
    .te_delta = 150,
    .min_count_bit_for_found = 33,
};

struct SubGhzProtocolDecoderNord_Ice {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderNord_Ice {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Nord_IceDecoderStepReset = 0,
    Nord_IceDecoderStepSaveDuration,
    Nord_IceDecoderStepCheckDuration,
} Nord_IceDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_nord_ice_decoder = {
    .alloc = subghz_protocol_decoder_nord_ice_alloc,
    .free = subghz_protocol_decoder_nord_ice_free,

    .feed = subghz_protocol_decoder_nord_ice_feed,
    .reset = subghz_protocol_decoder_nord_ice_reset,

    .get_hash_data = subghz_protocol_decoder_nord_ice_get_hash_data,
    .serialize = subghz_protocol_decoder_nord_ice_serialize,
    .deserialize = subghz_protocol_decoder_nord_ice_deserialize,
    .get_string = subghz_protocol_decoder_nord_ice_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nord_ice_encoder = {
    .alloc = subghz_protocol_encoder_nord_ice_alloc,
    .free = subghz_protocol_encoder_nord_ice_free,

    .deserialize = subghz_protocol_encoder_nord_ice_deserialize,
    .stop = subghz_protocol_encoder_nord_ice_stop,
    .yield = subghz_protocol_encoder_nord_ice_yield,
};

const SubGhzProtocol subghz_protocol_nord_ice = {
    .name = SUBGHZ_PROTOCOL_NORD_ICE_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_nord_ice_decoder,
    .encoder = &subghz_protocol_nord_ice_encoder,
};

void* subghz_protocol_encoder_nord_ice_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderNord_Ice* instance = malloc(sizeof(SubGhzProtocolEncoderNord_Ice));

    instance->base.protocol = &subghz_protocol_nord_ice;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 3;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_nord_ice_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderNord_Ice* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderNord_Ice instance
 */
static void subghz_protocol_encoder_nord_ice_get_upload(SubGhzProtocolEncoderNord_Ice* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nord_ice_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_nord_ice_const.te_short * 25);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_nord_ice_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nord_ice_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_nord_ice_const.te_short * 25);
            } else {
                instance->encoder.upload[index++] =
                    level_duration_make(false, (uint32_t)subghz_protocol_nord_ice_const.te_long);
            }
        }
    }

    instance->encoder.size_upload = index;
    return;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_nord_ice_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = (instance->data >> 15) << 9 |
                       (instance->data & 0x1FF); // 26 bits for serial
    instance->btn = (instance->data >> 9) & 0x3F; // 6 bits for button
}

SubGhzProtocolStatus
    subghz_protocol_encoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderNord_Ice* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_nord_ice_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_nord_ice_check_remote_controller(&instance->generic);
        subghz_protocol_encoder_nord_ice_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_nord_ice_stop(void* context) {
    SubGhzProtocolEncoderNord_Ice* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_nord_ice_yield(void* context) {
    SubGhzProtocolEncoderNord_Ice* instance = context;

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

void* subghz_protocol_decoder_nord_ice_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderNord_Ice* instance = malloc(sizeof(SubGhzProtocolDecoderNord_Ice));
    instance->base.protocol = &subghz_protocol_nord_ice;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_nord_ice_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;
    free(instance);
}

void subghz_protocol_decoder_nord_ice_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;
    instance->decoder.parser_step = Nord_IceDecoderStepReset;
}

void subghz_protocol_decoder_nord_ice_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;

    // Nord ICE Decoder
    // 2026.03 - @xMasterX (MMX)

    // Key samples
    //
    //                      Serial          Btn    Serial
    // 0x9467688A btn 1 = 10010100011001110 110100 010001010
    // 0x9467308A btn 2 = 10010100011001110 011000 010001010
    // 0x9467628A btn 3 = 10010100011001110 110001 010001010
    // 0x9467648A btn 4 = 10010100011001110 110010 010001010

    switch(instance->decoder.parser_step) {
    case Nord_IceDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nord_ice_const.te_short * 25) <
                        subghz_protocol_nord_ice_const.te_delta * 11)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = Nord_IceDecoderStepSaveDuration;
        }
        break;
    case Nord_IceDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Nord_IceDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Nord_IceDecoderStepReset;
        }
        break;
    case Nord_IceDecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short and long timing = 300us HIGH (te_last) and 800us LOW
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nord_ice_const.te_short) <
                subghz_protocol_nord_ice_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_nord_ice_const.te_long) <
                subghz_protocol_nord_ice_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Nord_IceDecoderStepSaveDuration;
                // Bit 1 is long and short timing = 800us HIGH (te_last) and 300us LOW
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nord_ice_const.te_long) <
                 subghz_protocol_nord_ice_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nord_ice_const.te_short) <
                 subghz_protocol_nord_ice_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Nord_IceDecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_nord_ice_const.te_short * 25) <
                subghz_protocol_nord_ice_const.te_delta * 11) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_nord_ice_const.te_short) <
                    subghz_protocol_nord_ice_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_nord_ice_const.te_long) <
                    subghz_protocol_nord_ice_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 33 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_nord_ice_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = Nord_IceDecoderStepReset;
            } else {
                instance->decoder.parser_step = Nord_IceDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Nord_IceDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_nord_ice_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_nord_ice_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_nord_ice_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_nord_ice_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderNord_Ice* instance = context;

    subghz_protocol_nord_ice_check_remote_controller(&instance->generic);

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    // for future use
    // // push protocol data to global variable
    // subghz_block_generic_global.btn_is_available = false;
    // subghz_block_generic_global.current_btn = instance->generic.btn;
    // subghz_block_generic_global.btn_length_bit = 4;
    // //

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key: 0x%08llX\r\n"
        "Yek: 0x%08llX\r\n"
        "Serial: 0x%07lX\r\n"
        "Btn: %02X",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint64_t)(instance->generic.data & 0xFFFFFFFFF),
        (code_found_reverse & 0xFFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
