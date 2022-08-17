#include "honeywell_wdb.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolHoneywellWDB"

/*
 * 
 * https://github.com/klohner/honeywell-wireless-doorbell
 *
 */

static const SubGhzBlockConst subghz_protocol_honeywell_wdb_const = {
    .te_short = 160,
    .te_long = 320,
    .te_delta = 60,
    .min_count_bit_for_found = 48,
};

struct SubGhzProtocolDecoderHoneywell_WDB {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    const char* device_type;
    const char* alert;
    uint8_t secret_knock;
    uint8_t relay;
    uint8_t lowbat;
};

struct SubGhzProtocolEncoderHoneywell_WDB {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Honeywell_WDBDecoderStepReset = 0,
    Honeywell_WDBDecoderStepFoundStartBit,
    Honeywell_WDBDecoderStepSaveDuration,
    Honeywell_WDBDecoderStepCheckDuration,
} Honeywell_WDBDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_honeywell_wdb_decoder = {
    .alloc = subghz_protocol_decoder_honeywell_wdb_alloc,
    .free = subghz_protocol_decoder_honeywell_wdb_free,

    .feed = subghz_protocol_decoder_honeywell_wdb_feed,
    .reset = subghz_protocol_decoder_honeywell_wdb_reset,

    .get_hash_data = subghz_protocol_decoder_honeywell_wdb_get_hash_data,
    .serialize = subghz_protocol_decoder_honeywell_wdb_serialize,
    .deserialize = subghz_protocol_decoder_honeywell_wdb_deserialize,
    .get_string = subghz_protocol_decoder_honeywell_wdb_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_honeywell_wdb_encoder = {
    .alloc = subghz_protocol_encoder_honeywell_wdb_alloc,
    .free = subghz_protocol_encoder_honeywell_wdb_free,

    .deserialize = subghz_protocol_encoder_honeywell_wdb_deserialize,
    .stop = subghz_protocol_encoder_honeywell_wdb_stop,
    .yield = subghz_protocol_encoder_honeywell_wdb_yield,
};

const SubGhzProtocol subghz_protocol_honeywell_wdb = {
    .name = SUBGHZ_PROTOCOL_HONEYWELL_WDB_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_honeywell_wdb_decoder,
    .encoder = &subghz_protocol_honeywell_wdb_encoder,
};

void* subghz_protocol_encoder_honeywell_wdb_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderHoneywell_WDB* instance =
        malloc(sizeof(SubGhzProtocolEncoderHoneywell_WDB));

    instance->base.protocol = &subghz_protocol_honeywell_wdb;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_honeywell_wdb_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderHoneywell_WDB* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 * @return true On success
 */
static bool subghz_protocol_encoder_honeywell_wdb_get_upload(
    SubGhzProtocolEncoderHoneywell_WDB* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_honeywell_wdb_const.te_short * 3);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_honeywell_wdb_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_honeywell_wdb_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_honeywell_wdb_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_honeywell_wdb_const.te_long);
        }
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_honeywell_wdb_const.te_short * 3);
    return true;
}

bool subghz_protocol_encoder_honeywell_wdb_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderHoneywell_WDB* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_honeywell_wdb_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_encoder_honeywell_wdb_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_honeywell_wdb_stop(void* context) {
    SubGhzProtocolEncoderHoneywell_WDB* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_honeywell_wdb_yield(void* context) {
    SubGhzProtocolEncoderHoneywell_WDB* instance = context;

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

void* subghz_protocol_decoder_honeywell_wdb_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderHoneywell_WDB* instance =
        malloc(sizeof(SubGhzProtocolDecoderHoneywell_WDB));
    instance->base.protocol = &subghz_protocol_honeywell_wdb;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_honeywell_wdb_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    free(instance);
}

void subghz_protocol_decoder_honeywell_wdb_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    instance->decoder.parser_step = Honeywell_WDBDecoderStepReset;
}

void subghz_protocol_decoder_honeywell_wdb_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    switch(instance->decoder.parser_step) {
    case Honeywell_WDBDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_honeywell_wdb_const.te_short * 3) <
                        subghz_protocol_honeywell_wdb_const.te_delta)) {
            //Found header Honeywell_WDB
            instance->decoder.decode_count_bit = 0;
            instance->decoder.decode_data = 0;
            instance->decoder.parser_step = Honeywell_WDBDecoderStepSaveDuration;
        }
        break;
    case Honeywell_WDBDecoderStepSaveDuration:
        if(level) { //save interval
            if(DURATION_DIFF(duration, subghz_protocol_honeywell_wdb_const.te_short * 3) <
               subghz_protocol_honeywell_wdb_const.te_delta) {
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_honeywell_wdb_const.min_count_bit_for_found) &&
                   ((instance->decoder.decode_data & 0x01) ==
                    subghz_protocol_blocks_get_parity(
                        instance->decoder.decode_data >> 1,
                        subghz_protocol_honeywell_wdb_const.min_count_bit_for_found - 1))) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.parser_step = Honeywell_WDBDecoderStepReset;
                break;
            }
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Honeywell_WDBDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = Honeywell_WDBDecoderStepReset;
        }
        break;
    case Honeywell_WDBDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_honeywell_wdb_const.te_short) <
                subghz_protocol_honeywell_wdb_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_honeywell_wdb_const.te_long) <
                subghz_protocol_honeywell_wdb_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = Honeywell_WDBDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_honeywell_wdb_const.te_long) <
                 subghz_protocol_honeywell_wdb_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_honeywell_wdb_const.te_short) <
                 subghz_protocol_honeywell_wdb_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = Honeywell_WDBDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = Honeywell_WDBDecoderStepReset;
        } else {
            instance->decoder.parser_step = Honeywell_WDBDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzProtocolDecoderHoneywell_WDB* instance
 */
static void subghz_protocol_honeywell_wdb_check_remote_controller(
    SubGhzProtocolDecoderHoneywell_WDB* instance) {
    /*
 *
 * Frame bits used in Honeywell RCWL300A, RCWL330A, Series 3, 5, 9 and all Decor Series Wireless Chimes
 * 0000 0000 1111 1111 2222 2222 3333 3333 4444 4444 5555 5555
 * 7654 3210 7654 3210 7654 3210 7654 3210 7654 3210 7654 3210
 * XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XX.. XXX. .... KEY DATA (any change and receiver doesn't seem to recognize signal)
 * XXXX XXXX XXXX XXXX XXXX .... .... .... .... .... .... .... KEY ID (different for each transmitter)
 * .... .... .... .... .... 0000 00.. 0000 0000 00.. 000. .... KEY UNKNOWN 0 (always 0 in devices I've tested)
 * .... .... .... .... .... .... ..XX .... .... .... .... .... DEVICE TYPE (10 = doorbell, 01 = PIR Motion sensor)
 * .... .... .... .... .... .... .... .... .... ..XX ...X XXX. FLAG DATA (may be modified for possible effects on receiver)
 * .... .... .... .... .... .... .... .... .... ..XX .... .... ALERT (00 = normal, 01 or 10 = right-left halo light pattern, 11 = full volume alarm)
 * .... .... .... .... .... .... .... .... .... .... ...X .... SECRET KNOCK (0 = default, 1 if doorbell is pressed 3x rapidly)
 * .... .... .... .... .... .... .... .... .... .... .... X... RELAY (1 if signal is a retransmission of a received transmission, only some models)
 * .... .... .... .... .... .... .... .... .... .... .... .X.. FLAG UNKNOWN (0 = default, but 1 is accepted and I don't observe any effects)
 * .... .... .... .... .... .... .... .... .... .... .... ..X. LOWBAT (1 if battery is low, receiver gives low battery alert)
 * .... .... .... .... .... .... .... .... .... .... .... ...X PARITY (LSB of count of set bits in previous 47 bits)
 * 
 */

    instance->generic.serial = (instance->generic.data >> 28) & 0xFFFFF;
    switch((instance->generic.data >> 20) & 0x3) {
    case 0x02:
        instance->device_type = "Doorbell";
        break;
    case 0x01:
        instance->device_type = "PIR-Motion";
        break;
    default:
        instance->device_type = "Unknown";
        break;
    }

    switch((instance->generic.data >> 16) & 0x3) {
    case 0x00:
        instance->alert = "Normal";
        break;
    case 0x01:
    case 0x02:
        instance->alert = "High";
        break;
    case 0x03:
        instance->alert = "Full";
        break;
    default:
        instance->alert = "Unknown";
        break;
    }

    instance->secret_knock = (uint8_t)((instance->generic.data >> 4) & 0x1);
    instance->relay = (uint8_t)((instance->generic.data >> 3) & 0x1);
    instance->lowbat = (uint8_t)((instance->generic.data >> 1) & 0x1);
}

uint8_t subghz_protocol_decoder_honeywell_wdb_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_honeywell_wdb_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_honeywell_wdb_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_honeywell_wdb_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_honeywell_wdb_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderHoneywell_WDB* instance = context;
    subghz_protocol_honeywell_wdb_check_remote_controller(instance);

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%05lX\r\n"
        "DT:%s  Al:%s\r\n"
        "SK:%01lX R:%01lX LBat:%01lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)((instance->generic.data >> 32) & 0xFFFFFFFF),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        instance->generic.serial,
        instance->device_type,
        instance->alert,
        instance->secret_knock,
        instance->relay,
        instance->lowbat);
}
