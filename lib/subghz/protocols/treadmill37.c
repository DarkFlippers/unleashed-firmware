#include "treadmill37.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolTreadmill37"

static const SubGhzBlockConst subghz_protocol_treadmill37_const = {
    .te_short = 300,
    .te_long = 900,
    .te_delta = 150,
    .min_count_bit_for_found = 37,
};

struct SubGhzProtocolDecoderTreadmill37 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderTreadmill37 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Treadmill37DecoderStepReset = 0,
    Treadmill37DecoderStepSaveDuration,
    Treadmill37DecoderStepCheckDuration,
} Treadmill37DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_treadmill37_decoder = {
    .alloc = subghz_protocol_decoder_treadmill37_alloc,
    .free = subghz_protocol_decoder_treadmill37_free,

    .feed = subghz_protocol_decoder_treadmill37_feed,
    .reset = subghz_protocol_decoder_treadmill37_reset,

    .get_hash_data = subghz_protocol_decoder_treadmill37_get_hash_data,
    .serialize = subghz_protocol_decoder_treadmill37_serialize,
    .deserialize = subghz_protocol_decoder_treadmill37_deserialize,
    .get_string = subghz_protocol_decoder_treadmill37_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_treadmill37_encoder = {
    .alloc = subghz_protocol_encoder_treadmill37_alloc,
    .free = subghz_protocol_encoder_treadmill37_free,

    .deserialize = subghz_protocol_encoder_treadmill37_deserialize,
    .stop = subghz_protocol_encoder_treadmill37_stop,
    .yield = subghz_protocol_encoder_treadmill37_yield,
};

const SubGhzProtocol subghz_protocol_treadmill37 = {
    .name = SUBGHZ_PROTOCOL_TREADMILL37_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_treadmill37_decoder,
    .encoder = &subghz_protocol_treadmill37_encoder,
};

void* subghz_protocol_encoder_treadmill37_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderTreadmill37* instance = malloc(sizeof(SubGhzProtocolEncoderTreadmill37));

    instance->base.protocol = &subghz_protocol_treadmill37;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 3;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_treadmill37_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderTreadmill37* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderTreadmill37 instance
 */
static void
    subghz_protocol_encoder_treadmill37_get_upload(SubGhzProtocolEncoderTreadmill37* instance) {
    furi_assert(instance);
    size_t index = 0;

    // Send key and GAP
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_treadmill37_const.te_long);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_treadmill37_const.te_short * 20);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_treadmill37_const.te_short);
            }
        } else {
            // Send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_treadmill37_const.te_short);
            if(i == 1) {
                //Send gap if bit was last
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_treadmill37_const.te_short * 20);
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_treadmill37_const.te_long);
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
static void subghz_protocol_treadmill37_check_remote_controller(SubGhzBlockGeneric* instance) {
    instance->serial = instance->data >> 17;
    instance->cnt = (instance->data >> 1) & 0xFFFF;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_treadmill37_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderTreadmill37* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_treadmill37_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_treadmill37_check_remote_controller(&instance->generic);
        subghz_protocol_encoder_treadmill37_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_treadmill37_stop(void* context) {
    SubGhzProtocolEncoderTreadmill37* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_treadmill37_yield(void* context) {
    SubGhzProtocolEncoderTreadmill37* instance = context;

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

void* subghz_protocol_decoder_treadmill37_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderTreadmill37* instance = malloc(sizeof(SubGhzProtocolDecoderTreadmill37));
    instance->base.protocol = &subghz_protocol_treadmill37;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_treadmill37_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;
    free(instance);
}

void subghz_protocol_decoder_treadmill37_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;
    instance->decoder.parser_step = Treadmill37DecoderStepReset;
}

void subghz_protocol_decoder_treadmill37_feed(
    void* context,
    bool level,
    volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;

    // Treadmill37 (QH-433) Decoder
    // 2026 - @xMasterX (MMX)

    // Key samples
    //              serial                  button           stop
    // 1800001830 = 00011000000000000000000 0000110000011000 0
    // 180000061E = 00011000000000000000000 0000001100001111 0
    // 180000142C = 00011000000000000000000 0000101000010110 0
    // 1800000C24 = 00011000000000000000000 0000011000010010 0
    // 180001556C = 00011000000000000000000 1010101010110110 0
    // 180001334A = 00011000000000000000000 1001100110100101 0

    switch(instance->decoder.parser_step) {
    case Treadmill37DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_treadmill37_const.te_short * 20) <
                        subghz_protocol_treadmill37_const.te_delta * 4)) {
            //Found GAP
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = Treadmill37DecoderStepSaveDuration;
        }
        break;
    case Treadmill37DecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Treadmill37DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Treadmill37DecoderStepReset;
        }
        break;
    case Treadmill37DecoderStepCheckDuration:
        if(!level) {
            // Bit 0 is short and long timing = 300us HIGH (te_last) and 900us LOW
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_treadmill37_const.te_short) <
                subghz_protocol_treadmill37_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_treadmill37_const.te_long) <
                subghz_protocol_treadmill37_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Treadmill37DecoderStepSaveDuration;
                // Bit 1 is long and short timing = 900us HIGH (te_last) and 300us LOW
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_treadmill37_const.te_long) <
                 subghz_protocol_treadmill37_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_treadmill37_const.te_short) <
                 subghz_protocol_treadmill37_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Treadmill37DecoderStepSaveDuration;
            } else if(
                // End of the key
                DURATION_DIFF(duration, subghz_protocol_treadmill37_const.te_short * 20) <
                subghz_protocol_treadmill37_const.te_delta * 4) {
                //Found next GAP and add bit 0 or 1 (only bit 0 was found on the remotes)
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_treadmill37_const.te_short) <
                    subghz_protocol_treadmill37_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }
                if((DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_treadmill37_const.te_long) <
                    subghz_protocol_treadmill37_const.te_delta)) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                // If got 37 bits key reading is finished
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_treadmill37_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = Treadmill37DecoderStepReset;
            } else {
                instance->decoder.parser_step = Treadmill37DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Treadmill37DecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_treadmill37_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_treadmill37_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_treadmill37_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_treadmill37_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_treadmill37_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderTreadmill37* instance = context;

    subghz_protocol_treadmill37_check_remote_controller(&instance->generic);

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
        "Serial: 0x%06lX\r\n"
        "Btn: %04lX",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint64_t)(instance->generic.data & 0xFFFFFFFFFF),
        (code_found_reverse & 0xFFFFFFFFFF),
        instance->generic.serial,
        instance->generic.cnt);
}
