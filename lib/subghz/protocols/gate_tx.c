#include "gate_tx.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolGateTx"

static const SubGhzBlockConst subghz_protocol_gate_tx_const = {
    .te_short = 350,
    .te_long = 700,
    .te_delta = 100,
    .min_count_bit_for_found = 24,
};

struct SubGhzProtocolDecoderGateTx {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderGateTx {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    GateTXDecoderStepReset = 0,
    GateTXDecoderStepFoundStartBit,
    GateTXDecoderStepSaveDuration,
    GateTXDecoderStepCheckDuration,
} GateTXDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_gate_tx_decoder = {
    .alloc = subghz_protocol_decoder_gate_tx_alloc,
    .free = subghz_protocol_decoder_gate_tx_free,

    .feed = subghz_protocol_decoder_gate_tx_feed,
    .reset = subghz_protocol_decoder_gate_tx_reset,

    .get_hash_data = subghz_protocol_decoder_gate_tx_get_hash_data,
    .serialize = subghz_protocol_decoder_gate_tx_serialize,
    .deserialize = subghz_protocol_decoder_gate_tx_deserialize,
    .get_string = subghz_protocol_decoder_gate_tx_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_gate_tx_encoder = {
    .alloc = subghz_protocol_encoder_gate_tx_alloc,
    .free = subghz_protocol_encoder_gate_tx_free,

    .deserialize = subghz_protocol_encoder_gate_tx_deserialize,
    .stop = subghz_protocol_encoder_gate_tx_stop,
    .yield = subghz_protocol_encoder_gate_tx_yield,
};

const SubGhzProtocol subghz_protocol_gate_tx = {
    .name = SUBGHZ_PROTOCOL_GATE_TX_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_gate_tx_decoder,
    .encoder = &subghz_protocol_gate_tx_encoder,
};

void* subghz_protocol_encoder_gate_tx_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderGateTx* instance = malloc(sizeof(SubGhzProtocolEncoderGateTx));

    instance->base.protocol = &subghz_protocol_gate_tx;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 52; //max 24bit*2 + 2 (start, stop)
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_gate_tx_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGateTx* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderGateTx instance
 * @return true On success
 */
static bool subghz_protocol_encoder_gate_tx_get_upload(SubGhzProtocolEncoderGateTx* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_gate_tx_const.te_short * 49);
    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_gate_tx_const.te_long);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_gate_tx_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gate_tx_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_gate_tx_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_gate_tx_const.te_long);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_gate_tx_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderGateTx* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_gate_tx_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_gate_tx_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_gate_tx_stop(void* context) {
    SubGhzProtocolEncoderGateTx* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_gate_tx_yield(void* context) {
    SubGhzProtocolEncoderGateTx* instance = context;

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

void* subghz_protocol_decoder_gate_tx_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderGateTx* instance = malloc(sizeof(SubGhzProtocolDecoderGateTx));
    instance->base.protocol = &subghz_protocol_gate_tx;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_gate_tx_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    free(instance);
}

void subghz_protocol_decoder_gate_tx_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    instance->decoder.parser_step = GateTXDecoderStepReset;
}

void subghz_protocol_decoder_gate_tx_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;

    switch(instance->decoder.parser_step) {
    case GateTXDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_gate_tx_const.te_short * 47) <
                        subghz_protocol_gate_tx_const.te_delta * 47)) {
            //Found Preambula
            instance->decoder.parser_step = GateTXDecoderStepFoundStartBit;
        }
        break;
    case GateTXDecoderStepFoundStartBit:
        if(level && ((DURATION_DIFF(duration, subghz_protocol_gate_tx_const.te_long) <
                      subghz_protocol_gate_tx_const.te_delta * 3))) {
            //Found start bit
            instance->decoder.parser_step = GateTXDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = GateTXDecoderStepReset;
        }
        break;
    case GateTXDecoderStepSaveDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_gate_tx_const.te_short * 10 +
                            subghz_protocol_gate_tx_const.te_delta)) {
                instance->decoder.parser_step = GateTXDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_gate_tx_const.min_count_bit_for_found) {
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
                instance->decoder.parser_step = GateTXDecoderStepCheckDuration;
            }
        }
        break;
    case GateTXDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gate_tx_const.te_short) <
                subghz_protocol_gate_tx_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_gate_tx_const.te_long) <
                subghz_protocol_gate_tx_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = GateTXDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_gate_tx_const.te_long) <
                 subghz_protocol_gate_tx_const.te_delta * 3) &&
                (DURATION_DIFF(duration, subghz_protocol_gate_tx_const.te_short) <
                 subghz_protocol_gate_tx_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = GateTXDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = GateTXDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = GateTXDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_gate_tx_check_remote_controller(SubGhzBlockGeneric* instance) {
    uint32_t code_found_reverse =
        subghz_protocol_blocks_reverse_key(instance->data, instance->data_count_bit);

    instance->serial = (code_found_reverse & 0xFF) << 12 |
                       ((code_found_reverse >> 8) & 0xFF) << 4 |
                       ((code_found_reverse >> 20) & 0x0F);
    instance->btn = ((code_found_reverse >> 16) & 0x0F);
}

uint8_t subghz_protocol_decoder_gate_tx_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_gate_tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_gate_tx_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, subghz_protocol_gate_tx_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_gate_tx_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderGateTx* instance = context;
    subghz_protocol_gate_tx_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%06lX\r\n"
        "Sn:%05lX  Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
