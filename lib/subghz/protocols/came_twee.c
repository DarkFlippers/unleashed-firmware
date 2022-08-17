#include "came_twee.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://phreakerclub.com/forum/showthread.php?t=635&highlight=came+twin
 *
 */

#define TAG "SubGhzProtocolCAME_Twee"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0200 ? '1' : '0'), (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'),     \
        (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), \
        (dip & 0x0008 ? '1' : '0'), (dip & 0x0004 ? '1' : '0'), (dip & 0x0002 ? '1' : '0'), \
        (dip & 0x0001 ? '1' : '0')

/** 
 * Rainbow table Came Twee.
 */
static const uint32_t came_twee_magic_numbers_xor[15] = {
    0x0E0E0E00,
    0x1D1D1D11,
    0x2C2C2C22,
    0x3B3B3B33,
    0x4A4A4A44,
    0x59595955,
    0x68686866,
    0x77777777,
    0x86868688,
    0x95959599,
    0xA4A4A4AA,
    0xB3B3B3BB,
    0xC2C2C2CC,
    0xD1D1D1DD,
    0xE0E0E0EE,
};

static const SubGhzBlockConst subghz_protocol_came_twee_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 250,
    .min_count_bit_for_found = 54,
};

struct SubGhzProtocolDecoderCameTwee {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    ManchesterState manchester_saved_state;
};

struct SubGhzProtocolEncoderCameTwee {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    CameTweeDecoderStepReset = 0,
    CameTweeDecoderStepDecoderData,
} CameTweeDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_came_twee_decoder = {
    .alloc = subghz_protocol_decoder_came_twee_alloc,
    .free = subghz_protocol_decoder_came_twee_free,

    .feed = subghz_protocol_decoder_came_twee_feed,
    .reset = subghz_protocol_decoder_came_twee_reset,

    .get_hash_data = subghz_protocol_decoder_came_twee_get_hash_data,
    .serialize = subghz_protocol_decoder_came_twee_serialize,
    .deserialize = subghz_protocol_decoder_came_twee_deserialize,
    .get_string = subghz_protocol_decoder_came_twee_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_came_twee_encoder = {
    .alloc = subghz_protocol_encoder_came_twee_alloc,
    .free = subghz_protocol_encoder_came_twee_free,

    .deserialize = subghz_protocol_encoder_came_twee_deserialize,
    .stop = subghz_protocol_encoder_came_twee_stop,
    .yield = subghz_protocol_encoder_came_twee_yield,
};

const SubGhzProtocol subghz_protocol_came_twee = {
    .name = SUBGHZ_PROTOCOL_CAME_TWEE_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_came_twee_decoder,
    .encoder = &subghz_protocol_came_twee_encoder,
};

void* subghz_protocol_encoder_came_twee_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderCameTwee* instance = malloc(sizeof(SubGhzProtocolEncoderCameTwee));

    instance->base.protocol = &subghz_protocol_came_twee;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 1536; //max upload 92*14 = 1288 !!!!
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_came_twee_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCameTwee* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static LevelDuration
    subghz_protocol_encoder_came_twee_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_came_twee_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_came_twee_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_came_twee_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_came_twee_const.te_short;
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
 * @param instance Pointer to a SubGhzProtocolEncoderCameTwee instance
 */
static void subghz_protocol_encoder_came_twee_get_upload(SubGhzProtocolEncoderCameTwee* instance) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    uint64_t temp_parcel = 0x003FFF7200000000; //parcel mask

    for(int i = 14; i >= 0; i--) {
        temp_parcel = (temp_parcel & 0xFFFFFFFF00000000) |
                      (instance->generic.serial ^ came_twee_magic_numbers_xor[i]);

        for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
            if(!manchester_encoder_advance(&enc_state, !bit_read(temp_parcel, i - 1), &result)) {
                instance->encoder.upload[index++] =
                    subghz_protocol_encoder_came_twee_add_duration_to_upload(result);
                manchester_encoder_advance(&enc_state, !bit_read(temp_parcel, i - 1), &result);
            }
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_came_twee_add_duration_to_upload(result);
        }
        instance->encoder.upload[index] = subghz_protocol_encoder_came_twee_add_duration_to_upload(
            manchester_encoder_finish(&enc_state));
        if(level_duration_get_level(instance->encoder.upload[index])) {
            index++;
        }
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_came_twee_const.te_long * 51);
    }
    instance->encoder.size_upload = index;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_came_twee_remote_controller(SubGhzBlockGeneric* instance) {
    /*      Came Twee 54 bit, rolling code 15 parcels with
    *       a decreasing counter from 0xE to 0x0
    *       with originally coded dip switches on the console 10 bit code
    * 
    *  0x003FFF72E04A6FEE
    *  0x003FFF72D17B5EDD
    *  0x003FFF72C2684DCC
    *  0x003FFF72B3193CBB
    *  0x003FFF72A40E2BAA
    *  0x003FFF72953F1A99
    *  0x003FFF72862C0988
    *  0x003FFF7277DDF877
    *  0x003FFF7268C2E766
    *  0x003FFF7259F3D655
    *  0x003FFF724AE0C544
    *  0x003FFF723B91B433
    *  0x003FFF722C86A322
    *  0x003FFF721DB79211
    *  0x003FFF720EA48100
    * 
    *   decryption
    * the last 32 bits, do XOR by the desired number, divide the result by 4,
    * convert the first 16 bits of the resulting 32-bit number to bin and do
    * bit-by-bit mirroring, adding up to 10 bits
    * 
    * Example
    * Step 1. 0x003FFF721DB79211        => 0x1DB79211
    * Step 4. 0x1DB79211 xor 0x1D1D1D11 => 0x00AA8F00
    * Step 4. 0x00AA8F00 / 4            => 0x002AA3C0
    * Step 5. 0x002AA3C0                => 0x002A
    * Step 6. 0x002A    bin             => b101010
    * Step 7. b101010                   => b0101010000
    * Step 8. b0101010000               => (Dip) Off ON Off ON Off ON Off Off Off Off
    */

    uint8_t cnt_parcel = (uint8_t)(instance->data & 0xF);
    uint32_t data = (uint32_t)(instance->data & 0x0FFFFFFFF);

    data = (data ^ came_twee_magic_numbers_xor[cnt_parcel]);
    instance->serial = data;
    data /= 4;
    instance->btn = (data >> 4) & 0x0F;
    data >>= 16;
    data = (uint16_t)subghz_protocol_blocks_reverse_key(data, 16);
    instance->cnt = data >> 6;
}

bool subghz_protocol_encoder_came_twee_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderCameTwee* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_came_twee_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_came_twee_remote_controller(&instance->generic);
        subghz_protocol_encoder_came_twee_get_upload(instance);
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_came_twee_stop(void* context) {
    SubGhzProtocolEncoderCameTwee* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_came_twee_yield(void* context) {
    SubGhzProtocolEncoderCameTwee* instance = context;

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

void* subghz_protocol_decoder_came_twee_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderCameTwee* instance = malloc(sizeof(SubGhzProtocolDecoderCameTwee));
    instance->base.protocol = &subghz_protocol_came_twee;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_came_twee_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    free(instance);
}

void subghz_protocol_decoder_came_twee_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    instance->decoder.parser_step = CameTweeDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_decoder_came_twee_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case CameTweeDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_came_twee_const.te_long * 51) <
                        subghz_protocol_came_twee_const.te_delta * 20)) {
            //Found header CAME
            instance->decoder.parser_step = CameTweeDecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventLongLow,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventLongHigh,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case CameTweeDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_came_twee_const.te_short) <
               subghz_protocol_came_twee_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_twee_const.te_long) <
                subghz_protocol_came_twee_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= ((uint32_t)subghz_protocol_came_twee_const.te_long * 2 +
                             subghz_protocol_came_twee_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_came_twee_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongLow,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = CameTweeDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_came_twee_const.te_short) <
               subghz_protocol_came_twee_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_twee_const.te_long) <
                subghz_protocol_came_twee_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = CameTweeDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | !data;
                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_came_twee_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_came_twee_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_came_twee_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_came_twee_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_came_twee_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderCameTwee* instance = context;
    subghz_protocol_came_twee_remote_controller(&instance->generic);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Btn:%lX\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        instance->generic.btn,
        CNT_TO_DIP(instance->generic.cnt));
}
