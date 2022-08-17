#include "power_smart.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolPowerSmart"
#define POWER_SMART_PACKET_HEADER 0xFD000000AA000000
#define POWER_SMART_PACKET_HEADER_MASK 0xFF000000FF000000

#define CHANNEL_PATTERN "%c%c%c%c%c%c"
#define CNT_TO_CHANNEL(dip)                                                             \
    (dip & 0x0001 ? '*' : '-'), (dip & 0x0002 ? '*' : '-'), (dip & 0x0004 ? '*' : '-'), \
        (dip & 0x0008 ? '*' : '-'), (dip & 0x0010 ? '*' : '-'), (dip & 0x0020 ? '*' : '-')

static const SubGhzBlockConst subghz_protocol_power_smart_const = {
    .te_short = 225,
    .te_long = 450,
    .te_delta = 100,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderPowerSmart {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderPowerSmart {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    PowerSmartDecoderStepReset = 0,
    PowerSmartDecoderFoundHeader,
    PowerSmartDecoderStepDecoderData,
} PowerSmartDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_power_smart_decoder = {
    .alloc = subghz_protocol_decoder_power_smart_alloc,
    .free = subghz_protocol_decoder_power_smart_free,

    .feed = subghz_protocol_decoder_power_smart_feed,
    .reset = subghz_protocol_decoder_power_smart_reset,

    .get_hash_data = subghz_protocol_decoder_power_smart_get_hash_data,
    .serialize = subghz_protocol_decoder_power_smart_serialize,
    .deserialize = subghz_protocol_decoder_power_smart_deserialize,
    .get_string = subghz_protocol_decoder_power_smart_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_power_smart_encoder = {
    .alloc = subghz_protocol_encoder_power_smart_alloc,
    .free = subghz_protocol_encoder_power_smart_free,

    .deserialize = subghz_protocol_encoder_power_smart_deserialize,
    .stop = subghz_protocol_encoder_power_smart_stop,
    .yield = subghz_protocol_encoder_power_smart_yield,
};

const SubGhzProtocol subghz_protocol_power_smart = {
    .name = SUBGHZ_PROTOCOL_POWER_SMART_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_power_smart_decoder,
    .encoder = &subghz_protocol_power_smart_encoder,
};

void* subghz_protocol_encoder_power_smart_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderPowerSmart* instance = malloc(sizeof(SubGhzProtocolEncoderPowerSmart));

    instance->base.protocol = &subghz_protocol_power_smart;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 1024;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_power_smart_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderPowerSmart* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static LevelDuration
    subghz_protocol_encoder_power_smart_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_power_smart_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_power_smart_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_power_smart_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_power_smart_const.te_short;
        data.level = true;
        break;

    default:
        furi_crash("SubGhz: ManchesterEncoderResult is incorrect.");
        break;
    }
    return level_duration_make(data.level, data.duration);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderPowerSmart instance
 */
static void
    subghz_protocol_encoder_power_smart_get_upload(SubGhzProtocolEncoderPowerSmart* instance) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    for(int i = 8; i > 0; i--) {
        for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
            if(!manchester_encoder_advance(
                   &enc_state, !bit_read(instance->generic.data, i - 1), &result)) {
                instance->encoder.upload[index++] =
                    subghz_protocol_encoder_power_smart_add_duration_to_upload(result);
                manchester_encoder_advance(
                    &enc_state, !bit_read(instance->generic.data, i - 1), &result);
            }
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_power_smart_add_duration_to_upload(result);
        }
    }
    instance->encoder.upload[index] = subghz_protocol_encoder_power_smart_add_duration_to_upload(
        manchester_encoder_finish(&enc_state));
    if(level_duration_get_level(instance->encoder.upload[index])) {
        index++;
    }
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_power_smart_const.te_long * 1111);
    instance->encoder.size_upload = index;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_power_smart_remote_controller(SubGhzBlockGeneric* instance) {
    /*
    * Protocol: Manchester encoding, symbol rate ~2222.
    * Packet Format: 
    *       0xFDXXXXYYAAZZZZWW where 0xFD and 0xAA sync word
    *                           XXXX = ~ZZZZ, YY=(~WW)-1 
    * Example:
    *                               SYNC1 K1 CHANNEL DATA1   K2 DATA2    SYNC2  ~K1 ~CHANNEL ~DATA2  ~K2 (~DATA2)-1
    *       0xFD2137ACAADEC852 => 11111101 0 010000 10011011 1 10101100 10101010  1  1011110 1100100  0  01010010
    *       0xFDA137ACAA5EC852 => 11111101 1 010000 10011011 1 10101100 10101010  0  1011110 1100100  0  01010010
    *       0xFDA136ACAA5EC952 => 11111101 1 010000 10011011 0 10101100 10101010  0  1011110 1100100  1  01010010
    * 
    * Key:
    *       K1K2
    *        0 0 - key_unknown
    *        0 1 - key_down
    *        1 0 - key_up
    *        1 1 - key_stop
    *        
    */

    instance->btn = ((instance->data >> 54) & 0x02) | ((instance->data >> 40) & 0x1);
    instance->serial = ((instance->data >> 33) & 0x3FFF00) | ((instance->data >> 32) & 0xFF);
    instance->cnt = ((instance->data >> 49) & 0x3F);
}

bool subghz_protocol_encoder_power_smart_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderPowerSmart* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_power_smart_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_power_smart_remote_controller(&instance->generic);
        subghz_protocol_encoder_power_smart_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_power_smart_stop(void* context) {
    SubGhzProtocolEncoderPowerSmart* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_power_smart_yield(void* context) {
    SubGhzProtocolEncoderPowerSmart* instance = context;

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

void* subghz_protocol_decoder_power_smart_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderPowerSmart* instance = malloc(sizeof(SubGhzProtocolDecoderPowerSmart));
    instance->base.protocol = &subghz_protocol_power_smart;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_power_smart_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    free(instance);
}

void subghz_protocol_decoder_power_smart_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

bool subghz_protocol_power_smart_chek_valid(uint64_t packet) {
    uint32_t data_1 = (uint32_t)((packet >> 40) & 0xFFFF);
    uint32_t data_2 = (uint32_t)((~packet >> 8) & 0xFFFF);
    uint8_t data_3 = (uint8_t)(packet >> 32) & 0xFF;
    uint8_t data_4 = (uint8_t)(((~packet) & 0xFF) - 1);
    return (data_1 == data_2) && (data_3 == data_4);
}

void subghz_protocol_decoder_power_smart_feed(
    void* context,
    bool level,
    volatile uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    ManchesterEvent event = ManchesterEventReset;
    if(!level) {
        if(DURATION_DIFF(duration, subghz_protocol_power_smart_const.te_short) <
           subghz_protocol_power_smart_const.te_delta) {
            event = ManchesterEventShortLow;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_power_smart_const.te_long) <
            subghz_protocol_power_smart_const.te_delta * 2) {
            event = ManchesterEventLongLow;
        }
    } else {
        if(DURATION_DIFF(duration, subghz_protocol_power_smart_const.te_short) <
           subghz_protocol_power_smart_const.te_delta) {
            event = ManchesterEventShortHigh;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_power_smart_const.te_long) <
            subghz_protocol_power_smart_const.te_delta * 2) {
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
        if((instance->decoder.decode_data & POWER_SMART_PACKET_HEADER_MASK) ==
           POWER_SMART_PACKET_HEADER) {
            if(subghz_protocol_power_smart_chek_valid(instance->decoder.decode_data)) {
                instance->decoder.decode_data = instance->decoder.decode_data;
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_count_bit =
                    subghz_protocol_power_smart_const.min_count_bit_for_found;
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

static const char* subghz_protocol_power_smart_get_name_button(uint8_t btn) {
    btn &= 0x3;
    const char* name_btn[0x4] = {"Unknown", "Down", "Up", "Stop"};
    return name_btn[btn];
}

uint8_t subghz_protocol_decoder_power_smart_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_power_smart_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_power_smart_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_power_smart_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_power_smart_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderPowerSmart* instance = context;
    subghz_protocol_power_smart_remote_controller(&instance->generic);

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%07lX \r\n"
        "Btn:%s\r\n"
        "Channel:" CHANNEL_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        subghz_protocol_power_smart_get_name_button(instance->generic.btn),
        CNT_TO_CHANNEL(instance->generic.cnt));
}
