#include "acurite_592txr.h"

#define TAG "WSProtocolAcurite_592TXR"

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/acurite.c
 * 
 * Acurite 592TXR Temperature Humidity sensor decoder
 * Message Type 0x04, 7 bytes
 * | Byte 0    | Byte 1    | Byte 2    | Byte 3    | Byte 4    | Byte 5    | Byte 6    |
 * | --------- | --------- | --------- | --------- | --------- | --------- | --------- |
 * | CCII IIII | IIII IIII | pB00 0100 | pHHH HHHH | p??T TTTT | pTTT TTTT | KKKK KKKK |
 * - C: Channel 00: C, 10: B, 11: A, (01 is invalid)
 * - I: Device ID (14 bits)
 * - B: Battery, 1 is battery OK, 0 is battery low
 * - M: Message type (6 bits), 0x04
 * - T: Temperature Celsius (11 - 14 bits?), + 1000 * 10
 * - H: Relative Humidity (%) (7 bits)
 * - K: Checksum (8 bits)
 * - p: Parity bit
 * Notes:
 * - Temperature
 *   - Encoded as Celsius + 1000 * 10
 *   - only 11 bits needed for specified range -40 C to 70 C (-40 F - 158 F)
 *   - However 14 bits available for temperature, giving possible range of -100 C to 1538.4 C
 *   - @todo - check if high 3 bits ever used for anything else
 * 
 */

static const SubGhzBlockConst ws_protocol_acurite_592txr_const = {
    .te_short = 200,
    .te_long = 400,
    .te_delta = 90,
    .min_count_bit_for_found = 56,
};

struct WSProtocolDecoderAcurite_592TXR {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;

    uint16_t header_count;
};

struct WSProtocolEncoderAcurite_592TXR {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    Acurite_592TXRDecoderStepReset = 0,
    Acurite_592TXRDecoderStepCheckPreambule,
    Acurite_592TXRDecoderStepSaveDuration,
    Acurite_592TXRDecoderStepCheckDuration,
} Acurite_592TXRDecoderStep;

const SubGhzProtocolDecoder ws_protocol_acurite_592txr_decoder = {
    .alloc = ws_protocol_decoder_acurite_592txr_alloc,
    .free = ws_protocol_decoder_acurite_592txr_free,

    .feed = ws_protocol_decoder_acurite_592txr_feed,
    .reset = ws_protocol_decoder_acurite_592txr_reset,

    .get_hash_data = ws_protocol_decoder_acurite_592txr_get_hash_data,
    .serialize = ws_protocol_decoder_acurite_592txr_serialize,
    .deserialize = ws_protocol_decoder_acurite_592txr_deserialize,
    .get_string = ws_protocol_decoder_acurite_592txr_get_string,
};

const SubGhzProtocolEncoder ws_protocol_acurite_592txr_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_acurite_592txr = {
    .name = WS_PROTOCOL_ACURITE_592TXR_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_acurite_592txr_decoder,
    .encoder = &ws_protocol_acurite_592txr_encoder,
};

void* ws_protocol_decoder_acurite_592txr_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderAcurite_592TXR* instance = malloc(sizeof(WSProtocolDecoderAcurite_592TXR));
    instance->base.protocol = &ws_protocol_acurite_592txr;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_acurite_592txr_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
    free(instance);
}

void ws_protocol_decoder_acurite_592txr_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
    instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
}

static bool ws_protocol_acurite_592txr_check_crc(WSProtocolDecoderAcurite_592TXR* instance) {
    uint8_t msg[] = {
        instance->decoder.decode_data >> 48,
        instance->decoder.decode_data >> 40,
        instance->decoder.decode_data >> 32,
        instance->decoder.decode_data >> 24,
        instance->decoder.decode_data >> 16,
        instance->decoder.decode_data >> 8};

    if((subghz_protocol_blocks_add_bytes(msg, 6) ==
        (uint8_t)(instance->decoder.decode_data & 0xFF)) &&
       (!subghz_protocol_blocks_parity_bytes(&msg[2], 4))) {
        return true;
    } else {
        return false;
    }
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_acurite_592txr_remote_controller(WSBlockGeneric* instance) {
    uint8_t channel[] = {3, 0, 2, 1};
    uint8_t channel_raw = ((instance->data >> 54) & 0x03);
    instance->channel = channel[channel_raw];
    instance->id = (instance->data >> 40) & 0x3FFF;
    instance->battery_low = !((instance->data >> 38) & 1);
    instance->humidity = (instance->data >> 24) & 0x7F;

    uint16_t temp_raw = ((instance->data >> 9) & 0xF80) | ((instance->data >> 8) & 0x7F);
    instance->temp = ((float)(temp_raw)-1000) / 10.0f;

    instance->btn = WS_NO_BTN;
}

void ws_protocol_decoder_acurite_592txr_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;

    switch(instance->decoder.parser_step) {
    case Acurite_592TXRDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_short * 3) <
                       ws_protocol_acurite_592txr_const.te_delta * 2)) {
            instance->decoder.parser_step = Acurite_592TXRDecoderStepCheckPreambule;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;

    case Acurite_592TXRDecoderStepCheckPreambule:
        if(level) {
            instance->decoder.te_last = duration;
        } else {
            if((DURATION_DIFF(
                    instance->decoder.te_last, ws_protocol_acurite_592txr_const.te_short * 3) <
                ws_protocol_acurite_592txr_const.te_delta * 2) &&
               (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_short * 3) <
                ws_protocol_acurite_592txr_const.te_delta * 2)) {
                //Found preambule
                instance->header_count++;
            } else if((instance->header_count > 2) && (instance->header_count < 5)) {
                if((DURATION_DIFF(
                        instance->decoder.te_last, ws_protocol_acurite_592txr_const.te_short) <
                    ws_protocol_acurite_592txr_const.te_delta) &&
                   (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_long) <
                    ws_protocol_acurite_592txr_const.te_delta)) {
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                    instance->decoder.parser_step = Acurite_592TXRDecoderStepSaveDuration;
                } else if(
                    (DURATION_DIFF(
                         instance->decoder.te_last, ws_protocol_acurite_592txr_const.te_long) <
                     ws_protocol_acurite_592txr_const.te_delta) &&
                    (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_short) <
                     ws_protocol_acurite_592txr_const.te_delta)) {
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                    instance->decoder.parser_step = Acurite_592TXRDecoderStepSaveDuration;
                } else {
                    instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
            }
        }
        break;

    case Acurite_592TXRDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Acurite_592TXRDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
        }
        break;

    case Acurite_592TXRDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)ws_protocol_acurite_592txr_const.te_short * 5)) {
                if((instance->decoder.decode_count_bit ==
                    ws_protocol_acurite_592txr_const.min_count_bit_for_found) &&
                   ws_protocol_acurite_592txr_check_crc(instance)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    ws_protocol_acurite_592txr_remote_controller(&instance->generic);
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
                break;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, ws_protocol_acurite_592txr_const.te_short) <
                 ws_protocol_acurite_592txr_const.te_delta) &&
                (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_long) <
                 ws_protocol_acurite_592txr_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Acurite_592TXRDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, ws_protocol_acurite_592txr_const.te_long) <
                 ws_protocol_acurite_592txr_const.te_delta) &&
                (DURATION_DIFF(duration, ws_protocol_acurite_592txr_const.te_short) <
                 ws_protocol_acurite_592txr_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Acurite_592TXRDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = Acurite_592TXRDecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_acurite_592txr_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_acurite_592txr_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    ws_protocol_decoder_acurite_592txr_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        ws_protocol_acurite_592txr_const.min_count_bit_for_found);
}

void ws_protocol_decoder_acurite_592txr_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderAcurite_592TXR* instance = context;
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
