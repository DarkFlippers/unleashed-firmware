#include "ambient_weather.h"
#include <lib/toolbox/manchester_decoder.h>

#define TAG "WSProtocolAmbient_Weather"

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/ambient_weather.c
 * 
 * Decode Ambient Weather F007TH, F012TH, TF 30.3208.02, SwitchDoc F016TH.
 * Devices supported:
 * - Ambient Weather F007TH Thermo-Hygrometer.
 * - Ambient Weather F012TH Indoor/Display Thermo-Hygrometer.
 * - TFA senders 30.3208.02 from the TFA "Klima-Monitor" 30.3054,
 * - SwitchDoc Labs F016TH.
 * This decoder handles the 433mhz/868mhz thermo-hygrometers.
 * The 915mhz (WH*) family of devices use different modulation/encoding.
 * Byte 0   Byte 1   Byte 2   Byte 3   Byte 4   Byte 5
 * xxxxMMMM IIIIIIII BCCCTTTT TTTTTTTT HHHHHHHH MMMMMMMM
 * - x: Unknown 0x04 on F007TH/F012TH
 * - M: Model Number?, 0x05 on F007TH/F012TH/SwitchDocLabs F016TH
 * - I: ID byte (8 bits), volatie, changes at power up,
 * - B: Battery Low
 * - C: Channel (3 bits 1-8) - F007TH set by Dip switch, F012TH soft setting
 * - T: Temperature 12 bits - Fahrenheit * 10 + 400
 * - H: Humidity (8 bits)
 * - M: Message integrity check LFSR Digest-8, gen 0x98, key 0x3e, init 0x64
 * 
 * three repeats without gap
 * full preamble is 0x00145 (the last bits might not be fixed, e.g. 0x00146)
 * and on decoding also 0xffd45
 */

#define AMBIENT_WEATHER_PACKET_HEADER_1 0xFFD440000000000 //0xffd45 .. 0xffd46
#define AMBIENT_WEATHER_PACKET_HEADER_2 0x001440000000000 //0x00145 .. 0x00146
#define AMBIENT_WEATHER_PACKET_HEADER_MASK 0xFFFFC0000000000

static const SubGhzBlockConst ws_protocol_ambient_weather_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 120,
    .min_count_bit_for_found = 48,
};

struct WSProtocolDecoderAmbient_Weather {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t header_count;
};

struct WSProtocolEncoderAmbient_Weather {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

const SubGhzProtocolDecoder ws_protocol_ambient_weather_decoder = {
    .alloc = ws_protocol_decoder_ambient_weather_alloc,
    .free = ws_protocol_decoder_ambient_weather_free,

    .feed = ws_protocol_decoder_ambient_weather_feed,
    .reset = ws_protocol_decoder_ambient_weather_reset,

    .get_hash_data = ws_protocol_decoder_ambient_weather_get_hash_data,
    .serialize = ws_protocol_decoder_ambient_weather_serialize,
    .deserialize = ws_protocol_decoder_ambient_weather_deserialize,
    .get_string = ws_protocol_decoder_ambient_weather_get_string,
};

const SubGhzProtocolEncoder ws_protocol_ambient_weather_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_ambient_weather = {
    .name = WS_PROTOCOL_AMBIENT_WEATHER_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_ambient_weather_decoder,
    .encoder = &ws_protocol_ambient_weather_encoder,
};

void* ws_protocol_decoder_ambient_weather_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderAmbient_Weather* instance = malloc(sizeof(WSProtocolDecoderAmbient_Weather));
    instance->base.protocol = &ws_protocol_ambient_weather;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_ambient_weather_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
    free(instance);
}

void ws_protocol_decoder_ambient_weather_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

static bool ws_protocol_ambient_weather_check_crc(WSProtocolDecoderAmbient_Weather* instance) {
    uint8_t msg[] = {
        instance->decoder.decode_data >> 40,
        instance->decoder.decode_data >> 32,
        instance->decoder.decode_data >> 24,
        instance->decoder.decode_data >> 16,
        instance->decoder.decode_data >> 8};

    uint8_t crc = subghz_protocol_blocks_lfsr_digest8(msg, 5, 0x98, 0x3e) ^ 0x64;
    return (crc == (uint8_t)(instance->decoder.decode_data & 0xFF));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_ambient_weather_remote_controller(WSBlockGeneric* instance) {
    instance->id = (instance->data >> 32) & 0xFF;
    instance->battery_low = (instance->data >> 31) & 1;
    instance->channel = ((instance->data >> 28) & 0x07) + 1;
    instance->temp =
        locale_fahrenheit_to_celsius(((float)((instance->data >> 16) & 0x0FFF) - 400.0f) / 10.0f);
    instance->humidity = (instance->data >> 8) & 0xFF;
    instance->btn = WS_NO_BTN;

    // ToDo maybe it won't be needed
    /*
    Sanity checks to reduce false positives and other bad data
    Packets with Bad data often pass the MIC check.
    - humidity > 100 (such as 255) and
    - temperatures > 140 F (such as 369.5 F and 348.8 F
    Specs in the F007TH and F012TH manuals state the range is:
    - Temperature: -40 to 140 F
    - Humidity: 10 to 99%
    @todo - sanity check b[0] "model number"
    - 0x45 - F007TH and F012TH
    - 0x?5 - SwitchDocLabs F016TH temperature sensor (based on comment b[0] & 0x0f == 5)
    - ? - TFA 30.3208.02
    if (instance->humidity < 0 || instance->humidity > 100) {
        ERROR;
    }

    if (instance->temp < -40.0 || instance->temp > 140.0) {
         ERROR;
    }
    */
}

void ws_protocol_decoder_ambient_weather_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    if(!level) {
        if(DURATION_DIFF(duration, ws_protocol_ambient_weather_const.te_short) <
           ws_protocol_ambient_weather_const.te_delta) {
            event = ManchesterEventShortLow;
        } else if(
            DURATION_DIFF(duration, ws_protocol_ambient_weather_const.te_long) <
            ws_protocol_ambient_weather_const.te_delta * 2) {
            event = ManchesterEventLongLow;
        }
    } else {
        if(DURATION_DIFF(duration, ws_protocol_ambient_weather_const.te_short) <
           ws_protocol_ambient_weather_const.te_delta) {
            event = ManchesterEventShortHigh;
        } else if(
            DURATION_DIFF(duration, ws_protocol_ambient_weather_const.te_long) <
            ws_protocol_ambient_weather_const.te_delta * 2) {
            event = ManchesterEventLongHigh;
        }
    }
    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok = manchester_advance(
            instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

        if(data_ok) {
            instance->decoder.decode_data = (instance->decoder.decode_data << 1) | !data;
        }

        if(((instance->decoder.decode_data & AMBIENT_WEATHER_PACKET_HEADER_MASK) ==
            AMBIENT_WEATHER_PACKET_HEADER_1) ||
           ((instance->decoder.decode_data & AMBIENT_WEATHER_PACKET_HEADER_MASK) ==
            AMBIENT_WEATHER_PACKET_HEADER_2)) {
            if(ws_protocol_ambient_weather_check_crc(instance)) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit =
                    ws_protocol_ambient_weather_const.min_count_bit_for_found;
                ws_protocol_ambient_weather_remote_controller(&instance->generic);
                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
            }
        }
    } else {
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        manchester_advance(
            instance->manchester_saved_state,
            ManchesterEventReset,
            &instance->manchester_saved_state,
            NULL);
    }
}

uint8_t ws_protocol_decoder_ambient_weather_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool ws_protocol_decoder_ambient_weather_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool ws_protocol_decoder_ambient_weather_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
    bool ret = false;
    do {
        if(!ws_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           ws_protocol_ambient_weather_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void ws_protocol_decoder_ambient_weather_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderAmbient_Weather* instance = context;
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
