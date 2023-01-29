#include "lacrosse_tx.h"

#define TAG "WSProtocolLaCrosse_TX"

/*
 * Help
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/lacrosse.c
 *  
 * 
 * LaCrosse TX 433 Mhz Temperature and Humidity Sensors.
 * - Tested: TX-7U and TX-6U (Temperature only)
 * - Not Tested but should work: TX-3, TX-4
 * - also TFA Dostmann 30.3120.90 sensor (for e.g. 35.1018.06 (WS-9015) station)
 * - also TFA Dostmann 30.3121 sensor
 * Protocol Documentation: http://www.f6fbb.org/domo/sensors/tx3_th.php
 * Message is 44 bits, 11 x 4 bit nybbles:
 *    [00] [cnt = 10] [type] [addr] [addr + parity] [v1] [v2] [v3] [iv1] [iv2] [check]
 * Notes:
 * - Zero Pulses are longer (1,400 uS High, 1,000 uS Low) = 2,400 uS
 * - One Pulses are shorter (  550 uS High, 1,000 uS Low) = 1,600 uS
 * - Sensor id changes when the battery is changed
 * - Primary Value are BCD with one decimal place: vvv = 12.3
 * - Secondary value is integer only intval = 12, seems to be a repeat of primary
 *   This may actually be an additional data check because the 4 bit checksum
 *   and parity bit is  pretty week at detecting errors.
 * - Temperature is in Celsius with 50.0 added (to handle negative values)
 * - Humidity values appear to be integer precision, decimal always 0.
 * - There is a 4 bit checksum and a parity bit covering the three digit value
 * - Parity check for TX-3 and TX-4 might be different.
 * - Msg sent with one repeat after 30 mS
 * - Temperature and humidity are sent as separate messages
 * - Frequency for each sensor may be could be off by as much as 50-75 khz
 * - LaCrosse Sensors in other frequency ranges (915 Mhz) use FSK not OOK
 *   so they can't be decoded by rtl_433 currently.
 * - Temperature and Humidity are sent in different messages bursts.
*/

#define LACROSSE_TX_GAP 1000
#define LACROSSE_TX_BIT_SIZE 44
#define LACROSSE_TX_SUNC_PATTERN 0x0A000000000
#define LACROSSE_TX_SUNC_MASK 0x0F000000000
#define LACROSSE_TX_MSG_TYPE_TEMP 0x00
#define LACROSSE_TX_MSG_TYPE_HUM 0x0E

static const SubGhzBlockConst ws_protocol_lacrosse_tx_const = {
    .te_short = 550,
    .te_long = 1300,
    .te_delta = 120,
    .min_count_bit_for_found = 40,
};

struct WSProtocolDecoderLaCrosse_TX {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;

    uint16_t header_count;
};

struct WSProtocolEncoderLaCrosse_TX {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    LaCrosse_TXDecoderStepReset = 0,
    LaCrosse_TXDecoderStepCheckPreambule,
    LaCrosse_TXDecoderStepSaveDuration,
    LaCrosse_TXDecoderStepCheckDuration,
} LaCrosse_TXDecoderStep;

const SubGhzProtocolDecoder ws_protocol_lacrosse_tx_decoder = {
    .alloc = ws_protocol_decoder_lacrosse_tx_alloc,
    .free = ws_protocol_decoder_lacrosse_tx_free,

    .feed = ws_protocol_decoder_lacrosse_tx_feed,
    .reset = ws_protocol_decoder_lacrosse_tx_reset,

    .get_hash_data = ws_protocol_decoder_lacrosse_tx_get_hash_data,
    .serialize = ws_protocol_decoder_lacrosse_tx_serialize,
    .deserialize = ws_protocol_decoder_lacrosse_tx_deserialize,
    .get_string = ws_protocol_decoder_lacrosse_tx_get_string,
};

const SubGhzProtocolEncoder ws_protocol_lacrosse_tx_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_lacrosse_tx = {
    .name = WS_PROTOCOL_LACROSSE_TX_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_lacrosse_tx_decoder,
    .encoder = &ws_protocol_lacrosse_tx_encoder,
};

void* ws_protocol_decoder_lacrosse_tx_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderLaCrosse_TX* instance = malloc(sizeof(WSProtocolDecoderLaCrosse_TX));
    instance->base.protocol = &ws_protocol_lacrosse_tx;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_lacrosse_tx_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
    free(instance);
}

void ws_protocol_decoder_lacrosse_tx_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
    instance->header_count = 0;
    instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
}

static bool ws_protocol_lacrosse_tx_check_crc(WSProtocolDecoderLaCrosse_TX* instance) {
    if(!instance->decoder.decode_data) return false;
    uint8_t msg[] = {
        (instance->decoder.decode_data >> 36) & 0x0F,
        (instance->decoder.decode_data >> 32) & 0x0F,
        (instance->decoder.decode_data >> 28) & 0x0F,
        (instance->decoder.decode_data >> 24) & 0x0F,
        (instance->decoder.decode_data >> 20) & 0x0F,
        (instance->decoder.decode_data >> 16) & 0x0F,
        (instance->decoder.decode_data >> 12) & 0x0F,
        (instance->decoder.decode_data >> 8) & 0x0F,
        (instance->decoder.decode_data >> 4) & 0x0F};

    uint8_t crc = subghz_protocol_blocks_add_bytes(msg, 9);
    return ((crc & 0x0F) == ((instance->decoder.decode_data) & 0x0F));
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_lacrosse_tx_remote_controller(WSBlockGeneric* instance) {
    uint8_t msg_type = (instance->data >> 32) & 0x0F;
    instance->id = (((instance->data >> 28) & 0x0F) << 3) | (((instance->data >> 24) & 0x0F) >> 1);

    float msg_value = (float)((instance->data >> 20) & 0x0F) * 10.0f +
                      (float)((instance->data >> 16) & 0x0F) +
                      (float)((instance->data >> 12) & 0x0F) * 0.1f;

    if(msg_type == LACROSSE_TX_MSG_TYPE_TEMP) { //-V1051
        instance->temp = msg_value - 50.0f;
        instance->humidity = WS_NO_HUMIDITY;
    } else if(msg_type == LACROSSE_TX_MSG_TYPE_HUM) {
        //ToDo for verification, records are needed with sensors maintaining temperature and temperature for this standard
        instance->humidity = (uint8_t)msg_value;
    } else {
        furi_crash("WS: WSProtocolLaCrosse_TX incorrect msg_type.");
    }

    instance->btn = WS_NO_BTN;
    instance->battery_low = WS_NO_BATT;
    instance->channel = WS_NO_CHANNEL;
}

void ws_protocol_decoder_lacrosse_tx_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;

    switch(instance->decoder.parser_step) {
    case LaCrosse_TXDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, LACROSSE_TX_GAP) <
                        ws_protocol_lacrosse_tx_const.te_delta * 2)) {
            instance->decoder.parser_step = LaCrosse_TXDecoderStepCheckPreambule;
            instance->header_count = 0;
        }
        break;

    case LaCrosse_TXDecoderStepCheckPreambule:

        if(level) {
            if((DURATION_DIFF(duration, ws_protocol_lacrosse_tx_const.te_short) <
                ws_protocol_lacrosse_tx_const.te_delta) &&
               (instance->header_count > 1)) {
                instance->decoder.parser_step = LaCrosse_TXDecoderStepCheckDuration;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.te_last = duration;
            } else if(duration > (ws_protocol_lacrosse_tx_const.te_long * 2)) {
                instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, LACROSSE_TX_GAP) <
               ws_protocol_lacrosse_tx_const.te_delta * 2) {
                instance->decoder.te_last = duration;
                instance->header_count++;
            } else {
                instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
            }
        }

        break;

    case LaCrosse_TXDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = LaCrosse_TXDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
        }
        break;

    case LaCrosse_TXDecoderStepCheckDuration:

        if(!level) {
            if(duration > LACROSSE_TX_GAP * 3) {
                if(DURATION_DIFF(
                       instance->decoder.te_last, ws_protocol_lacrosse_tx_const.te_short) <
                   ws_protocol_lacrosse_tx_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                    instance->decoder.parser_step = LaCrosse_TXDecoderStepSaveDuration;
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, ws_protocol_lacrosse_tx_const.te_long) <
                    ws_protocol_lacrosse_tx_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                    instance->decoder.parser_step = LaCrosse_TXDecoderStepSaveDuration;
                }
                if((instance->decoder.decode_data & LACROSSE_TX_SUNC_MASK) ==
                   LACROSSE_TX_SUNC_PATTERN) {
                    if(ws_protocol_lacrosse_tx_check_crc(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = LACROSSE_TX_BIT_SIZE;
                        ws_protocol_lacrosse_tx_remote_controller(&instance->generic);
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }

                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->header_count = 0;
                instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
                break;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_lacrosse_tx_const.te_short) <
                 ws_protocol_lacrosse_tx_const.te_delta) &&
                (DURATION_DIFF(duration, LACROSSE_TX_GAP) <
                 ws_protocol_lacrosse_tx_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = LaCrosse_TXDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_lacrosse_tx_const.te_long) <
                 ws_protocol_lacrosse_tx_const.te_delta) &&
                (DURATION_DIFF(duration, LACROSSE_TX_GAP) <
                 ws_protocol_lacrosse_tx_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = LaCrosse_TXDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
            }

        } else {
            instance->decoder.parser_step = LaCrosse_TXDecoderStepReset;
        }

        break;
    }
}

uint8_t ws_protocol_decoder_lacrosse_tx_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool ws_protocol_decoder_lacrosse_tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool ws_protocol_decoder_lacrosse_tx_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
    bool ret = false;
    do {
        if(!ws_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           ws_protocol_lacrosse_tx_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void ws_protocol_decoder_lacrosse_tx_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderLaCrosse_TX* instance = context;
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
