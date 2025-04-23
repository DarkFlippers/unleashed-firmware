#include "revers_rb2.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolRevers_RB2"

static const SubGhzBlockConst subghz_protocol_revers_rb2_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 160,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderRevers_RB2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderRevers_RB2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Revers_RB2DecoderStepReset = 0,
    Revers_RB2DecoderStepHeader,
    Revers_RB2DecoderStepDecoderData,
} Revers_RB2DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_revers_rb2_decoder = {
    .alloc = subghz_protocol_decoder_revers_rb2_alloc,
    .free = subghz_protocol_decoder_revers_rb2_free,

    .feed = subghz_protocol_decoder_revers_rb2_feed,
    .reset = subghz_protocol_decoder_revers_rb2_reset,

    .get_hash_data = subghz_protocol_decoder_revers_rb2_get_hash_data,
    .serialize = subghz_protocol_decoder_revers_rb2_serialize,
    .deserialize = subghz_protocol_decoder_revers_rb2_deserialize,
    .get_string = subghz_protocol_decoder_revers_rb2_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_revers_rb2_encoder = {
    .alloc = subghz_protocol_encoder_revers_rb2_alloc,
    .free = subghz_protocol_encoder_revers_rb2_free,

    .deserialize = subghz_protocol_encoder_revers_rb2_deserialize,
    .stop = subghz_protocol_encoder_revers_rb2_stop,
    .yield = subghz_protocol_encoder_revers_rb2_yield,
};

const SubGhzProtocol subghz_protocol_revers_rb2 = {
    .name = SUBGHZ_PROTOCOL_REVERSRB2_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_revers_rb2_decoder,
    .encoder = &subghz_protocol_revers_rb2_encoder,
};

void* subghz_protocol_encoder_revers_rb2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderRevers_RB2* instance = malloc(sizeof(SubGhzProtocolEncoderRevers_RB2));

    instance->base.protocol = &subghz_protocol_revers_rb2;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_revers_rb2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderRevers_RB2* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static LevelDuration
    subghz_protocol_encoder_revers_rb2_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_revers_rb2_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_revers_rb2_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_revers_rb2_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_revers_rb2_const.te_short;
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
 * @param instance Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 */
static void
    subghz_protocol_encoder_revers_rb2_get_upload(SubGhzProtocolEncoderRevers_RB2* instance) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(!manchester_encoder_advance(
               &enc_state, bit_read(instance->generic.data, i - 1), &result)) {
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_revers_rb2_add_duration_to_upload(result);
            manchester_encoder_advance(
                &enc_state, bit_read(instance->generic.data, i - 1), &result);
        }
        instance->encoder.upload[index++] =
            subghz_protocol_encoder_revers_rb2_add_duration_to_upload(result);
    }
    instance->encoder.upload[index] = subghz_protocol_encoder_revers_rb2_add_duration_to_upload(
        manchester_encoder_finish(&enc_state));
    if(level_duration_get_level(instance->encoder.upload[index])) {
        index++;
    }
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)320);
    instance->encoder.size_upload = index;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_revers_rb2_remote_controller(SubGhzBlockGeneric* instance) {
    // Revers RB2 / RB2M Decoder
    // 02.2025 - @xMasterX (MMX)
    instance->serial = (((instance->data << 16) >> 16) >> 10);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderRevers_RB2* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_revers_rb2_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_revers_rb2_remote_controller(&instance->generic);
        subghz_protocol_encoder_revers_rb2_get_upload(instance);
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_revers_rb2_stop(void* context) {
    SubGhzProtocolEncoderRevers_RB2* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_revers_rb2_yield(void* context) {
    SubGhzProtocolEncoderRevers_RB2* instance = context;

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

void* subghz_protocol_decoder_revers_rb2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderRevers_RB2* instance = malloc(sizeof(SubGhzProtocolDecoderRevers_RB2));
    instance->base.protocol = &subghz_protocol_revers_rb2;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_revers_rb2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    free(instance);
}

void subghz_protocol_decoder_revers_rb2_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_decoder_revers_rb2_addbit(void* context, bool data) {
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
    instance->decoder.decode_count_bit++;

    if(instance->decoder.decode_count_bit >= 65) {
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        return;
    }

    if(instance->decoder.decode_count_bit <
       subghz_protocol_revers_rb2_const.min_count_bit_for_found) {
        return;
    }

    // Revers RB2 / RB2M Decoder
    // 02.2025 - @xMasterX (MMX)

    uint16_t preamble = (instance->decoder.decode_data >> 48) & 0xFF;
    uint16_t stop_code = (instance->decoder.decode_data & 0x3FF);

    if(preamble == 0xFF && stop_code == 0x200) {
        //Found header and stop code
        instance->generic.data = instance->decoder.decode_data;
        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

        if(instance->base.callback)
            instance->base.callback(&instance->base, instance->base.context);

        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        manchester_advance(
            instance->manchester_saved_state,
            ManchesterEventReset,
            &instance->manchester_saved_state,
            NULL);
    }
}

void subghz_protocol_decoder_revers_rb2_feed(void* context, bool level, volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    ManchesterEvent event = ManchesterEventReset;

    switch(instance->decoder.parser_step) {
    case Revers_RB2DecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, 600) < subghz_protocol_revers_rb2_const.te_delta)) {
            instance->decoder.parser_step = Revers_RB2DecoderStepHeader;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case Revers_RB2DecoderStepHeader:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_short) <
               subghz_protocol_revers_rb2_const.te_delta) {
                if(instance->decoder.te_last == 1) {
                    instance->header_count++;
                }
                instance->decoder.te_last = level;
            } else {
                instance->header_count = 0;
                instance->decoder.te_last = 0;
                instance->decoder.parser_step = Revers_RB2DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_short) <
               subghz_protocol_revers_rb2_const.te_delta) {
                if(instance->decoder.te_last == 0) {
                    instance->header_count++;
                }
                instance->decoder.te_last = level;
            } else {
                instance->header_count = 0;
                instance->decoder.te_last = 0;
                instance->decoder.parser_step = Revers_RB2DecoderStepReset;
            }
        }

        if(instance->header_count == 4) {
            instance->header_count = 0;
            instance->decoder.decode_data = 0xF;
            instance->decoder.decode_count_bit = 4;
            instance->decoder.parser_step = Revers_RB2DecoderStepDecoderData;
        }
        break;
    case Revers_RB2DecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_short) <
               subghz_protocol_revers_rb2_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_long) <
                subghz_protocol_revers_rb2_const.te_delta) {
                event = ManchesterEventLongLow;
            } else {
                instance->decoder.parser_step = Revers_RB2DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_short) <
               subghz_protocol_revers_rb2_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_revers_rb2_const.te_long) <
                subghz_protocol_revers_rb2_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = Revers_RB2DecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                subghz_protocol_decoder_revers_rb2_addbit(instance, data);
            }
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_revers_rb2_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_revers_rb2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_revers_rb2_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_revers_rb2_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderRevers_RB2* instance = context;
    subghz_protocol_revers_rb2_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:%lX%08lX\r\n"
        "Sn:0x%08lX \r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial);
}
