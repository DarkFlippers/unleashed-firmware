#include "auriol_hg0601a.h"

#define TAG "WSProtocolAuriol_TH"

/*
 *
Auriol HG06061A-DCF-TX sensor.

Data layout:
		DDDDDDDD-B0-NN-TT-TTTTTTTTTT-CCCC-HHHHHHHH
Exmpl.:	11110100-10-01-00-0001001100-1111-01011101

- D: id, 8 bit
- B: where B is the battery status: 1=OK, 0=LOW, 1 bit
- 0: just zero :)
- N: NN is the channel: 00=CH1, 01=CH2, 11=CH3, 2bit
- T: temperature, 12 bit: 2's complement, scaled by 10
- C: 4 bit: seems to be 0xf constantly, a separator between temp and humidity
- H: humidity sensor, humidity is 8 bits

 * The sensor sends 37 bits 10 times,
 * the packets are ppm modulated (distance coding) with a pulse of ~500 us
 * followed by a short gap of ~1000 us for a 0 bit or a long ~2000 us gap for a
 * 1 bit, the sync gap is ~4000 us.
 * 
 */

#define AURIOL_TH_CONST_DATA 0b1110

static const SubGhzBlockConst ws_protocol_auriol_th_const = {
    .te_short = 500,
    .te_long = 2000,
    .te_delta = 150,
    .min_count_bit_for_found = 37,
};

struct WSProtocolDecoderAuriol_TH {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    WSBlockGeneric generic;
};

struct WSProtocolEncoderAuriol_TH {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    WSBlockGeneric generic;
};

typedef enum {
    auriol_THDecoderStepReset = 0,
    auriol_THDecoderStepSaveDuration,
    auriol_THDecoderStepCheckDuration,
} auriol_THDecoderStep;

const SubGhzProtocolDecoder ws_protocol_auriol_th_decoder = {
    .alloc = ws_protocol_decoder_auriol_th_alloc,
    .free = ws_protocol_decoder_auriol_th_free,

    .feed = ws_protocol_decoder_auriol_th_feed,
    .reset = ws_protocol_decoder_auriol_th_reset,

    .get_hash_data = ws_protocol_decoder_auriol_th_get_hash_data,
    .serialize = ws_protocol_decoder_auriol_th_serialize,
    .deserialize = ws_protocol_decoder_auriol_th_deserialize,
    .get_string = ws_protocol_decoder_auriol_th_get_string,
};

const SubGhzProtocolEncoder ws_protocol_auriol_th_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol ws_protocol_auriol_th = {
    .name = WS_PROTOCOL_AURIOL_TH_NAME,
    .type = SubGhzProtocolWeatherStation,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_868 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &ws_protocol_auriol_th_decoder,
    .encoder = &ws_protocol_auriol_th_encoder,
};

void* ws_protocol_decoder_auriol_th_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    WSProtocolDecoderAuriol_TH* instance = malloc(sizeof(WSProtocolDecoderAuriol_TH));
    instance->base.protocol = &ws_protocol_auriol_th;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void ws_protocol_decoder_auriol_th_free(void* context) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
    free(instance);
}

void ws_protocol_decoder_auriol_th_reset(void* context) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
    instance->decoder.parser_step = auriol_THDecoderStepReset;
}

static bool ws_protocol_auriol_th_check(WSProtocolDecoderAuriol_TH* instance) {
    uint8_t type = (instance->decoder.decode_data >> 8) & 0x0F;

    if((type == AURIOL_TH_CONST_DATA) && ((instance->decoder.decode_data >> 4) != 0xffffffff)) {
        return true;
    } else {
        return false;
    }
    return true;
}

/**
 * Analysis of received data
 * @param instance Pointer to a WSBlockGeneric* instance
 */
static void ws_protocol_auriol_th_remote_controller(WSBlockGeneric* instance) {
    instance->id = (instance->data >> 31) & 0xFF;
    instance->battery_low = ((instance->data >> 30) & 1);
    instance->channel = ((instance->data >> 25) & 0x03) + 1;
    instance->btn = WS_NO_BTN;
    if(!((instance->data >> 23) & 1)) {
        instance->temp = (float)((instance->data >> 13) & 0x07FF) / 10.0f;
    } else {
        instance->temp = (float)((~(instance->data >> 13) & 0x07FF) + 1) / -10.0f;
    }

    instance->humidity = (instance->data >> 1) & 0x7F;
}

void ws_protocol_decoder_auriol_th_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;

    switch(instance->decoder.parser_step) {
    case auriol_THDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, ws_protocol_auriol_th_const.te_short * 8) <
                        ws_protocol_auriol_th_const.te_delta)) {
            //Found sync
            instance->decoder.parser_step = auriol_THDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;

    case auriol_THDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = auriol_THDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = auriol_THDecoderStepReset;
        }
        break;

    case auriol_THDecoderStepCheckDuration:
        if(!level) {
            if(DURATION_DIFF(duration, ws_protocol_auriol_th_const.te_short * 8) <
               ws_protocol_auriol_th_const.te_delta) {
                //Found sync
                instance->decoder.parser_step = auriol_THDecoderStepReset;
                if((instance->decoder.decode_count_bit ==
                    ws_protocol_auriol_th_const.min_count_bit_for_found) &&
                   ws_protocol_auriol_th_check(instance)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    ws_protocol_auriol_th_remote_controller(&instance->generic);
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                    instance->decoder.parser_step = auriol_THDecoderStepCheckDuration;
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;

                break;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_auriol_th_const.te_short) <
                 ws_protocol_auriol_th_const.te_delta) &&
                (DURATION_DIFF(duration, ws_protocol_auriol_th_const.te_short * 2) <
                 ws_protocol_auriol_th_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = auriol_THDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, ws_protocol_auriol_th_const.te_short) <
                 ws_protocol_auriol_th_const.te_delta) &&
                (DURATION_DIFF(duration, ws_protocol_auriol_th_const.te_short * 4) <
                 ws_protocol_auriol_th_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = auriol_THDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = auriol_THDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = auriol_THDecoderStepReset;
        }
        break;
    }
}

uint8_t ws_protocol_decoder_auriol_th_get_hash_data(void* context) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool ws_protocol_decoder_auriol_th_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
    return ws_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool ws_protocol_decoder_auriol_th_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
    bool ret = false;
    do {
        if(!ws_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           ws_protocol_auriol_th_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void ws_protocol_decoder_auriol_th_get_string(void* context, FuriString* output) {
    furi_assert(context);
    WSProtocolDecoderAuriol_TH* instance = context;
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
