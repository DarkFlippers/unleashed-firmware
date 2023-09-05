#include "somfy_telis.h"
#include <lib/toolbox/manchester_decoder.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocolSomfyTelis"

static const SubGhzBlockConst subghz_protocol_somfy_telis_const = {
    .te_short = 640,
    .te_long = 1280,
    .te_delta = 250,
    .min_count_bit_for_found = 56,
};

struct SubGhzProtocolDecoderSomfyTelis {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    ManchesterState manchester_saved_state;
};

struct SubGhzProtocolEncoderSomfyTelis {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    SomfyTelisDecoderStepReset = 0,
    SomfyTelisDecoderStepCheckPreambula,
    SomfyTelisDecoderStepFoundPreambula,
    SomfyTelisDecoderStepStartDecode,
    SomfyTelisDecoderStepDecoderData,
} SomfyTelisDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_somfy_telis_decoder = {
    .alloc = subghz_protocol_decoder_somfy_telis_alloc,
    .free = subghz_protocol_decoder_somfy_telis_free,

    .feed = subghz_protocol_decoder_somfy_telis_feed,
    .reset = subghz_protocol_decoder_somfy_telis_reset,

    .get_hash_data = subghz_protocol_decoder_somfy_telis_get_hash_data,
    .serialize = subghz_protocol_decoder_somfy_telis_serialize,
    .deserialize = subghz_protocol_decoder_somfy_telis_deserialize,
    .get_string = subghz_protocol_decoder_somfy_telis_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_somfy_telis_encoder = {
    .alloc = subghz_protocol_encoder_somfy_telis_alloc,
    .free = subghz_protocol_encoder_somfy_telis_free,

    .deserialize = subghz_protocol_encoder_somfy_telis_deserialize,
    .stop = subghz_protocol_encoder_somfy_telis_stop,
    .yield = subghz_protocol_encoder_somfy_telis_yield,
};

const SubGhzProtocol subghz_protocol_somfy_telis = {
    .name = SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_somfy_telis_decoder,
    .encoder = &subghz_protocol_somfy_telis_encoder,
};

void* subghz_protocol_encoder_somfy_telis_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderSomfyTelis* instance = malloc(sizeof(SubGhzProtocolEncoderSomfyTelis));

    instance->base.protocol = &subghz_protocol_somfy_telis;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 512;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    return instance;
}

void subghz_protocol_encoder_somfy_telis_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyTelis* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x1 | 0x2 | 0x4 | 0x8 |
 * @return Button code
 */
static uint8_t subghz_protocol_somfy_telis_get_btn_code();

static bool subghz_protocol_somfy_telis_gen_data(
    SubGhzProtocolEncoderSomfyTelis* instance,
    uint8_t btn,
    bool new_remote) {
    // If we doing a clone we will use its data
    uint64_t data = instance->generic.data ^ (instance->generic.data >> 8);
    if(!new_remote) {
        instance->generic.btn = (data >> 44) & 0xF; // ctrl
        btn = instance->generic.btn;
        instance->generic.cnt = (data >> 24) & 0xFFFF; // rolling code
        instance->generic.serial = data & 0xFFFFFF; // address
    }

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(btn);
    }

    btn = subghz_protocol_somfy_telis_get_btn_code();

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }

    uint8_t frame[7];
    if(!new_remote) {
        frame[0] = data >> 48;
    } else {
        frame[0] = 0xA7;
    }
    frame[1] = btn << 4;
    frame[2] = instance->generic.cnt >> 8;
    frame[3] = instance->generic.cnt;
    frame[4] = instance->generic.serial >> 16;
    frame[5] = instance->generic.serial >> 8;
    frame[6] = instance->generic.serial;

    uint8_t checksum = 0;
    for(uint8_t i = 0; i < 7; i++) {
        checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
    }
    checksum &= 0xF;

    frame[1] |= checksum;

    for(uint8_t i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
    }
    data = 0;
    for(uint8_t i = 0; i < 7; ++i) {
        data <<= 8;
        data |= frame[i];
    }
    instance->generic.data = data;
    return true;
}

bool subghz_protocol_somfy_telis_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyTelis* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.data_count_bit = 56;
    bool res = subghz_protocol_somfy_telis_gen_data(instance, btn, true);
    if(res) {
        return SubGhzProtocolStatusOk ==
               subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 * @return true On success
 */
static bool subghz_protocol_encoder_somfy_telis_get_upload(
    SubGhzProtocolEncoderSomfyTelis* instance,
    uint8_t btn) {
    furi_assert(instance);

    // Gen new key
    if(!subghz_protocol_somfy_telis_gen_data(instance, btn, false)) {
        return false;
    }

    size_t index = 0;

    //Send header
    //Wake up
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)9415); // 1
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)89565); // 0
    //Hardware sync
    for(uint8_t i = 0; i < 2; ++i) {
        instance->encoder.upload[index++] = level_duration_make(
            true, (uint32_t)subghz_protocol_somfy_telis_const.te_short * 4); // 1
        instance->encoder.upload[index++] = level_duration_make(
            false, (uint32_t)subghz_protocol_somfy_telis_const.te_short * 4); // 0
    }
    //Software sync
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)4550); // 1
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0

    //Send key data MSB manchester

    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                instance->encoder.upload[index - 1].duration *= 2; // 00
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
            }

        } else {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                instance->encoder.upload[index - 1].duration *= 2; // 11
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
            }
        }
    }

    //Inter-frame silence
    if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
        instance->encoder.upload[index - 1].duration += (uint32_t)30415;
    } else {
        instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)30415);
    }

    //Retransmission
    for(uint8_t i = 0; i < 2; i++) {
        //Hardware sync
        for(uint8_t i = 0; i < 7; ++i) {
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_somfy_telis_const.te_short * 4); // 1
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_somfy_telis_const.te_short * 4); // 0
        }
        //Software sync
        instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)4550); // 1
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0

        //Send key data MSB manchester

        for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
            if(bit_read(instance->generic.data, i - 1)) {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                    instance->encoder.upload[index - 1].duration *= 2; // 00
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
                }

            } else {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                    instance->encoder.upload[index - 1].duration *= 2; // 11
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 1
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_telis_const.te_short); // 0
                }
            }
        }

        //Inter-frame silence
        if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
            instance->encoder.upload[index - 1].duration += (uint32_t)30415;
        } else {
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)30415);
        }
    }

    size_t size_upload = index;

    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_somfy_telis_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyTelis* instance = context;
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

        subghz_protocol_encoder_somfy_telis_get_upload(instance, instance->generic.btn);

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

void subghz_protocol_encoder_somfy_telis_stop(void* context) {
    SubGhzProtocolEncoderSomfyTelis* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_somfy_telis_yield(void* context) {
    SubGhzProtocolEncoderSomfyTelis* instance = context;

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

void* subghz_protocol_decoder_somfy_telis_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderSomfyTelis* instance = malloc(sizeof(SubGhzProtocolDecoderSomfyTelis));
    instance->base.protocol = &subghz_protocol_somfy_telis;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_decoder_somfy_telis_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;
    free(instance);
}

void subghz_protocol_decoder_somfy_telis_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;
    instance->decoder.parser_step = SomfyTelisDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

/** 
 * Сhecksum calculation.
 * @param data Вata for checksum calculation
 * @return CRC
 */
static uint8_t subghz_protocol_somfy_telis_crc(uint64_t data) {
    uint8_t crc = 0;
    data &= 0xFFF0FFFFFFFFFF;
    for(uint8_t i = 0; i < 56; i += 8) {
        crc = crc ^ data >> i ^ (data >> (i + 4));
    }
    return crc & 0xf;
}

void subghz_protocol_decoder_somfy_telis_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case SomfyTelisDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short * 4) <
                          subghz_protocol_somfy_telis_const.te_delta * 4) {
            instance->decoder.parser_step = SomfyTelisDecoderStepFoundPreambula;
            instance->header_count++;
        }
        break;
    case SomfyTelisDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short * 4) <
                        subghz_protocol_somfy_telis_const.te_delta * 4)) {
            instance->decoder.parser_step = SomfyTelisDecoderStepCheckPreambula;
        } else {
            instance->header_count = 0;
            instance->decoder.parser_step = SomfyTelisDecoderStepReset;
        }
        break;
    case SomfyTelisDecoderStepCheckPreambula:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short * 4) <
               subghz_protocol_somfy_telis_const.te_delta * 4) {
                instance->decoder.parser_step = SomfyTelisDecoderStepFoundPreambula;
                instance->header_count++;
            } else if(
                (instance->header_count > 1) &&
                (DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short * 7) <
                 subghz_protocol_somfy_telis_const.te_delta * 4)) {
                instance->decoder.parser_step = SomfyTelisDecoderStepDecoderData;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->header_count = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
            }
        }

        break;

    case SomfyTelisDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short) <
               subghz_protocol_somfy_telis_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_long) <
                subghz_protocol_somfy_telis_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= (subghz_protocol_somfy_telis_const.te_long +
                             subghz_protocol_somfy_telis_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_somfy_telis_const.min_count_bit_for_found) {
                    //check crc
                    uint64_t data_tmp = instance->decoder.decode_data ^
                                        (instance->decoder.decode_data >> 8);
                    if(((data_tmp >> 40) & 0xF) == subghz_protocol_somfy_telis_crc(data_tmp)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
                instance->decoder.parser_step = SomfyTelisDecoderStepReset;
            } else {
                instance->decoder.parser_step = SomfyTelisDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_short) <
               subghz_protocol_somfy_telis_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_telis_const.te_long) <
                subghz_protocol_somfy_telis_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = SomfyTelisDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_somfy_telis_check_remote_controller(SubGhzBlockGeneric* instance) {
    //https://pushstack.wordpress.com/somfy-rts-protocol/
    /*
 *                                                  604 us
 *                                                  /
 *  | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 67648 us |  30415 us  |
 *  
 *  +--------+        +--------+        +---...---+
 *  +        +--------+        +--------+         +--+XXXX...XXX+-----...-----
 *  
 *  |              hw. sync.            |   soft.    |          | Inter-frame
 *  |                                   |   sync.    |   data   |     gap
 *  
 * 
 *     encrypt              |           decrypt
 *  
 *  package 56 bit    cnt    key  btn|crc    cnt     serial
 *  0xA7232323312222 - 0   => A7    8 0   | 00 00 | 12 13 00
 *  0xA7222223312222 - 1   => A7    8 5   | 00 01 | 12 13 00
 *  0xA7212123312222 - 2   => A7    8 6   | 00 02 | 12 13 00
 * 
 * Key: “Encryption Key”, Most significant 4-bit are always 0xA, Least Significant bits is 
 *      a linear counter. In the Smoove Origin this counter is increased together with the 
 *      rolling code. But leaving this on a constant value also works. Gerardwr notes that 
 *      for some other types of remotes the MSB is not constant.
 * Btn: 4-bit Control codes, this indicates the button that is pressed
 * CRC: 4-bit Checksum.
 * Ctn: 16-bit rolling code (big-endian) increased with every button press.
 * Serial: 24-bit identifier of sending device (little-endian)
 * 
 * 
 *      Decrypt
 *  
 *      uint8_t frame[7];
 *      for (i=1; i < 7; i++) {
 *          frame[i] = frame[i] ^ frame[i-1];
 *      }
 *      or
 *      uint64 Decrypt = frame ^ (frame>>8);
 * 
 *      Btn
 *
 *     Value  Button(s)       Description
 *      0x1     My          Stop or move to favourite position
 *      0x2     Up          Move up
 *      0x3     My + Up     Set upper motor limit in initial programming mode
 *      0x4     Down        Move down
 *      0x5     My + Down   Set lower motor limit in initial programming mode
 *      0x6     Up + Down   Change motor limit and initial programming mode
 *      0x8     Prog        Used for (de-)registering remotes, see below
 *      0x9     Sun + Flag  Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
 *      0xA     Flag        Disable sun detector (FLAG symbol on the Telis Soliris RC)
 * 
 *      CRC
 * 
 *      uint8_t frame[7];
 *      for (i=0; i < 7; i++) {
 *          cksum = cksum ^ frame[i] ^ (frame[i] >> 4);
 *      }
 *      cksum = cksum & 0xf;
 *
 */

    uint64_t data = instance->data ^ (instance->data >> 8);
    instance->btn = (data >> 44) & 0xF; // ctrl
    instance->cnt = (data >> 24) & 0xFFFF; // rolling code
    instance->serial = data & 0xFFFFFF; // address

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(3);
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_somfy_telis_get_name_button(uint8_t btn) {
    const char* name_btn[16] = {
        "Unknown",
        "My",
        "Up",
        "My+Up",
        "Down",
        "My+Down",
        "Up+Down",
        "0x07",
        "Prog",
        "Sun+Flag",
        "Flag",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

uint8_t subghz_protocol_decoder_somfy_telis_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_somfy_telis_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_somfy_telis_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;
    return subghz_block_generic_deserialize_check_count_bit(
        &instance->generic,
        flipper_format,
        subghz_protocol_somfy_telis_const.min_count_bit_for_found);
}

static uint8_t subghz_protocol_somfy_telis_get_btn_code() {
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
        case 0x4:
            btn = 0x1;
            break;
        case 0x8:
            btn = 0x1;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x4;
            break;
        case 0x2:
            btn = 0x4;
            break;
        case 0x4:
            btn = 0x2;
            break;
        case 0x8:
            btn = 0x4;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x1:
            btn = 0x8;
            break;
        case 0x2:
            btn = 0x8;
            break;
        case 0x4:
            btn = 0x8;
            break;
        case 0x8:
            btn = 0x2;
            break;

        default:
            break;
        }
    }

    return btn;
}

void subghz_protocol_decoder_somfy_telis_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyTelis* instance = context;

    subghz_protocol_somfy_telis_check_remote_controller(&instance->generic);
    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%06lX \r\n"
        "Cnt:0x%04lX\r\n"
        "Btn:%s\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->generic.serial,
        instance->generic.cnt,
        subghz_protocol_somfy_telis_get_name_button(instance->generic.btn));
}
