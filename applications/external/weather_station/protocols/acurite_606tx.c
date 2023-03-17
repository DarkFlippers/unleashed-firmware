#include "acurite_606tx.h"

#define TAG "WSProtocolAcurite_606TX"

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/5bef4e43133ac4c0e2d18d36f87c52b4f9458453/src/devices/acurite.c#L1644
 * 
 *     0000 1111 | 0011 0000 | 0101 1100 | 1110 0111
 *     iiii iiii | buuu tttt | tttt tttt | cccc cccc
 * - i: identification; changes on battery switch
 * - c: lfsr_digest8;
 * - u: unknown;
 * - b: battery low; flag to indicate low battery voltage
 * - t: Temperature; in °C
 * 
 */

static const SubGhzBlockConst ws_protocol_acurite_606tx_const = {
    .te_short = 500,
    .te_long = 2000,
    .te_delta = 150,
    .min_count_bit_for_found = 32,
};

struct WSProtocolDecoderAcurite_606TX {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;
};

struct WSProtocolEncoderAcurite_606TX {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    Acurite_606TXDecoderStepReset = 0,
    Acurite_606TXDecoderStepSaveDuration,
    Acurite_606TXDecoderStepCheckDuration,
} Acurite_606TXDecoderStep;

const SubGhzProtocolDecoder ws_protocol_acurite_606tx_decoder = {
    .alloc = ws_protocol_decoder_acurite_606tx_alloc,
    .free = ws_protocol_decoder_acurite_606tx_free,

    .feed = ws_protocol_decoder_acurite_606tx_feed,
    .reset = ws_protocol_decoder_acurite_606tx_reset,

    .get_hash_data = ws_protocol_decoder_acurite_606tx_get_hash_data,
    .serialize = ws_protocol_decoder_acurite_606tx_serialize,
    .deserialize = ws_protocol_decoder_acurite_606tx_deserialize,
    .get_string = ws_protocol_decoder_acurite_606tx_get_string,
};

const SubGhzProtocolEncoder ws_protocol_acurite_606tx_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_acurite_606tx = {
    .name = WS_PROTOCOL_ACURITE_606TX_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_acurite_606tx_decoder,
    .encoder = &ws_protocol_acurite_606tx_encoder,
};

void* ws_protocol_decoder_acurite_606tx_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderAcurite_606TX* instance = malloc(sizeof(WSProtocolDecoderAcurite_606TX));
    instance->base.protocol = &ws_protocol_acurite_606tx;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_acurite_606tx_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
    free(instance);
}

void ws_protocol_decoder_acurite_606tx_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
    instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
}

static bool ws_protocol_acurite_606tx_check(WSProtocolDecoderAcurite_606TX* instance) {
    if(!instance->decoder.decode_data) return false;
    uint8_t msg[] = {
        instance->decoder.decode_data >> 24,
        instance->decoder.decode_data >> 16,
        instance->decoder.decode_data >> 8};

    uint8_t crc = subghz_protocol_blocks_lfsr_digest8(msg, 3, 0x98, 0xF1);
    return (crc == (instance->decoder.decode_data & 0xFF));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_acurite_606tx_remote_controller(WSBlockGeneric* instance) {
    instance->id = (instance->data >> 24) & 0xFF;
    instance->battery_low = (instance->data >> 23) & 1;

    instance->channel = WS_NO_CHANNEL;

    if(!((instance->data >> 19) & 1)) {
        instance->temp = (float)((instance->data >> 8) & 0x07FF) / 10.0f;
    } else {
        instance->temp = (float)((~(instance->data >> 8) & 0x07FF) + 1) / -10.0f;
    }
    instance->btn = WS_NO_BTN;
    instance->humidity = WS_NO_HUMIDITY;
}

void ws_protocol_decoder_acurite_606tx_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;

    switch(instance->decoder.parser_step) {
    case Acurite_606TXDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, ws_protocol_acurite_606tx_const.te_short * 17) <
                        ws_protocol_acurite_606tx_const.te_delta * 8)) {
            //Found syncPrefix
            instance->decoder.parser_step = Acurite_606TXDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;

    case Acurite_606TXDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Acurite_606TXDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
        }
        break;

    case Acurite_606TXDecoderStepCheckDuration:
        if(!level) {
            if(DURATION_DIFF(instance->decoder.te_last, ws_protocol_acurite_606tx_const.te_short) <
               ws_protocol_acurite_606tx_const.te_delta) {
                if((DURATION_DIFF(duration, ws_protocol_acurite_606tx_const.te_short) <
                    ws_protocol_acurite_606tx_const.te_delta) ||
                   (duration > ws_protocol_acurite_606tx_const.te_long * 3)) {
                    //Found syncPostfix
                    instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
                    if((instance->decoder.decode_count_bit ==
                        ws_protocol_acurite_606tx_const.min_count_bit_for_found) &&
                       ws_protocol_acurite_606tx_check(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        ws_protocol_acurite_606tx_remote_controller(&instance->generic);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                } else if(
                    DURATION_DIFF(duration, ws_protocol_acurite_606tx_const.te_long) <
                    ws_protocol_acurite_606tx_const.te_delta * 2) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                    instance->decoder.parser_step = Acurite_606TXDecoderStepSaveDuration;
                } else if(
                    DURATION_DIFF(duration, ws_protocol_acurite_606tx_const.te_long * 2) <
                    ws_protocol_acurite_606tx_const.te_delta * 4) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                    instance->decoder.parser_step = Acurite_606TXDecoderStepSaveDuration;
                } else {
                    instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Acurite_606TXDecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_acurite_606tx_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_acurite_606tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    ws_protocol_decoder_acurite_606tx_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        ws_protocol_acurite_606tx_const.min_count_bit_for_found);
}

void ws_protocol_decoder_acurite_606tx_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderAcurite_606TX* instance = context;
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
