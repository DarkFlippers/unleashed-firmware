#include "wendox.h"

#define TAG "WSProtocolWendox"

static const SubGhzBlockConst ws_protocol_wendox_const = {
    .te_short = 1955,
    .te_long = 5865,
    .te_delta = 300,
    .min_count_bit_for_found = 28,
};

struct WSProtocolDecoderWendox {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;

    uint16_t header_count;
};

struct WSProtocolEncoderWendox {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    WendoxDecoderStepReset = 0,
    WendoxDecoderStepCheckPreambule,
    WendoxDecoderStepSaveDuration,
    WendoxDecoderStepCheckDuration,
} WendoxDecoderStep;

const SubGhzProtocolDecoder ws_protocol_wendox_decoder = {
    .alloc = ws_protocol_decoder_wendox_alloc,
    .free = ws_protocol_decoder_wendox_free,

    .feed = ws_protocol_decoder_wendox_feed,
    .reset = ws_protocol_decoder_wendox_reset,

    .get_hash_data = ws_protocol_decoder_wendox_get_hash_data,
    .serialize = ws_protocol_decoder_wendox_serialize,
    .deserialize = ws_protocol_decoder_wendox_deserialize,
    .get_string = ws_protocol_decoder_wendox_get_string,
};

const SubGhzProtocolEncoder ws_protocol_wendox_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_wendox = {
    .name = WS_PROTOCOL_WENDOX_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_wendox_decoder,
    .encoder = &ws_protocol_wendox_encoder,
};

void* ws_protocol_decoder_wendox_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderWendox* instance = malloc(sizeof(WSProtocolDecoderWendox));
    instance->base.protocol = &ws_protocol_wendox;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_wendox_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    free(instance);
}

void ws_protocol_decoder_wendox_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    instance->decoder.parser_step = WendoxDecoderStepReset;
}

// static bool ws_protocol_wendox_check(WSProtocolDecoderWendox* instance) {
//     if(!instance->decoder.decode_data) return false;
//     uint8_t msg[] = {
//         instance->decoder.decode_data >> 24,
//         instance->decoder.decode_data >> 16,
//         instance->decoder.decode_data >> 8};

//     uint8_t crc = subghz_protocol_blocks_lfsr_digest8(msg, 3, 0x98, 0xF1);
//     return (crc == (instance->decoder.decode_data & 0xFF));
// }

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_wendox_remote_controller(WSBlockGeneric* instance) {
    instance->id = (instance->data >> 4) & 0xFF;
    instance->battery_low = WS_NO_BATT;
    instance->channel = WS_NO_CHANNEL;

    if(((instance->data >> 22) & 1)) {
        instance->temp = (float)(((instance->data >> 13) & 0x1FF) + 12) / 10.0f;
    } else {
        instance->temp = (float)((~(instance->data >> 13) & 0x1FF) + 1 - 12) / -10.0f;
    }
    FURI_LOG_E("TAG", "%llX %f", instance->data, (double)instance->temp);

    instance->btn = WS_NO_BTN;
    instance->humidity = WS_NO_HUMIDITY;
}

void ws_protocol_decoder_wendox_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;

    switch(instance->decoder.parser_step) {
    case WendoxDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, ws_protocol_wendox_const.te_short) <
                       ws_protocol_wendox_const.te_delta)) {
            instance->decoder.parser_step = WendoxDecoderStepCheckPreambule;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;

    case WendoxDecoderStepCheckPreambule:
        if(level) {
            instance->decoder.te_last = duration;
        } else {
            if((DURATION_DIFF(instance->decoder.te_last, ws_protocol_wendox_const.te_short) <
                ws_protocol_wendox_const.te_delta * 1) &&
               (DURATION_DIFF(duration, ws_protocol_wendox_const.te_long) <
                ws_protocol_wendox_const.te_delta * 2)) {
                instance->header_count++;
            } else if((instance->header_count > 4) && (instance->header_count < 12)) {
                if((DURATION_DIFF(instance->decoder.te_last, ws_protocol_wendox_const.te_long) <
                    ws_protocol_wendox_const.te_delta * 2) &&
                   (DURATION_DIFF(duration, ws_protocol_wendox_const.te_short) <
                    ws_protocol_wendox_const.te_delta)) {
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                    instance->decoder.parser_step = WendoxDecoderStepSaveDuration;
                } else {
                    instance->decoder.parser_step = WendoxDecoderStepReset;
                }

            } else {
                instance->decoder.parser_step = WendoxDecoderStepReset;
            }
        }
        break;

    case WendoxDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = WendoxDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = WendoxDecoderStepReset;
        }
        break;

    case WendoxDecoderStepCheckDuration:
        if(!level) {
            if(duration > ws_protocol_wendox_const.te_short + ws_protocol_wendox_const.te_long) {
                if(instance->decoder.decode_count_bit ==
                   ws_protocol_wendox_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    ws_protocol_wendox_remote_controller(&instance->generic);
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }

                instance->decoder.parser_step = WendoxDecoderStepReset;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_wendox_const.te_short) <
                 ws_protocol_wendox_const.te_delta) &&
                (DURATION_DIFF(duration, ws_protocol_wendox_const.te_long) <
                 ws_protocol_wendox_const.te_delta * 3)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = WendoxDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_wendox_const.te_long) <
                 ws_protocol_wendox_const.te_delta * 2) &&
                (DURATION_DIFF(duration, ws_protocol_wendox_const.te_short) <
                 ws_protocol_wendox_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = WendoxDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = WendoxDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = WendoxDecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_wendox_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_wendox_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    ws_protocol_decoder_wendox_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, ws_protocol_wendox_const.min_count_bit_for_found);
}

void ws_protocol_decoder_wendox_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderWendox* instance = context;
    furi_string_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%lX Ch:%d  Bat:%d\r\n"
        "Temp:%3.1f C Hum:%d%%",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data),
        instance->generic.id,
        instance->generic.channel,
        instance->generic.battery_low,
        (double)instance->generic.temp,
        instance->generic.humidity);
}
