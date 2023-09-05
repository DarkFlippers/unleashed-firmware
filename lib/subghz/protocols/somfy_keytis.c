#include "somfy_keytis.h"
#include <lib/toolbox/manchester_decoder.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolSomfyKeytis"

static const SubGhzBlockConst subghz_protocol_somfy_keytis_const = {
    .te_short = 640,
    .te_long = 1280,
    .te_delta = 250,
    .min_count_bit_for_found = 80,
};

struct SubGhzProtocolDecoderSomfyKeytis {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    ManchesterState manchester_saved_state;
    uint32_t press_duration_counter;
};

struct SubGhzProtocolEncoderSomfyKeytis {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    SomfyKeytisDecoderStepReset = 0,
    SomfyKeytisDecoderStepCheckPreambula,
    SomfyKeytisDecoderStepFoundPreambula,
    SomfyKeytisDecoderStepStartDecode,
    SomfyKeytisDecoderStepDecoderData,
} SomfyKeytisDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_somfy_keytis_decoder = {
    .alloc = subghz_protocol_decoder_somfy_keytis_alloc,
    .free = subghz_protocol_decoder_somfy_keytis_free,

    .feed = subghz_protocol_decoder_somfy_keytis_feed,
    .reset = subghz_protocol_decoder_somfy_keytis_reset,

    .get_hash_data = subghz_protocol_decoder_somfy_keytis_get_hash_data,
    .serialize = subghz_protocol_decoder_somfy_keytis_serialize,
    .deserialize = subghz_protocol_decoder_somfy_keytis_deserialize,
    .get_string = subghz_protocol_decoder_somfy_keytis_get_string,
};

const SubGhzProtocol subghz_protocol_somfy_keytis = {
    .name = SUBGHZ_PROTOCOL_SOMFY_KEYTIS_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_somfy_keytis_decoder,
    .encoder = &subghz_protocol_somfy_keytis_encoder,
};

const SubGhzProtocolEncoder subghz_protocol_somfy_keytis_encoder = {
    .alloc = subghz_protocol_encoder_somfy_keytis_alloc,
    .free = subghz_protocol_encoder_somfy_keytis_free,

    .deserialize = subghz_protocol_encoder_somfy_keytis_deserialize,
    .stop = subghz_protocol_encoder_somfy_keytis_stop,
    .yield = subghz_protocol_encoder_somfy_keytis_yield,
};

void* subghz_protocol_encoder_somfy_keytis_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderSomfyKeytis* instance = malloc(sizeof(SubGhzProtocolEncoderSomfyKeytis));

    instance->base.protocol = &subghz_protocol_somfy_keytis;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 512;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    return instance;
}

void* subghz_protocol_decoder_somfy_keytis_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderSomfyKeytis* instance = malloc(sizeof(SubGhzProtocolDecoderSomfyKeytis));
    instance->base.protocol = &subghz_protocol_somfy_keytis;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_encoder_somfy_keytis_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyKeytis* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_decoder_somfy_keytis_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    free(instance);
}

void subghz_protocol_decoder_somfy_keytis_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

static bool
    subghz_protocol_somfy_keytis_gen_data(SubGhzProtocolEncoderSomfyKeytis* instance, uint8_t btn) {
    UNUSED(btn);
    uint64_t data = instance->generic.data ^ (instance->generic.data >> 8);
    instance->generic.btn = (data >> 48) & 0xF;
    instance->generic.cnt = (data >> 24) & 0xFFFF;
    instance->generic.serial = data & 0xFFFFFF;

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }

    uint8_t frame[10];
    frame[0] = (0xA << 4) | instance->generic.btn;
    frame[1] = 0xF << 4;
    frame[2] = instance->generic.cnt >> 8;
    frame[3] = instance->generic.cnt;
    frame[4] = instance->generic.serial >> 16;
    frame[5] = instance->generic.serial >> 8;
    frame[6] = instance->generic.serial;
    frame[7] = 0xC4;
    frame[8] = 0x00;
    frame[9] = 0x19;

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
    data = 0;
    for(uint8_t i = 7; i < 10; ++i) {
        data <<= 8;
        data |= frame[i];
    }
    instance->generic.data_2 = data;
    return true;
}

bool subghz_protocol_somfy_keytis_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyKeytis* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.data_count_bit = 80;
    bool res = subghz_protocol_somfy_keytis_gen_data(instance, btn);
    if(res) {
        return SubGhzProtocolStatusOk ==
               subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    }
    return res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderSomfyKeytis instance
 * @return true On success
 */
static bool subghz_protocol_encoder_somfy_keytis_get_upload(
    SubGhzProtocolEncoderSomfyKeytis* instance,
    uint8_t btn) {
    furi_assert(instance);

    // Gen new key
    if(!subghz_protocol_somfy_keytis_gen_data(instance, btn)) {
        return false;
    }

    size_t index = 0;

    //Send header
    //Wake up
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)9415); // 1
    instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)89565); // 0
    //Hardware sync
    for(uint8_t i = 0; i < 12; ++i) {
        instance->encoder.upload[index++] = level_duration_make(
            true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 4); // 1
        instance->encoder.upload[index++] = level_duration_make(
            false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 4); // 0
    }
    //Software sync
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)4550); // 1
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0

    //Send key data MSB manchester

    for(uint8_t i = instance->generic.data_count_bit - 24; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                instance->encoder.upload[index - 1].duration *= 2; // 00
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
            }

        } else {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                instance->encoder.upload[index - 1].duration *= 2; // 11
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
            }
        }
    }

    for(uint8_t i = 24; i > 0; i--) {
        if(bit_read(instance->generic.data_2, i - 1)) {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                instance->encoder.upload[index - 1].duration *= 2; // 00
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
            }

        } else {
            if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                instance->encoder.upload[index - 1].duration *= 2; // 11
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
            } else {
                instance->encoder.upload[index++] = level_duration_make(
                    true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                instance->encoder.upload[index++] = level_duration_make(
                    false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
            }
        }
    }

    //Inter-frame silence
    if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
        instance->encoder.upload[index - 1].duration +=
            (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 3;
    } else {
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 3);
    }

    for(uint8_t i = 0; i < 2; ++i) {
        //Hardware sync
        for(uint8_t i = 0; i < 6; ++i) {
            instance->encoder.upload[index++] = level_duration_make(
                true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 4); // 1
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 4); // 0
        }
        //Software sync
        instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)4550); // 1
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0

        //Send key data MSB manchester

        for(uint8_t i = instance->generic.data_count_bit - 24; i > 0; i--) {
            if(bit_read(instance->generic.data, i - 1)) {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                    instance->encoder.upload[index - 1].duration *= 2; // 00
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                }

            } else {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                    instance->encoder.upload[index - 1].duration *= 2; // 11
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                }
            }
        }

        for(uint8_t i = 24; i > 0; i--) {
            if(bit_read(instance->generic.data_2, i - 1)) {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
                    instance->encoder.upload[index - 1].duration *= 2; // 00
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                }

            } else {
                if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_HIGH) {
                    instance->encoder.upload[index - 1].duration *= 2; // 11
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                } else {
                    instance->encoder.upload[index++] = level_duration_make(
                        true, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 1
                    instance->encoder.upload[index++] = level_duration_make(
                        false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short); // 0
                }
            }
        }
        //Inter-frame silence
        if(instance->encoder.upload[index - 1].level == LEVEL_DURATION_LEVEL_LOW) {
            instance->encoder.upload[index - 1].duration +=
                (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 3;
        } else {
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 3);
        }
    }

    //Inter-frame silence
    instance->encoder.upload[index - 1].duration +=
        (uint32_t)30415 - (uint32_t)subghz_protocol_somfy_keytis_const.te_short * 3;

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
    subghz_protocol_encoder_somfy_keytis_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderSomfyKeytis* instance = context;
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

        subghz_protocol_encoder_somfy_keytis_get_upload(instance, instance->generic.btn);

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

void subghz_protocol_encoder_somfy_keytis_stop(void* context) {
    SubGhzProtocolEncoderSomfyKeytis* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_somfy_keytis_yield(void* context) {
    SubGhzProtocolEncoderSomfyKeytis* instance = context;

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

/** 
 * Сhecksum calculation.
 * @param data Вata for checksum calculation
 * @return CRC
 */
static uint8_t subghz_protocol_somfy_keytis_crc(uint64_t data) {
    uint8_t crc = 0;
    data &= 0xFFF0FFFFFFFFFF;
    for(uint8_t i = 0; i < 56; i += 8) {
        crc = crc ^ data >> i ^ (data >> (i + 4));
    }
    return crc & 0xf;
}

void subghz_protocol_decoder_somfy_keytis_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case SomfyKeytisDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
                          subghz_protocol_somfy_keytis_const.te_delta * 4) {
            instance->decoder.parser_step = SomfyKeytisDecoderStepFoundPreambula;
            instance->header_count++;
        }
        break;
    case SomfyKeytisDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
                        subghz_protocol_somfy_keytis_const.te_delta * 4)) {
            instance->decoder.parser_step = SomfyKeytisDecoderStepCheckPreambula;
        } else {
            instance->header_count = 0;
            instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
        }
        break;
    case SomfyKeytisDecoderStepCheckPreambula:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
               subghz_protocol_somfy_keytis_const.te_delta * 4) {
                instance->decoder.parser_step = SomfyKeytisDecoderStepFoundPreambula;
                instance->header_count++;
            } else if(
                (instance->header_count > 1) &&
                (DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 7) <
                 subghz_protocol_somfy_keytis_const.te_delta * 4)) {
                instance->decoder.parser_step = SomfyKeytisDecoderStepDecoderData;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->press_duration_counter = 0;
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

    case SomfyKeytisDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short) <
               subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_long) <
                subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= (subghz_protocol_somfy_keytis_const.te_long +
                             subghz_protocol_somfy_keytis_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_somfy_keytis_const.min_count_bit_for_found) {
                    //check crc
                    uint64_t data_tmp = instance->generic.data ^ (instance->generic.data >> 8);
                    if(((data_tmp >> 40) & 0xF) == subghz_protocol_somfy_keytis_crc(data_tmp)) {
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
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            } else {
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short) <
               subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_long) <
                subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                if(instance->decoder.decode_count_bit < 56) {
                    instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                } else {
                    instance->press_duration_counter = (instance->press_duration_counter << 1) |
                                                       data;
                }

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
static void subghz_protocol_somfy_keytis_check_remote_controller(SubGhzBlockGeneric* instance) {
    //https://pushstack.wordpress.com/somfy-rts-protocol/
    /*
 *                                                  604 us
 *                                                  /
 *  | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 
 *  
 *  +--------+        +--------+        +---...---+
 *  +        +--------+        +--------+         +--+XXXX...XXX+
 *  
 *  |              hw. sync.            |   soft.    |          |
 *  |                                   |   sync.    |   data   |
 *  
 * 
 *     encrypt              |           decrypt
 *  
 *  package 80 bit   pdc      key btn   crc cnt serial
 * 
 * 0xA453537C4B9855 C40019 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 C80026 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 CC0033 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D00049 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D4005C => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D80063 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 DC0076 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E00086 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E40093 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E800AC => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 EC00B9 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F000C3 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F400D6 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F800E9 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC00FC => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0102 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0113 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0120 => 0xA  4  F  7 002F 37D3CD
 * ..........
 * 0xA453537C4B9855 FC048F => 0xA  4  F  7 002F 37D3CD
 *
 * Pdc: "Press Duration Counter" the total delay of the button is sent 72 parcels,
 *            pdc          cnt4b           cnt8b  pdc_crc
 *           C40019	 =>	 11 0001 00 0000 00000001 1001
 *           C80026  =>  11 0010 00 0000 00000010 0110
 *           CC0033  =>  11 0011 00 0000 00000011 0011
 *           D00049  =>  11 0100 00 0000 00000100 1001
 *           D4005C  =>  11 0101 00 0000 00000101 1100
 *           D80063  =>  11 0110 00 0000 00000110 0011
 *           DC0076  =>  11 0111 00 0000 00000111 0110
 *           E00086  =>  11 1000 00 0000 00001000 0110
 *           E40093  =>  11 1001 00 0000 00001001 0011
 *           E800AC  =>  11 1010 00 0000 00001010 1100
 *           EC00B9  =>  11 1011 00 0000 00001011 1001
 *           F000C3  =>  11 1100 00 0000 00001100 0011
 *           F400D6  =>  11 1101 00 0000 00001101 0110
 *           F800E9  =>  11 1110 00 0000 00001110 1001
 *           FC00FC  =>  11 1111 00 0000 00001111 1100
 *           FC0102  =>  11 1111 00 0000 00010000 0010
 *           FC0113  =>  11 1111 00 0000 00010001 0011
 *           FC0120  =>  11 1111 00 0000 00010010 0000
 * 
 *           Cnt4b: 4-bit counter changes from 1 to 15 then always equals 15
 *           Cnt8b: 8-bit counter changes from 1 to 72 (0x48)
 *           Ppdc_crc: 
 *                  uint8_t crc=0;
 *                  for(i=4; i<24; i+=4){
 *                      crc ^=(pdc>>i);
 *                  }
 *                  return crc;
 *               example: crc = 1^0^0^4^C = 9
 *           11, 00, 0000: const
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
 *      CRC
 * 
 *      uint8_t frame[7];
 *      for (i=0; i < 7; i++) {
 *          crc = crc ^ frame[i] ^ (frame[i] >> 4);
 *      }
 *      crc = crc & 0xf;
 *
 */

    uint64_t data = instance->data ^ (instance->data >> 8);
    instance->btn = (data >> 48) & 0xF;
    instance->cnt = (data >> 24) & 0xFFFF;
    instance->serial = data & 0xFFFFFF;
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_somfy_keytis_get_name_button(uint8_t btn) {
    const char* name_btn[0x10] = {
        "Unknown",
        "0x01",
        "0x02",
        "Prog",
        "Key_1",
        "0x05",
        "0x06",
        "0x07",
        "0x08",
        "0x09",
        "0x0A",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

uint8_t subghz_protocol_decoder_somfy_keytis_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_somfy_keytis_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_uint32(
           flipper_format, "Duration_Counter", &instance->press_duration_counter, 1)) {
        FURI_LOG_E(TAG, "Unable to add Duration_Counter");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_somfy_keytis_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_somfy_keytis_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        if(!flipper_format_read_uint32(
               flipper_format,
               "Duration_Counter",
               (uint32_t*)&instance->press_duration_counter,
               1)) {
            FURI_LOG_E(TAG, "Missing Duration_Counter");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
    } while(false);

    return ret;
}

void subghz_protocol_decoder_somfy_keytis_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;

    subghz_protocol_somfy_keytis_check_remote_controller(&instance->generic);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "%lX%08lX%06lX\r\n"
        "Sn:0x%06lX \r\n"
        "Cnt:0x%04lX\r\n"
        "Btn:%s\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->press_duration_counter,
        instance->generic.serial,
        instance->generic.cnt,
        subghz_protocol_somfy_keytis_get_name_button(instance->generic.btn));
}
