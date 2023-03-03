#include "magellan.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolMagellan"

static const SubGhzBlockConst subghz_protocol_magellan_const = {
    .te_short = 200,
    .te_long = 400,
    .te_delta = 100,
    .min_count_bit_for_found = 32,
};

struct SubGhzProtocolDecoderMagellan {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderMagellan {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    MagellanDecoderStepReset = 0,
    MagellanDecoderStepCheckPreambula,
    MagellanDecoderStepFoundPreambula,
    MagellanDecoderStepSaveDuration,
    MagellanDecoderStepCheckDuration,
} MagellanDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_magellan_decoder = {
    .alloc = subghz_protocol_decoder_magellan_alloc,
    .free = subghz_protocol_decoder_magellan_free,

    .feed = subghz_protocol_decoder_magellan_feed,
    .reset = subghz_protocol_decoder_magellan_reset,

    .get_hash_data = subghz_protocol_decoder_magellan_get_hash_data,
    .serialize = subghz_protocol_decoder_magellan_serialize,
    .deserialize = subghz_protocol_decoder_magellan_deserialize,
    .get_string = subghz_protocol_decoder_magellan_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_magellan_encoder = {
    .alloc = subghz_protocol_encoder_magellan_alloc,
    .free = subghz_protocol_encoder_magellan_free,

    .deserialize = subghz_protocol_encoder_magellan_deserialize,
    .stop = subghz_protocol_encoder_magellan_stop,
    .yield = subghz_protocol_encoder_magellan_yield,
};

const SubGhzProtocol subghz_protocol_magellan = {
    .name = SUBGHZ_PROTOCOL_MAGELLAN_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_magellan_decoder,
    .encoder = &subghz_protocol_magellan_encoder,
};

void* subghz_protocol_encoder_magellan_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderMagellan* instance = malloc(sizeof(SubGhzProtocolEncoderMagellan));

    instance->base.protocol = &subghz_protocol_magellan;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_magellan_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderMagellan* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderMagellan instance
 * @return true On success
 */
static bool subghz_protocol_encoder_magellan_get_upload(SubGhzProtocolEncoderMagellan* instance) {
    furi_assert(instance);

    size_t index = 0;

    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_short * 4);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_short);
    for(uint8_t i = 0; i < 12; i++) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_short);
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_long);

    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_long * 3);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_long);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_long);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_short);
        }
    }

    //Send stop bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_magellan_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_magellan_const.te_long * 100);

    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_magellan_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderMagellan* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_magellan_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_magellan_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_magellan_stop(void* context) {
    SubGhzProtocolEncoderMagellan* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_magellan_yield(void* context) {
    SubGhzProtocolEncoderMagellan* instance = context;

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

void* subghz_protocol_decoder_magellan_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderMagellan* instance = malloc(sizeof(SubGhzProtocolDecoderMagellan));
    instance->base.protocol = &subghz_protocol_magellan;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_magellan_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    free(instance);
}

void subghz_protocol_decoder_magellan_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    instance->decoder.parser_step = MagellanDecoderStepReset;
}

uint8_t subghz_protocol_magellan_crc8(uint8_t* data, size_t len) {
    uint8_t crc = 0x00;
    size_t i, j;
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
    return crc;
}

static bool subghz_protocol_magellan_check_crc(SubGhzProtocolDecoderMagellan* instance) {
    uint8_t data[3] = {
        instance->decoder.decode_data >> 24,
        instance->decoder.decode_data >> 16,
        instance->decoder.decode_data >> 8};
    return (instance->decoder.decode_data & 0xFF) ==
           subghz_protocol_magellan_crc8(data, sizeof(data));
}

void subghz_protocol_decoder_magellan_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;

    switch(instance->decoder.parser_step) {
    case MagellanDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_short) <
                       subghz_protocol_magellan_const.te_delta)) {
            instance->decoder.parser_step = MagellanDecoderStepCheckPreambula;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;

    case MagellanDecoderStepCheckPreambula:
        if(level) {
            instance->decoder.te_last = duration;
        } else {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_magellan_const.te_short) <
                subghz_protocol_magellan_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_short) <
                subghz_protocol_magellan_const.te_delta)) {
                // Found header
                instance->header_count++;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_magellan_const.te_short) <
                 subghz_protocol_magellan_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_long) <
                 subghz_protocol_magellan_const.te_delta * 2) &&
                (instance->header_count > 10)) {
                instance->decoder.parser_step = MagellanDecoderStepFoundPreambula;
            } else {
                instance->decoder.parser_step = MagellanDecoderStepReset;
            }
        }
        break;

    case MagellanDecoderStepFoundPreambula:
        if(level) {
            instance->decoder.te_last = duration;
        } else {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_magellan_const.te_short * 6) <
                subghz_protocol_magellan_const.te_delta * 3) &&
               (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_long) <
                subghz_protocol_magellan_const.te_delta * 2)) {
                instance->decoder.parser_step = MagellanDecoderStepSaveDuration;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
            } else {
                instance->decoder.parser_step = MagellanDecoderStepReset;
            }
        }
        break;

    case MagellanDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = MagellanDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = MagellanDecoderStepReset;
        }
        break;

    case MagellanDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_magellan_const.te_short) <
                subghz_protocol_magellan_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_long) <
                subghz_protocol_magellan_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = MagellanDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_magellan_const.te_long) <
                 subghz_protocol_magellan_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_magellan_const.te_short) <
                 subghz_protocol_magellan_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = MagellanDecoderStepSaveDuration;
            } else if(duration >= (subghz_protocol_magellan_const.te_long * 3)) {
                //Found stop bit
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_magellan_const.min_count_bit_for_found) &&
                   subghz_protocol_magellan_check_crc(instance)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = MagellanDecoderStepReset;
            } else {
                instance->decoder.parser_step = MagellanDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = MagellanDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_magellan_check_remote_controller(SubGhzBlockGeneric* instance) {
    /*
*   package 32b            data 24b           CRC8
*   0x037AE4828 => 001101111010111001001000 00101000
*   
*   0x037AE48 (flipped in reverse bit sequence) => 0x1275EC
*
*   0x1275EC =>  0x12-event codes, 0x75EC-serial (dec 117236)
*
*   event codes
*   bit_0: 1-Open/Motion, 0-close/ok
*   bit_1: 1-Tamper On (alarm), 0-Tamper Off (ok)
*   bit_2: ?
*   bit_3: 1-power on
*   bit_4: model type - wireless reed
*   bit_5: model type - motion sensor
*   bit_6: ?
*   bit_7: ?
*
*/
    uint64_t data_rev = subghz_protocol_blocks_reverse_key(instance->data >> 8, 24);
    instance->serial = data_rev & 0xFFFF;
    instance->btn = (data_rev >> 16) & 0xFF;
}

static void subghz_protocol_magellan_get_event_serialize(uint8_t event, FuriString* output) {
    furi_string_cat_printf(
        output,
        "%s%s%s%s%s%s%s%s",
        ((event >> 4) & 0x1 ? (event & 0x1 ? " Open" : " Close") :
                              (event & 0x1 ? " Motion" : " Ok")),
        ((event >> 1) & 0x1 ? ", Tamper On\n(Alarm)" : ""),
        ((event >> 2) & 0x1 ? ", ?" : ""),
        ((event >> 3) & 0x1 ? ", Power On" : ""),
        ((event >> 4) & 0x1 ? ", MT:Wireless_Reed" : ""),
        ((event >> 5) & 0x1 ? ", MT:Motion_Sensor" : ""),
        ((event >> 6) & 0x1 ? ", ?" : ""),
        ((event >> 7) & 0x1 ? ", ?" : ""));
}

uint8_t subghz_protocol_decoder_magellan_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_magellan_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_magellan_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_magellan_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_magellan_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderMagellan* instance = context;
    subghz_protocol_magellan_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%08lX\r\n"
        "Sn:%03ld%03ld, Event:0x%02X\r\n"
        "Stat:",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        (instance->generic.serial >> 8) & 0xFF,
        instance->generic.serial & 0xFF,
        instance->generic.btn);

    subghz_protocol_magellan_get_event_serialize(instance->generic.btn, output);
}
