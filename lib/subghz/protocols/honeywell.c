#include "honeywell.h"

#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

// Created by HTotoo 2023-10-30
// Got a lot of help from LiQuiDz.
// Protocol decoding help from: https://github.com/merbanan/rtl_433/blob/master/src/devices/honeywell.c
// Fixes and style changes to use similar codebase as other protocols by @xMasterX 2025-10

/*
64 bit packets, repeated multiple times per open/close event.

Protocol whitepaper: "DEFCON 22: Home Insecurity" by Logan Lamb.

Data layout:

    PP PP C IIIII EE SS SS

- P: 16bit Preamble and sync bit (always ff fe)
- C: 4bit Channel
- I: 20bit Device serial number / or counter value
- E: 8bit Event, where 0x80 = Open/Close, 0x04 = Heartbeat / or id
- S: 16bit CRC
*/

#define TAG "SubGhzProtocolHoneywell"

static const SubGhzBlockConst subghz_protocol_honeywell_const = {
    .te_long = 280,
    .te_short = 143,
    .te_delta = 51,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderHoneywell {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockGeneric generic;
    SubGhzBlockDecoder decoder;
    ManchesterState manchester_saved_state;
};

struct SubGhzProtocolEncoderHoneywell {
    SubGhzProtocolEncoderBase base;
    SubGhzBlockGeneric generic;
    SubGhzProtocolBlockEncoder encoder;
};

const SubGhzProtocolDecoder subghz_protocol_honeywell_decoder = {
    .alloc = subghz_protocol_decoder_honeywell_alloc,
    .free = subghz_protocol_decoder_honeywell_free,
    .feed = subghz_protocol_decoder_honeywell_feed,
    .reset = subghz_protocol_decoder_honeywell_reset,
    .get_hash_data = subghz_protocol_decoder_honeywell_get_hash_data,
    .serialize = subghz_protocol_decoder_honeywell_serialize,
    .deserialize = subghz_protocol_decoder_honeywell_deserialize,
    .get_string = subghz_protocol_decoder_honeywell_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_honeywell_encoder = {
    .alloc = subghz_protocol_encoder_honeywell_alloc,
    .free = subghz_protocol_encoder_honeywell_free,
    .deserialize = subghz_protocol_encoder_honeywell_deserialize,
    .stop = subghz_protocol_encoder_honeywell_stop,
    .yield = subghz_protocol_encoder_honeywell_yield,
};

const SubGhzProtocol subghz_protocol_honeywell = {
    .name = SUBGHZ_PROTOCOL_HONEYWELL_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send |
            SubGhzProtocolFlag_Sensors,
    .encoder = &subghz_protocol_honeywell_encoder,
    .decoder = &subghz_protocol_honeywell_decoder,

};

static void subghz_protocol_decoder_honeywell_addbit(void* context, bool data);

void* subghz_protocol_decoder_honeywell_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHoneywell* instance = malloc(sizeof(SubGhzProtocolDecoderHoneywell));
    instance->base.protocol = &subghz_protocol_honeywell;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void* subghz_protocol_encoder_honeywell_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHoneywell* instance = malloc(sizeof(SubGhzProtocolEncoderHoneywell));

    instance->base.protocol = &subghz_protocol_honeywell;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 4;
    instance->encoder.size_upload = 64 * 2 + 10;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_honeywell_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHoneywell* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_decoder_honeywell_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;
    free(instance);
}

uint16_t subghz_protocol_honeywell_crc16(
    uint8_t const message[],
    unsigned nBytes,
    uint16_t polynomial,
    uint16_t init) {
    uint16_t remainder = init;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte] << 8;
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x8000) {
                remainder = (remainder << 1) ^ polynomial;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}

static LevelDuration
    subghz_protocol_encoder_honeywell_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_honeywell_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_honeywell_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_honeywell_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_honeywell_const.te_short;
        data.level = true;
        break;

    default:
        furi_crash("SubGhz: ManchesterEncoderResult is incorrect.");
        break;
    }
    return level_duration_make(data.level, data.duration);
}

static void
    subghz_protocol_encoder_honeywell_get_upload(SubGhzProtocolEncoderHoneywell* instance) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    for(uint8_t i = 63; i > 0; i--) {
        if(!manchester_encoder_advance(
               &enc_state, bit_read(instance->generic.data, i - 1), &result)) {
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_honeywell_add_duration_to_upload(result);
            manchester_encoder_advance(
                &enc_state, bit_read(instance->generic.data, i - 1), &result);
        }
        instance->encoder.upload[index++] =
            subghz_protocol_encoder_honeywell_add_duration_to_upload(result);
    }
    instance->encoder.upload[index] = subghz_protocol_encoder_honeywell_add_duration_to_upload(
        manchester_encoder_finish(&enc_state));
    if(level_duration_get_level(instance->encoder.upload[index])) {
        index++;
    }
    //Send delay
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_honeywell_const.te_long * 300);

    instance->encoder.size_upload = index;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_honeywell_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHoneywell* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_honeywell_get_upload(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void subghz_protocol_encoder_honeywell_stop(void* context) {
    SubGhzProtocolEncoderHoneywell* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_honeywell_yield(void* context) {
    SubGhzProtocolEncoderHoneywell* instance = context;

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

void subghz_protocol_decoder_honeywell_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;
    instance->decoder.decode_data = 0;
    instance->decoder.decode_count_bit = 0;
}

void subghz_protocol_decoder_honeywell_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    if(!level) {
        if(DURATION_DIFF(duration, subghz_protocol_honeywell_const.te_short) <
           subghz_protocol_honeywell_const.te_delta) {
            event = ManchesterEventShortLow;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_honeywell_const.te_long) <
            subghz_protocol_honeywell_const.te_delta * 2) {
            event = ManchesterEventLongLow;
        }
    } else {
        if(DURATION_DIFF(duration, subghz_protocol_honeywell_const.te_short) <
           subghz_protocol_honeywell_const.te_delta) {
            event = ManchesterEventShortHigh;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_honeywell_const.te_long) <
            subghz_protocol_honeywell_const.te_delta * 2) {
            event = ManchesterEventLongHigh;
        }
    }
    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok = manchester_advance(
            instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);
        if(data_ok) {
            subghz_protocol_decoder_honeywell_addbit(instance, data);
        }
    } else {
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
    }
}

static void subghz_protocol_decoder_honeywell_addbit(void* context, bool data) {
    SubGhzProtocolDecoderHoneywell* instance = context;
    instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
    instance->decoder.decode_count_bit++;

    if(instance->decoder.decode_count_bit < 62) {
        return;
    }

    uint16_t preamble = (instance->decoder.decode_data >> 48) & 0xFFFF;
    //can be multiple, since flipper can't read it well.. (it can, but the sensors are not that good, there are multiple of variations seen)
    if(preamble == 0b0011111111111110 || preamble == 0b0111111111111110 ||
       preamble == 0b1111111111111110) {
        uint8_t datatocrc[4];
        datatocrc[0] = (instance->decoder.decode_data >> 40) & 0xFF;
        datatocrc[1] = (instance->decoder.decode_data >> 32) & 0xFF;
        datatocrc[2] = (instance->decoder.decode_data >> 24) & 0xFF;
        datatocrc[3] = (instance->decoder.decode_data >> 16) & 0xFF;
        uint8_t channel = (instance->decoder.decode_data >> 44) & 0xF;
        uint16_t crc_calc = 0;
        if(channel == 0x2 || channel == 0x4 || channel == 0xA) {
            // 2GIG brand
            crc_calc = subghz_protocol_honeywell_crc16(datatocrc, 4, 0x8050, 0);
        } else if(channel == 0x8) {
            crc_calc = subghz_protocol_honeywell_crc16(datatocrc, 4, 0x8005, 0);
        } else {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            return;
        }
        uint16_t crc = instance->decoder.decode_data & 0xFFFF;
        if(crc == crc_calc) {
            // Removing possible artifacts from higher bits and setting header to FF FE
            instance->generic.data =
                ((((((0xFF << 16) | ((instance->decoder.decode_data >> 40) & 0xFFFF)) << 16) |
                   ((instance->decoder.decode_data >> 24) & 0xFFFF))
                  << 16) |
                 ((instance->decoder.decode_data >> 8) & 0xFFFF))
                    << 8 |
                (instance->decoder.decode_data & 0xFF);
            instance->generic.data_count_bit = 64;
            if(instance->base.callback)
                instance->base.callback(&instance->base, instance->base.context);
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            return;
        }
    } else if(instance->decoder.decode_count_bit >= 64) {
        instance->decoder.decode_data = 0;
        instance->decoder.decode_count_bit = 0;
        return;
    }
}

uint8_t subghz_protocol_decoder_honeywell_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_honeywell_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_honeywell_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_honeywell_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_honeywell_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell* instance = context;

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    instance->generic.serial = (instance->generic.data >> 24) & 0xFFFFF;
    uint8_t sensor_status = (instance->generic.data >> 16) & 0xFF;

    uint8_t channel = (instance->generic.data >> 44) & 0xF;
    uint8_t contact = (sensor_status & 0x80) >> 7;
    uint8_t tamper = (sensor_status & 0x40) >> 6;
    uint8_t reed = (sensor_status & 0x20) >> 5;
    uint8_t alarm = (sensor_status & 0x10) >> 4;
    uint8_t battery_low = (sensor_status & 0x08) >> 3;
    uint8_t heartbeat = (sensor_status & 0x04) >> 2;

    furi_string_cat_printf(
        output,
        "%s\r\n%dbit  "
        "Sn:%07lu  Ch:%u\r\n"
        "LowBat:%d  HB: %d  Cont: %s\r\n"
        "Key:%08lX%08lX\r\n"
        "State: L1:%u  L2:%u  L3:%u  L4:%u",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.serial,
        channel,
        battery_low,
        heartbeat,
        contact ? "open" : "closed",
        code_found_hi,
        code_found_lo,
        contact,
        reed,
        alarm,
        tamper);
}
