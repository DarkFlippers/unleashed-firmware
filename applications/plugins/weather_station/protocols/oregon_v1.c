#include "oregon_v1.h"
#include <lib/toolbox/manchester_decoder.h>

#define TAG "WSProtocolOregon_V1"

/*
 * Help
 * https://github.dev/merbanan/rtl_433/blob/bb1be7f186ac0fdb7dc5d77693847d96fb95281e/src/devices/oregon_scientific_v1.c
 * 
 * OSv1 protocol.
 * 
 * MC with nominal bit width of 2930 us.
 * Pulses are somewhat longer than nominal half-bit width, 1748 us / 3216 us,
 * Gaps are somewhat shorter than nominal half-bit width, 1176 us / 2640 us.
 * After 12 preamble bits there is 4200 us gap, 5780 us pulse, 5200 us gap.
 * And next 32 bit data
 * 
 * Care must be taken with the gap after the sync pulse since it
 * is outside of the normal clocking.  Because of this a data stream
 * beginning with a 0 will have data in this gap.   
 * 
 * 
 * Data is in reverse order of bits
 *      RevBit(data32bit)=> tib23atad
 * 
 *      tib23atad => xxxxxxxx | busuTTTT | ttttzzzz | ccuuiiii 
 *
 *      - i: ID
 *      - x: CRC;
 *      - u: unknown;
 *      - b: battery low; flag to indicate low battery voltage
 *      - s: temperature sign
 *      - T: BCD, Temperature; in °C * 10
 *      - t: BCD, Temperature; in °C * 1
 *      - z: BCD, Temperature; in °C * 0.1
 *      - c: Channel 00=CH1, 01=CH2, 10=CH3
 * 
 */

#define OREGON_V1_HEADER_OK 0xFF

static const SubGhzBlockConst ws_protocol_oregon_v1_const = {
    .te_short = 1465,
    .te_long = 2930,
    .te_delta = 350,
    .min_count_bit_for_found = 32,
};

struct WSProtocolDecoderOregon_V1 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;
    ManchesterState manchester_state;
    uint16_t header_count;
    uint8_t first_bit;
};

struct WSProtocolEncoderOregon_V1 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    Oregon_V1DecoderStepReset = 0,
    Oregon_V1DecoderStepFoundPreamble,
    Oregon_V1DecoderStepParse,
} Oregon_V1DecoderStep;

const SubGhzProtocolDecoder ws_protocol_oregon_v1_decoder = {
    .alloc = ws_protocol_decoder_oregon_v1_alloc,
    .free = ws_protocol_decoder_oregon_v1_free,

    .feed = ws_protocol_decoder_oregon_v1_feed,
    .reset = ws_protocol_decoder_oregon_v1_reset,

    .get_hash_data = ws_protocol_decoder_oregon_v1_get_hash_data,
    .serialize = ws_protocol_decoder_oregon_v1_serialize,
    .deserialize = ws_protocol_decoder_oregon_v1_deserialize,
    .get_string = ws_protocol_decoder_oregon_v1_get_string,
};

const SubGhzProtocolEncoder ws_protocol_oregon_v1_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_oregon_v1 = {
    .name = WS_PROTOCOL_OREGON_V1_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_oregon_v1_decoder,
    .encoder = &ws_protocol_oregon_v1_encoder,
};

void* ws_protocol_decoder_oregon_v1_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderOregon_V1* instance = malloc(sizeof(WSProtocolDecoderOregon_V1));
    instance->base.protocol = &ws_protocol_oregon_v1;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_oregon_v1_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    free(instance);
}

void ws_protocol_decoder_oregon_v1_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    instance->decoder.parser_step = Oregon_V1DecoderStepReset;
}

static bool ws_protocol_oregon_v1_check(WSProtocolDecoderOregon_V1* instance) {
    if(!instance->decoder.decode_data) return false;
    uint64_t data = subghz_protocol_blocks_reverse_key(instance->decoder.decode_data, 32);
    uint16_t crc = (data & 0xff) + ((data >> 8) & 0xff) + ((data >> 16) & 0xff);
    crc = (crc & 0xff) + ((crc >> 8) & 0xff);
    return (crc == ((data >> 24) & 0xFF));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_oregon_v1_remote_controller(WSBlockGeneric* instance) {
    uint64_t data = subghz_protocol_blocks_reverse_key(instance->data, 32);

    instance->id = data & 0xFF;
    instance->channel = ((data >> 6) & 0x03) + 1;

    float temp_raw =
        ((data >> 8) & 0x0F) * 0.1f + ((data >> 12) & 0x0F) + ((data >> 16) & 0x0F) * 10.0f;
    if(!((data >> 21) & 1)) {
        instance->temp = temp_raw;
    } else {
        instance->temp = -temp_raw;
    }

    instance->battery_low = !((instance->data >> 23) & 1ULL);

    instance->btn = WS_NO_BTN;
    instance->humidity = WS_NO_HUMIDITY;
}

void ws_protocol_decoder_oregon_v1_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case Oregon_V1DecoderStepReset:
        if((level) && (DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short) <
                       ws_protocol_oregon_v1_const.te_delta)) {
            instance->decoder.parser_step = Oregon_V1DecoderStepFoundPreamble;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;
    case Oregon_V1DecoderStepFoundPreamble:
        if(level) {
            //keep high levels, if they suit our durations
            if((DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short) <
                ws_protocol_oregon_v1_const.te_delta) ||
               (DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short * 4) <
                ws_protocol_oregon_v1_const.te_delta)) {
                instance->decoder.te_last = duration;
            } else {
                instance->decoder.parser_step = Oregon_V1DecoderStepReset;
            }
        } else if(
            //checking low levels
            (DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short) <
             ws_protocol_oregon_v1_const.te_delta) &&
            (DURATION_DIFF(instance->decoder.te_last, ws_protocol_oregon_v1_const.te_short) <
             ws_protocol_oregon_v1_const.te_delta)) {
            // Found header
            instance->header_count++;
        } else if(
            (DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short * 3) <
             ws_protocol_oregon_v1_const.te_delta) &&
            (DURATION_DIFF(instance->decoder.te_last, ws_protocol_oregon_v1_const.te_short) <
             ws_protocol_oregon_v1_const.te_delta)) {
            // check header
            if(instance->header_count > 7) {
                instance->header_count = OREGON_V1_HEADER_OK;
            }
        } else if(
            (instance->header_count == OREGON_V1_HEADER_OK) &&
            (DURATION_DIFF(instance->decoder.te_last, ws_protocol_oregon_v1_const.te_short * 4) <
             ws_protocol_oregon_v1_const.te_delta)) {
            //found all the necessary patterns
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 1;
            manchester_advance(
                instance->manchester_state,
                ManchesterEventReset,
                &instance->manchester_state,
                NULL);
            instance->decoder.parser_step = Oregon_V1DecoderStepParse;
            if(duration < ws_protocol_oregon_v1_const.te_short * 4) {
                instance->first_bit = 1;
            } else {
                instance->first_bit = 0;
            }
        } else {
            instance->decoder.parser_step = Oregon_V1DecoderStepReset;
        }
        break;
    case Oregon_V1DecoderStepParse:
        if(level) {
            if(DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short) <
               ws_protocol_oregon_v1_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_long) <
                ws_protocol_oregon_v1_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = Oregon_V1DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_short) <
               ws_protocol_oregon_v1_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, ws_protocol_oregon_v1_const.te_long) <
                ws_protocol_oregon_v1_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(duration >= ((uint32_t)ws_protocol_oregon_v1_const.te_long * 2)) {
                if(instance->decoder.decode_count_bit ==
                   ws_protocol_oregon_v1_const.min_count_bit_for_found) {
                    if(instance->first_bit) {
                        instance->decoder.decode_data = ~instance->decoder.decode_data | (1 << 31);
                    }
                    if(ws_protocol_oregon_v1_check(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        ws_protocol_oregon_v1_remote_controller(&instance->generic);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                manchester_advance(
                    instance->manchester_state,
                    ManchesterEventReset,
                    &instance->manchester_state,
                    NULL);
            } else {
                instance->decoder.parser_step = Oregon_V1DecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_state, event, &instance->manchester_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | !data;
                instance->decoder.decode_count_bit++;
            }
        }

        break;
    }
}

uint8_t ws_protocol_decoder_oregon_v1_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus ws_protocol_decoder_oregon_v1_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    ws_protocol_decoder_oregon_v1_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
    return ws_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, ws_protocol_oregon_v1_const.min_count_bit_for_found);
}

void ws_protocol_decoder_oregon_v1_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderOregon_V1* instance = context;
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
