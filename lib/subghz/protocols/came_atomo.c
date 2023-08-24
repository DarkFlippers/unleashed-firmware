#include "came_atomo.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocoCameAtomo"

static const SubGhzBlockConst subghz_protocol_came_atomo_const = {
    .te_short = 600,
    .te_long = 1200,
    .te_delta = 250,
    .min_count_bit_for_found = 62,
};

struct SubGhzProtocolDecoderCameAtomo {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    ManchesterState manchester_saved_state;
};

struct SubGhzProtocolEncoderCameAtomo {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    CameAtomoDecoderStepReset = 0,
    CameAtomoDecoderStepDecoderData,
} CameAtomoDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_came_atomo_decoder = {
    .alloc = subghz_protocol_decoder_came_atomo_alloc,
    .free = subghz_protocol_decoder_came_atomo_free,

    .feed = subghz_protocol_decoder_came_atomo_feed,
    .reset = subghz_protocol_decoder_came_atomo_reset,

    .get_hash_data = subghz_protocol_decoder_came_atomo_get_hash_data,
    .serialize = subghz_protocol_decoder_came_atomo_serialize,
    .deserialize = subghz_protocol_decoder_came_atomo_deserialize,
    .get_string = subghz_protocol_decoder_came_atomo_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_came_atomo_encoder = {
    .alloc = subghz_protocol_encoder_came_atomo_alloc,
    .free = subghz_protocol_encoder_came_atomo_free,

    .deserialize = subghz_protocol_encoder_came_atomo_deserialize,
    .stop = subghz_protocol_encoder_came_atomo_stop,
    .yield = subghz_protocol_encoder_came_atomo_yield,
};

const SubGhzProtocol subghz_protocol_came_atomo = {
    .name = SUBGHZ_PROTOCOL_CAME_ATOMO_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_came_atomo_decoder,
    .encoder = &subghz_protocol_came_atomo_encoder,
};

static void subghz_protocol_came_atomo_remote_controller(SubGhzBlockGeneric* instance);

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x0 | 0x2 | 0x4 | 0x6 |
 * @return Button code
 */
static uint8_t subghz_protocol_came_atomo_get_btn_code();

void* subghz_protocol_encoder_came_atomo_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderCameAtomo* instance = malloc(sizeof(SubGhzProtocolEncoderCameAtomo));

    instance->base.protocol = &subghz_protocol_came_atomo;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 900; //actual size 766+
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_came_atomo_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCameAtomo* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static LevelDuration
    subghz_protocol_encoder_came_atomo_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_came_atomo_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_came_atomo_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_came_atomo_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_came_atomo_const.te_short;
        data.level = true;
        break;

    default:
        FURI_LOG_E(TAG, "SubGhz: ManchesterEncoderResult is incorrect.");
        break;
    }
    return level_duration_make(data.level, data.duration);
}

bool subghz_protocol_came_atomo_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderCameAtomo* instance = context;
    instance->generic.btn = 0x1;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.cnt_2 = 0x7e;
    instance->generic.data_count_bit = 62;
    instance->generic.data_2 =
        ((uint64_t)0x7e << 56 | (uint64_t)cnt << 40 | (uint64_t)serial << 8);

    uint8_t pack[8] = {};

    pack[0] = (instance->generic.cnt_2);
    pack[1] = (instance->generic.cnt >> 8);
    pack[2] = (instance->generic.cnt & 0xFF);
    pack[3] = ((instance->generic.data_2 >> 32) & 0xFF);
    pack[4] = ((instance->generic.data_2 >> 24) & 0xFF);
    pack[5] = ((instance->generic.data_2 >> 16) & 0xFF);
    pack[6] = ((instance->generic.data_2 >> 8) & 0xFF);
    pack[7] = (instance->generic.data_2 & 0xFF);

    atomo_encrypt(pack);
    uint32_t hi = pack[0] << 24 | pack[1] << 16 | pack[2] << 8 | pack[3];
    uint32_t lo = pack[4] << 24 | pack[5] << 16 | pack[6] << 8 | pack[7];
    instance->generic.data = (uint64_t)hi << 32 | lo;

    instance->generic.data ^= 0xFFFFFFFFFFFFFFFF;
    instance->generic.data >>= 4;
    instance->generic.data &= 0xFFFFFFFFFFFFFFF;

    return SubGhzProtocolStatusOk ==
           subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderCameAtomo instance
 */
static void subghz_protocol_encoder_came_atomo_get_upload(
    SubGhzProtocolEncoderCameAtomo* instance,
    uint8_t btn) {
    furi_assert(instance);
    size_t index = 0;

    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    uint8_t pack[8] = {};

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(btn);
    }

    btn = subghz_protocol_came_atomo_get_btn_code();

    if(btn == 0x1) {
        btn = 0x0;
    } else if(btn == 0x2) {
        btn = 0x2;
    } else if(btn == 0x3) {
        btn = 0x4;
    } else if(btn == 0x4) {
        btn = 0x6;
    }

    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_came_atomo_const.te_long * 15);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_came_atomo_const.te_long * 60);

    for(uint8_t i = 0; i < 8; i++) {
        pack[0] = (instance->generic.data_2 >> 56);
        pack[1] = (instance->generic.cnt >> 8);
        pack[2] = (instance->generic.cnt & 0xFF);
        pack[3] = ((instance->generic.data_2 >> 32) & 0xFF);
        pack[4] = ((instance->generic.data_2 >> 24) & 0xFF);
        pack[5] = ((instance->generic.data_2 >> 16) & 0xFF);
        pack[6] = ((instance->generic.data_2 >> 8) & 0xFF);
        pack[7] = (btn << 4);

        if(pack[0] == 0x7F) {
            pack[0] = 0;
        } else {
            pack[0] += (i + 1);
        }

        atomo_encrypt(pack);
        uint32_t hi = pack[0] << 24 | pack[1] << 16 | pack[2] << 8 | pack[3];
        uint32_t lo = pack[4] << 24 | pack[5] << 16 | pack[6] << 8 | pack[7];
        instance->generic.data = (uint64_t)hi << 32 | lo;

        instance->generic.data ^= 0xFFFFFFFFFFFFFFFF;
        instance->generic.data >>= 4;
        instance->generic.data &= 0xFFFFFFFFFFFFFFF;

        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_came_atomo_const.te_long);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_came_atomo_const.te_short);

        for(uint8_t i = (instance->generic.data_count_bit - 2); i > 0; i--) {
            if(!manchester_encoder_advance(
                   &enc_state, !bit_read(instance->generic.data, i - 1), &result)) {
                instance->encoder.upload[index++] =
                    subghz_protocol_encoder_came_atomo_add_duration_to_upload(result);
                manchester_encoder_advance(
                    &enc_state, !bit_read(instance->generic.data, i - 1), &result);
            }
            instance->encoder.upload[index++] =
                subghz_protocol_encoder_came_atomo_add_duration_to_upload(result);
        }
        instance->encoder.upload[index] =
            subghz_protocol_encoder_came_atomo_add_duration_to_upload(
                manchester_encoder_finish(&enc_state));
        if(level_duration_get_level(instance->encoder.upload[index])) {
            index++;
        }
        //Send pause
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_came_atomo_const.te_delta * 272);
    }
    instance->encoder.size_upload = index;
    instance->generic.cnt_2++;
    pack[0] = (instance->generic.cnt_2);
    pack[1] = (instance->generic.cnt >> 8);
    pack[2] = (instance->generic.cnt & 0xFF);
    pack[3] = ((instance->generic.data_2 >> 32) & 0xFF);
    pack[4] = ((instance->generic.data_2 >> 24) & 0xFF);
    pack[5] = ((instance->generic.data_2 >> 16) & 0xFF);
    pack[6] = ((instance->generic.data_2 >> 8) & 0xFF);
    pack[7] = (btn << 4);

    atomo_encrypt(pack);
    uint32_t hi = pack[0] << 24 | pack[1] << 16 | pack[2] << 8 | pack[3];
    uint32_t lo = pack[4] << 24 | pack[5] << 16 | pack[6] << 8 | pack[7];
    instance->generic.data = (uint64_t)hi << 32 | lo;

    instance->generic.data ^= 0xFFFFFFFFFFFFFFFF;
    instance->generic.data >>= 4;
    instance->generic.data &= 0xFFFFFFFFFFFFFFF;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_came_atomo_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderCameAtomo* instance = context;
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

        subghz_protocol_came_atomo_remote_controller(&instance->generic);
        subghz_protocol_encoder_came_atomo_get_upload(instance, instance->generic.btn);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void subghz_protocol_encoder_came_atomo_stop(void* context) {
    SubGhzProtocolEncoderCameAtomo* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_came_atomo_yield(void* context) {
    SubGhzProtocolEncoderCameAtomo* instance = context;

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

void* subghz_protocol_decoder_came_atomo_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderCameAtomo* instance = malloc(sizeof(SubGhzProtocolDecoderCameAtomo));
    instance->base.protocol = &subghz_protocol_came_atomo;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_came_atomo_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    free(instance);
}

void subghz_protocol_decoder_came_atomo_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    instance->decoder.parser_step = CameAtomoDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_decoder_came_atomo_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case CameAtomoDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long * 60) <
                        subghz_protocol_came_atomo_const.te_delta * 40)) {
            //Found header CAME
            instance->decoder.parser_step = CameAtomoDecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 1;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case CameAtomoDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_short) <
               subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long) <
                subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= ((uint32_t)subghz_protocol_came_atomo_const.te_long * 2 +
                             subghz_protocol_came_atomo_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_came_atomo_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 1;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = CameAtomoDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_short) <
               subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long) <
                subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = CameAtomoDecoderStepReset;
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

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file
 */
static void subghz_protocol_came_atomo_remote_controller(SubGhzBlockGeneric* instance) {
    /*
    * ***SkorP ver.***
    * 0x1fafef3ed0f7d9ef
    * 0x185fcc1531ee86e7
    * 0x184fa96912c567ff
    * 0x187f8a42f3dc38f7
    * 0x186f63915492a5cd
    * 0x181f40bab58bfac5
    * 0x180f25c696a01bdd
    * 0x183f06ed77b944d5
    * 0x182ef661d83d21a9
    * 0x18ded54a39247ea1
    * 0x18ceb0361a0f9fb9
    * 0x18fe931dfb16c0b1
    * 0x18ee7ace5c585d8b
    * ........ 
    * transmission consists of 99 parcels with increasing counter while holding down the button
    * with each new press, the counter in the encrypted part increases
    * 
    * 0x1FAFF13ED0F7D9EF
    * 0x1FAFF11ED0F7D9EF
    * 0x1FAFF10ED0F7D9EF
    * 0x1FAFF0FED0F7D9EF
    * 0x1FAFF0EED0F7D9EF
    * 0x1FAFF0DED0F7D9EF
    * 0x1FAFF0CED0F7D9EF
    * 0x1FAFF0BED0F7D9EF
    * 0x1FAFF0AED0F7D9EF 
    * 
    *                   where     0x1FAF - parcel counter, 0хF0A - button press counter,
    *                           0xED0F7D9E - serial number, 0хF -  key
    * 0x1FAF parcel counter - 1 in the parcel queue ^ 0x185F =  0x07F0
    * 0x185f ^ 0x185F = 0x0000
    * 0x184f ^ 0x185F = 0x0010
    * 0x187f ^ 0x185F = 0x0020
    * .....
    * 0x182e ^ 0x185F = 0x0071 
    * 0x18de ^ 0x185F = 0x0081
    * .....
    * 0x1e43 ^ 0x185F = 0x061C
    *                           where the last nibble is incremented every 8 samples
    * 
    * Decode
    * 
    * 0x1cf6931dfb16c0b1 => 0x1cf6
    * 0x1cf6 ^ 0x185F = 0x04A9
    * 0x04A9 => 0x04A = 74 (dec)
    * 74+1 % 32(atomo_magic_xor) = 11
    * GET atomo_magic_xor[11] = 0xXXXXXXXXXXXXXXXX
    * 0x931dfb16c0b1 ^ 0xXXXXXXXXXXXXXXXX =  0xEF3ED0F7D9EF
    * 0xEF3 ED0F7D9E F  => 0xEF3 - CNT, 0xED0F7D9E - SN, 0xF - key
    * 
    *  ***Eng1n33r ver. (actual)***
    * 0x1FF08D9924984115 - received data
    * 0x00F7266DB67BEEA0 - inverted data
    * 0x0501FD0000A08300 - decrypted data, 
    * where: 0x05 - Button hold-cycle counter (8-bit, from 0 to 0x7F)
    *        0x01FD - Parcel counter (normal 16-bit counter)
    *        0x0000A083 - Serial number (32-bit)
    *        0x0 - Button code (4-bit, 0x0 - #1 left-up; 0x2 - #2 right-up; 0x4 - #3 left-down;  0x6 - #4 right-down)
    *        0x0 - Last zero nibble
    * */

    instance->data ^= 0xFFFFFFFFFFFFFFFF;
    instance->data <<= 4;

    uint8_t pack[8] = {};
    pack[0] = (instance->data >> 56);
    pack[1] = ((instance->data >> 48) & 0xFF);
    pack[2] = ((instance->data >> 40) & 0xFF);
    pack[3] = ((instance->data >> 32) & 0xFF);
    pack[4] = ((instance->data >> 24) & 0xFF);
    pack[5] = ((instance->data >> 16) & 0xFF);
    pack[6] = ((instance->data >> 8) & 0xFF);
    pack[7] = (instance->data & 0xFF);

    atomo_decrypt(pack);

    instance->cnt_2 = pack[0];
    instance->cnt = (uint16_t)pack[1] << 8 | pack[2];
    instance->serial = (uint32_t)(pack[3]) << 24 | pack[4] << 16 | pack[5] << 8 | pack[6];

    uint8_t btn_decode = (pack[7] >> 4);
    if(btn_decode == 0x0) {
        instance->btn = 0x1;
    } else if(btn_decode == 0x2) {
        instance->btn = 0x2;
    } else if(btn_decode == 0x4) {
        instance->btn = 0x3;
    } else if(btn_decode == 0x6) {
        instance->btn = 0x4;
    }

    uint32_t hi = pack[0] << 24 | pack[1] << 16 | pack[2] << 8 | pack[3];
    uint32_t lo = pack[4] << 24 | pack[5] << 16 | pack[6] << 8 | pack[7];
    instance->data_2 = (uint64_t)hi << 32 | lo;

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(3);
}

void atomo_encrypt(uint8_t* buff) {
    uint8_t tmpB = (~buff[0] + 1) & 0x7F;

    uint8_t bitCnt = 8;
    while(bitCnt < 59) {
        if((tmpB & 0x18) && (((tmpB / 8) & 3) != 3)) {
            tmpB = ((tmpB << 1) & 0xFF) | 1;
        } else {
            tmpB = (tmpB << 1) & 0xFF;
        }

        if(tmpB & 0x80) {
            buff[bitCnt / 8] ^= (0x80 >> (bitCnt & 7));
        }

        bitCnt++;
    }

    buff[0] = (buff[0] ^ 5) & 0x7F;
}

void atomo_decrypt(uint8_t* buff) {
    buff[0] = (buff[0] ^ 5) & 0x7F;
    uint8_t tmpB = (-buff[0]) & 0x7F;

    uint8_t bitCnt = 8;
    while(bitCnt < 59) {
        if((tmpB & 0x18) && (((tmpB / 8) & 3) != 3)) {
            tmpB = ((tmpB << 1) & 0xFF) | 1;
        } else {
            tmpB = (tmpB << 1) & 0xFF;
        }

        if(tmpB & 0x80) {
            buff[bitCnt / 8] ^= (0x80 >> (bitCnt & 7));
        }

        bitCnt++;
    }
}

static uint8_t subghz_protocol_came_atomo_get_btn_code() {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x2;
            break;
        case 0x2:
            btn = 0x1;
            break;
        case 0x3:
            btn = 0x1;
            break;
        case 0x4:
            btn = 0x1;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x3;
            break;
        case 0x2:
            btn = 0x3;
            break;
        case 0x3:
            btn = 0x2;
            break;
        case 0x4:
            btn = 0x2;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x4;
            break;
        case 0x2:
            btn = 0x4;
            break;
        case 0x3:
            btn = 0x4;
            break;
        case 0x4:
            btn = 0x3;
            break;

        default:
            break;
        }
    }

    return btn;
}

uint8_t subghz_protocol_decoder_came_atomo_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_came_atomo_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_came_atomo_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_came_atomo_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_came_atomo_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    subghz_protocol_came_atomo_remote_controller(&instance->generic);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%08lX%08lX\r\n"
        "Sn:0x%08lX       Btn:%01X\r\n"
        "Pcl_Cnt:0x%04lX\r\n"
        "Btn_Cnt:0x%02X",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt,
        instance->generic.cnt_2);
}
