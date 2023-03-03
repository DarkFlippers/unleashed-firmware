#include "acurite_609txc.h"

#define TAG "WSProtocolAcurite_609TXC"

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/5bef4e43133ac4c0e2d18d36f87c52b4f9458453/src/devices/acurite.c#L216
 *
 *     0000 1111 | 0011 0000 | 0101 1100 | 0000 0000 | 1110 0111
 *     iiii iiii | buuu tttt | tttt tttt | hhhh hhhh | cccc cccc
 * - i: identification; changes on battery switch
 * - c: checksum (sum of previous by bytes)
 * - u: unknown
 * - b: battery low; flag to indicate low battery voltage
 * - t: temperature; in °C * 10, 12 bit with complement
 * - h: humidity
 *
 */

static const SubGhzBlockConst ws_protocol_acurite_609txc_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 150,
    .min_count_bit_for_found = 40,
};

struct WSProtocolDecoderAcurite_609TXC {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;
};

struct WSProtocolEncoderAcurite_609TXC {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    Acurite_609TXCDecoderStepReset = 0,
    Acurite_609TXCDecoderStepSaveDuration,
    Acurite_609TXCDecoderStepCheckDuration,
} Acurite_609TXCDecoderStep;

const SubGhzProtocolDecoder ws_protocol_acurite_609txc_decoder = {
    .alloc = ws_protocol_decoder_acurite_609txc_alloc,
    .free = ws_protocol_decoder_acurite_609txc_free,

    .feed = ws_protocol_decoder_acurite_609txc_feed,
    .reset = ws_protocol_decoder_acurite_609txc_reset,

    .get_hash_data = ws_protocol_decoder_acurite_609txc_get_hash_data,
    .serialize = ws_protocol_decoder_acurite_609txc_serialize,
    .deserialize = ws_protocol_decoder_acurite_609txc_deserialize,
    .get_string = ws_protocol_decoder_acurite_609txc_get_string,
};

const SubGhzProtocolEncoder ws_protocol_acurite_609txc_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_acurite_609txc = {
    .name = WS_PROTOCOL_ACURITE_609TXC_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_acurite_609txc_decoder,
    .encoder = &ws_protocol_acurite_609txc_encoder,
};

void* ws_protocol_decoder_acurite_609txc_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderAcurite_609TXC* instance = malloc(sizeof(WSProtocolDecoderAcurite_609TXC));
    instance->base.protocol = &ws_protocol_acurite_609txc;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_acurite_609txc_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    free(instance);
}

void ws_protocol_decoder_acurite_609txc_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
}

static bool ws_protocol_acurite_609txc_check(WSProtocolDecoderAcurite_609TXC* instance) {
    if(!instance->decoder.decode_data) return false;
    uint8_t crc = (uint8_t)(instance->decoder.decode_data >> 32) +
                  (uint8_t)(instance->decoder.decode_data >> 24) +
                  (uint8_t)(instance->decoder.decode_data >> 16) +
                  (uint8_t)(instance->decoder.decode_data >> 8);
    return (crc == (instance->decoder.decode_data & 0xFF));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_acurite_609txc_remote_controller(WSBlockGeneric* instance) {
    instance->id = (instance->data >> 32) & 0xFF;
    instance->battery_low = (instance->data >> 31) & 1;

    instance->channel = WS_NO_CHANNEL;

    // Temperature in Celsius is encoded as a 12 bit integer value
    // multiplied by 10 using the 4th - 6th nybbles (bytes 1 & 2)
    // negative values are recovered by sign extend from int16_t.
    int16_t temp_raw =
        (int16_t)(((instance->data >> 12) & 0xf000) | ((instance->data >> 16) << 4));
    instance->temp = (temp_raw >> 4) * 0.1f;
    instance->humidity = (instance->data >> 8) & 0xff;
    instance->btn = WS_NO_BTN;
}

void ws_protocol_decoder_acurite_609txc_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;

    switch(instance->decoder.parser_step) {
    case Acurite_609TXCDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, ws_protocol_acurite_609txc_const.te_short * 17) <
                        ws_protocol_acurite_609txc_const.te_delta * 8)) {
            //Found syncPrefix
            instance->decoder.parser_step = Acurite_609TXCDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;

    case Acurite_609TXCDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Acurite_609TXCDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
        }
        break;

    case Acurite_609TXCDecoderStepCheckDuration:
        if(!level) {
            if(DURATION_DIFF(instance->decoder.te_last, ws_protocol_acurite_609txc_const.te_short) <
               ws_protocol_acurite_609txc_const.te_delta) {
                if((DURATION_DIFF(duration, ws_protocol_acurite_609txc_const.te_short) <
                    ws_protocol_acurite_609txc_const.te_delta) ||
                   (duration > ws_protocol_acurite_609txc_const.te_long * 3)) {
                    //Found syncPostfix
                    instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
                    if((instance->decoder.decode_count_bit ==
                        ws_protocol_acurite_609txc_const.min_count_bit_for_found) &&
                       ws_protocol_acurite_609txc_check(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        ws_protocol_acurite_609txc_remote_controller(&instance->generic);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                } else if(
                    DURATION_DIFF(duration, ws_protocol_acurite_609txc_const.te_long) <
                    ws_protocol_acurite_609txc_const.te_delta * 2) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                    instance->decoder.parser_step = Acurite_609TXCDecoderStepSaveDuration;
                } else if(
                    DURATION_DIFF(duration, ws_protocol_acurite_609txc_const.te_long * 2) <
                    ws_protocol_acurite_609txc_const.te_delta * 4) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                    instance->decoder.parser_step = Acurite_609TXCDecoderStepSaveDuration;
                } else {
                    instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Acurite_609TXCDecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_acurite_609txc_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_acurite_609txc_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    ws_protocol_decoder_acurite_609txc_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        ws_protocol_acurite_609txc_const.min_count_bit_for_found);
}

void ws_protocol_decoder_acurite_609txc_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderAcurite_609TXC* instance = context;
    furi_string_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%lX Ch:%d  Bat:%d\r\n"
        "Temp:%3.1f C Hum:%d%%",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 40),
        (uint32_t)(instance->generic.data),
        instance->generic.id,
        instance->generic.channel,
        instance->generic.battery_low,
        (double)instance->generic.temp,
        instance->generic.humidity);
}
