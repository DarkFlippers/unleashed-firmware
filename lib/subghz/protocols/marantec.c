#include "marantec.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolMarantec"

static const SubGhzBlockConst subghz_protocol_marantec_const = {
    .te_short = 1000,
    .te_long = 2000,
    .te_delta = 200,
    .min_count_bit_for_found = 49,
};

struct SubGhzProtocolDecoderMarantec {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderMarantec {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    MarantecDecoderStepReset = 0,
    MarantecDecoderFoundHeader,
    MarantecDecoderStepDecoderData,
} MarantecDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_marantec_decoder = {
    .alloc = subghz_protocol_decoder_marantec_alloc,
    .free = subghz_protocol_decoder_marantec_free,

    .feed = subghz_protocol_decoder_marantec_feed,
    .reset = subghz_protocol_decoder_marantec_reset,

    .get_hash_data = subghz_protocol_decoder_marantec_get_hash_data,
    .serialize = subghz_protocol_decoder_marantec_serialize,
    .deserialize = subghz_protocol_decoder_marantec_deserialize,
    .get_string = subghz_protocol_decoder_marantec_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_marantec_encoder = {
    .alloc = subghz_protocol_encoder_marantec_alloc,
    .free = subghz_protocol_encoder_marantec_free,

    .deserialize = subghz_protocol_encoder_marantec_deserialize,
    .stop = subghz_protocol_encoder_marantec_stop,
    .yield = subghz_protocol_encoder_marantec_yield,
};

const SubGhzProtocol subghz_protocol_marantec = {
    .name = SUBGHZ_PROTOCOL_MARANTEC_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_marantec_decoder,
    .encoder = &subghz_protocol_marantec_encoder,
};

void* subghz_protocol_encoder_marantec_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderMarantec* instance = malloc(sizeof(SubGhzProtocolEncoderMarantec));

    instance->base.protocol = &subghz_protocol_marantec;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_marantec_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderMarantec* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static LevelDuration
    subghz_protocol_encoder_marantec_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_marantec_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_marantec_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_marantec_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_marantec_const.te_short;
        data.level = true;
        break;

    default:
        furi_crash("SubGhz: ManchesterEncoderResult is incorrect.");
        break;
    }
    return level_duration_make(data.level, data.duration);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderMarantec instance
 */
static void subghz_protocol_encoder_marantec_get_upload(SubGhzProtocolEncoderMarantec* instance) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    if(!manchester_encoder_advance(
           &enc_state,
           bit_read(instance->generic.data, instance->generic.data_count_bit - 1),
           &result)) {
        instance->encoder.upload[index++] =
            subghz_protocol_encoder_marantec_add_duration_to_upload(result);
        manchester_encoder_advance(
            &enc_state,
            bit_read(instance->generic.data, instance->generic.data_count_bit - 1),
            &result);
    }
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_marantec_const.te_long * 5);

    for(uint8_t i = instance->generic.data_count_bit - 1; i > 0; i--) {
        if(!manchester_encoder_advance(
               &enc_state, bit_read(instance->generic.data, i - 1), &result)) {
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_marantec_add_duration_to_upload(result);
            manchester_encoder_advance(
                &enc_state, bit_read(instance->generic.data, i - 1), &result);
        }
        instance->encoder.upload[index++] =
            subghz_protocol_encoder_marantec_add_duration_to_upload(result);
    }
    instance->encoder.upload[index] = subghz_protocol_encoder_marantec_add_duration_to_upload(
        manchester_encoder_finish(&enc_state));
    if(level_duration_get_level(instance->encoder.upload[index])) {
        index++;
    }
    instance->encoder.size_upload = index;
}

uint8_t subghz_protocol_marantec_crc8(uint8_t* data, size_t len) {
    uint8_t crc = 0x08;
    size_t i, j;
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x1D);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_marantec_remote_controller(SubGhzBlockGeneric* instance) {
    instance->btn = (instance->data >> 16) & 0xF;
    instance->serial = ((instance->data >> 12) & 0xFFFFFF00) | ((instance->data >> 8) & 0xFF);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_marantec_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderMarantec* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_marantec_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_marantec_remote_controller(&instance->generic);
        subghz_protocol_encoder_marantec_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_marantec_stop(void* context) {
    SubGhzProtocolEncoderMarantec* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_marantec_yield(void* context) {
    SubGhzProtocolEncoderMarantec* instance = context;

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

void* subghz_protocol_decoder_marantec_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderMarantec* instance = malloc(sizeof(SubGhzProtocolDecoderMarantec));
    instance->base.protocol = &subghz_protocol_marantec;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_marantec_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    free(instance);
}

void subghz_protocol_decoder_marantec_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_decoder_marantec_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    ManchesterEvent event = ManchesterEventReset;

    switch(instance->decoder.parser_step) {
    case MarantecDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_marantec_const.te_long * 5) <
                        subghz_protocol_marantec_const.te_delta * 8)) {
            //Found header marantec
            instance->decoder.parser_step = MarantecDecoderStepDecoderData;
            instance->decoder.decode_data = 1;
            instance->decoder.decode_count_bit = 1;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case MarantecDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_marantec_const.te_short) <
               subghz_protocol_marantec_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_marantec_const.te_long) <
                subghz_protocol_marantec_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= ((uint32_t)subghz_protocol_marantec_const.te_long * 2 +
                             subghz_protocol_marantec_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_marantec_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 1;
                instance->decoder.decode_count_bit = 1;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = MarantecDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_marantec_const.te_short) <
               subghz_protocol_marantec_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_marantec_const.te_long) <
                subghz_protocol_marantec_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = MarantecDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_marantec_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_marantec_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_marantec_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_marantec_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_marantec_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderMarantec* instance = context;
    subghz_protocol_marantec_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%07lX \r\n"
        "Btn:%X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->generic.btn);
}
