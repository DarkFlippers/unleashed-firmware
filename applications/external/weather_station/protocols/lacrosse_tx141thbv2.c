#include "lacrosse_tx141thbv2.h"

#define TAG "WSProtocolLaCrosse_TX141THBv2"

#define LACROSSE_TX141TH_BV2_BIT_COUNT 41

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/lacrosse_tx141x.c
 *  
 *     iiii iiii | bkcc tttt | tttt tttt | hhhh hhhh | cccc cccc | u - 41 bit
 *        or
 *     iiii iiii | bkcc tttt | tttt tttt | hhhh hhhh | cccc cccc | -40 bit
 * - i: identification; changes on battery switch
 * - c: lfsr_digest8_reflect;
 * - u: unknown; 
 * - b: battery low; flag to indicate low battery voltage
 * - h: Humidity; 
 * - t: Temperature; in °F as binary number with one decimal place + 50 °F offset
 * - n: Channel; Channel number 1 - 3
 */

static const SubGhzBlockConst ws_protocol_lacrosse_tx141thbv2_const = {
    .te_short = 208,
    .te_long = 417,
    .te_delta = 120,
    .min_count_bit_for_found = 40,
};

struct WSProtocolDecoderLaCrosse_TX141THBv2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;

    uint16_t header_count;
};

struct WSProtocolEncoderLaCrosse_TX141THBv2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    LaCrosse_TX141THBv2DecoderStepReset = 0,
    LaCrosse_TX141THBv2DecoderStepCheckPreambule,
    LaCrosse_TX141THBv2DecoderStepSaveDuration,
    LaCrosse_TX141THBv2DecoderStepCheckDuration,
} LaCrosse_TX141THBv2DecoderStep;

const SubGhzProtocolDecoder ws_protocol_lacrosse_tx141thbv2_decoder = {
    .alloc = ws_protocol_decoder_lacrosse_tx141thbv2_alloc,
    .free = ws_protocol_decoder_lacrosse_tx141thbv2_free,

    .feed = ws_protocol_decoder_lacrosse_tx141thbv2_feed,
    .reset = ws_protocol_decoder_lacrosse_tx141thbv2_reset,

    .get_hash_data = ws_protocol_decoder_lacrosse_tx141thbv2_get_hash_data,
    .serialize = ws_protocol_decoder_lacrosse_tx141thbv2_serialize,
    .deserialize = ws_protocol_decoder_lacrosse_tx141thbv2_deserialize,
    .get_string = ws_protocol_decoder_lacrosse_tx141thbv2_get_string,
};

const SubGhzProtocolEncoder ws_protocol_lacrosse_tx141thbv2_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_lacrosse_tx141thbv2 = {
    .name = WS_PROTOCOL_LACROSSE_TX141THBV2_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_lacrosse_tx141thbv2_decoder,
    .encoder = &ws_protocol_lacrosse_tx141thbv2_encoder,
};

void* ws_protocol_decoder_lacrosse_tx141thbv2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance =
        malloc(sizeof(WSProtocolDecoderLaCrosse_TX141THBv2));
    instance->base.protocol = &ws_protocol_lacrosse_tx141thbv2;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_lacrosse_tx141thbv2_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
    free(instance);
}

void ws_protocol_decoder_lacrosse_tx141thbv2_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
    instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
}

static bool
    ws_protocol_lacrosse_tx141thbv2_check_crc(WSProtocolDecoderLaCrosse_TX141THBv2* instance) {
    if(!instance->decoder.decode_data) return false;
    uint64_t data = instance->decoder.decode_data;
    if(instance->decoder.decode_count_bit == LACROSSE_TX141TH_BV2_BIT_COUNT) {
        data >>= 1;
    }
    uint8_t msg[] = {data >> 32, data >> 24, data >> 16, data >> 8};

    uint8_t crc = subghz_protocol_blocks_lfsr_digest8_reflect(msg, 4, 0x31, 0xF4);
    return (crc == (data & 0xFF));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_lacrosse_tx141thbv2_remote_controller(WSBlockGeneric* instance) {
    uint64_t data = instance->data;
    if(instance->data_count_bit == LACROSSE_TX141TH_BV2_BIT_COUNT) {
        data >>= 1;
    }
    instance->id = data >> 32;
    instance->battery_low = (data >> 31) & 1;
    instance->btn = (data >> 30) & 1;
    instance->channel = ((data >> 28) & 0x03) + 1;
    instance->temp = ((float)((data >> 16) & 0x0FFF) - 500.0f) / 10.0f;
    instance->humidity = (data >> 8) & 0xFF;
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static bool ws_protocol_decoder_lacrosse_tx141thbv2_add_bit(
    WSProtocolDecoderLaCrosse_TX141THBv2* instance,
    uint32_t te_last,
    uint32_t te_current) {
    furi_assert(instance);
    bool ret = false;
    if(DURATION_DIFF(
           te_last + te_current,
           ws_protocol_lacrosse_tx141thbv2_const.te_short +
               ws_protocol_lacrosse_tx141thbv2_const.te_long) <
       ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2) {
        if(te_last > te_current) {
            subghz_protocol_blocks_add_bit(&instance->decoder, 1);
        } else {
            subghz_protocol_blocks_add_bit(&instance->decoder, 0);
        }
        ret = true;
    }

    return ret;
}
void ws_protocol_decoder_lacrosse_tx141thbv2_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;

    switch(instance->decoder.parser_step) {
    case LaCrosse_TX141THBv2DecoderStepReset:
        if((level) &&
           (DURATION_DIFF(duration, ws_protocol_lacrosse_tx141thbv2_const.te_short * 4) <
            ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2)) {
            instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepCheckPreambule;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;

    case LaCrosse_TX141THBv2DecoderStepCheckPreambule:
        if(level) {
            instance->decoder.te_last = duration;
        } else {
            if((DURATION_DIFF(
                    instance->decoder.te_last,
                    ws_protocol_lacrosse_tx141thbv2_const.te_short * 4) <
                ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2) &&
               (DURATION_DIFF(duration, ws_protocol_lacrosse_tx141thbv2_const.te_short * 4) <
                ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2)) {
                //Found preambule
                instance->header_count++;
            } else if(instance->header_count == 4) {
                if(ws_protocol_decoder_lacrosse_tx141thbv2_add_bit(
                       instance, instance->decoder.te_last, duration)) {
                    instance->decoder.decode_data = instance->decoder.decode_data & 1;
                    instance->decoder.decode_count_bit = 1;
                    instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepSaveDuration;
                } else {
                    instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
            }
        }
        break;

    case LaCrosse_TX141THBv2DecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
        }
        break;

    case LaCrosse_TX141THBv2DecoderStepCheckDuration:
        if(!level) {
            if(((DURATION_DIFF(
                     instance->decoder.te_last,
                     ws_protocol_lacrosse_tx141thbv2_const.te_short * 3) <
                 ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2) &&
                (DURATION_DIFF(duration, ws_protocol_lacrosse_tx141thbv2_const.te_short * 4) <
                 ws_protocol_lacrosse_tx141thbv2_const.te_delta * 2))) {
                if((instance->decoder.decode_count_bit ==
                    ws_protocol_lacrosse_tx141thbv2_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit == LACROSSE_TX141TH_BV2_BIT_COUNT)) {
                    if(ws_protocol_lacrosse_tx141thbv2_check_crc(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        ws_protocol_lacrosse_tx141thbv2_remote_controller(&instance->generic);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                    instance->header_count = 1;
                    instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepCheckPreambule;
                    break;
                }
            } else if(ws_protocol_decoder_lacrosse_tx141thbv2_add_bit(
                          instance, instance->decoder.te_last, duration)) {
                instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = LaCrosse_TX141THBv2DecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_lacrosse_tx141thbv2_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_lacrosse_tx141thbv2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus ws_protocol_decoder_lacrosse_tx141thbv2_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        ws_protocol_lacrosse_tx141thbv2_const.min_count_bit_for_found);
}

void ws_protocol_decoder_lacrosse_tx141thbv2_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX141THBv2* instance = context;
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
