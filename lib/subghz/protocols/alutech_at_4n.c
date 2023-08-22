#include "alutech_at_4n.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocoAlutech_at_4n"

#define SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE 0xFFFFFFFF

static const SubGhzBlockConst subghz_protocol_alutech_at_4n_const = {
    .te_short = 400,
    .te_long = 800,
    .te_delta = 140,
    .min_count_bit_for_found = 72,
};

struct SubGhzProtocolDecoderAlutech_at_4n {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint64_t data;
    uint32_t crc;
    uint16_t header_count;

    const char* alutech_at_4n_rainbow_table_file_name;
};

struct SubGhzProtocolEncoderAlutech_at_4n {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
    const char* alutech_at_4n_rainbow_table_file_name;
    uint32_t crc;
};

typedef enum {
    Alutech_at_4nDecoderStepReset = 0,
    Alutech_at_4nDecoderStepCheckPreambula,
    Alutech_at_4nDecoderStepSaveDuration,
    Alutech_at_4nDecoderStepCheckDuration,
} Alutech_at_4nDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_alutech_at_4n_decoder = {
    .alloc = subghz_protocol_decoder_alutech_at_4n_alloc,
    .free = subghz_protocol_decoder_alutech_at_4n_free,

    .feed = subghz_protocol_decoder_alutech_at_4n_feed,
    .reset = subghz_protocol_decoder_alutech_at_4n_reset,

    .get_hash_data = subghz_protocol_decoder_alutech_at_4n_get_hash_data,
    .serialize = subghz_protocol_decoder_alutech_at_4n_serialize,
    .deserialize = subghz_protocol_decoder_alutech_at_4n_deserialize,
    .get_string = subghz_protocol_decoder_alutech_at_4n_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_alutech_at_4n_encoder = {
    .alloc = subghz_protocol_encoder_alutech_at_4n_alloc,
    .free = subghz_protocol_encoder_alutech_at_4n_free,

    .deserialize = subghz_protocol_encoder_alutech_at_4n_deserialize,
    .stop = subghz_protocol_encoder_alutech_at_4n_stop,
    .yield = subghz_protocol_encoder_alutech_at_4n_yield,
};

const SubGhzProtocol subghz_protocol_alutech_at_4n = {
    .name = SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_alutech_at_4n_decoder,
    .encoder = &subghz_protocol_alutech_at_4n_encoder,
};

static void subghz_protocol_alutech_at_4n_remote_controller(
    SubGhzBlockGeneric* instance,
    uint8_t crc,
    const char* file_name);

void* subghz_protocol_encoder_alutech_at_4n_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderAlutech_at_4n* instance =
        malloc(sizeof(SubGhzProtocolEncoderAlutech_at_4n));

    instance->base.protocol = &subghz_protocol_alutech_at_4n;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 512;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    instance->alutech_at_4n_rainbow_table_file_name =
        subghz_environment_get_alutech_at_4n_rainbow_table_file_name(environment);
    if(instance->alutech_at_4n_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->alutech_at_4n_rainbow_table_file_name);
    }

    return instance;
}

void subghz_protocol_encoder_alutech_at_4n_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_encoder_alutech_at_4n_stop(void* context) {
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_alutech_at_4n_yield(void* context) {
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;

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
 * Read bytes from rainbow table
 * @param file_name Full path to rainbow table the file
 * @param number_alutech_at_4n_magic_data number in the array
 * @return alutech_at_4n_magic_data
 */
static uint32_t subghz_protocol_alutech_at_4n_get_magic_data_in_file(
    const char* file_name,
    uint8_t number_alutech_at_4n_magic_data) {
    if(!strcmp(file_name, "")) return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;

    uint8_t buffer[sizeof(uint32_t)] = {0};
    uint32_t address = number_alutech_at_4n_magic_data * sizeof(uint32_t);
    uint32_t alutech_at_4n_magic_data = 0;

    if(subghz_keystore_raw_get_data(file_name, address, buffer, sizeof(uint32_t))) {
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            alutech_at_4n_magic_data = (alutech_at_4n_magic_data << 8) | buffer[i];
        }
    } else {
        alutech_at_4n_magic_data = SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;
    }
    return alutech_at_4n_magic_data;
}

static uint8_t subghz_protocol_alutech_at_4n_crc(uint64_t data) {
    uint8_t* p = (uint8_t*)&data;
    uint8_t crc = 0xff;
    for(uint8_t y = 0; y < 8; y++) {
        crc = crc ^ p[y];
        for(uint8_t i = 0; i < 8; i++) {
            if((crc & 0x80) != 0) {
                crc <<= 1;
                crc ^= 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static uint8_t subghz_protocol_alutech_at_4n_decrypt_data_crc(uint8_t data) {
    uint8_t crc = data;
    for(uint8_t i = 0; i < 8; i++) {
        if((crc & 0x80) != 0) {
            crc <<= 1;
            crc ^= 0x31;
        } else {
            crc <<= 1;
        }
    }
    return ~crc;
}

static uint64_t subghz_protocol_alutech_at_4n_decrypt(uint64_t data, const char* file_name) {
    uint8_t* p = (uint8_t*)&data;
    uint32_t data1 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    uint32_t data2 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
    uint32_t data3 = 0;
    uint32_t magic_data[] = {
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 0),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 1),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 2),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 3),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 4),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 5)};

    uint32_t i = magic_data[0];
    do {
        data2 = data2 -
                ((magic_data[1] + (data1 << 4)) ^ ((magic_data[2] + (data1 >> 5)) ^ (data1 + i)));
        data3 = data2 + i;
        i += magic_data[3];
        data1 =
            data1 - ((magic_data[4] + (data2 << 4)) ^ ((magic_data[5] + (data2 >> 5)) ^ data3));
    } while(i != 0);

    p[0] = (uint8_t)(data1 >> 24);
    p[1] = (uint8_t)(data1 >> 16);
    p[3] = (uint8_t)data1;
    p[4] = (uint8_t)(data2 >> 24);
    p[5] = (uint8_t)(data2 >> 16);
    p[2] = (uint8_t)(data1 >> 8);
    p[6] = (uint8_t)(data2 >> 8);
    p[7] = (uint8_t)data2;

    return data;
}

static uint64_t subghz_protocol_alutech_at_4n_encrypt(uint64_t data, const char* file_name) {
    uint8_t* p = (uint8_t*)&data;
    uint32_t data1 = 0;
    uint32_t data2 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    uint32_t data3 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
    uint32_t magic_data[] = {
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 6),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 4),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 5),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 1),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 2),
        subghz_protocol_alutech_at_4n_get_magic_data_in_file(file_name, 0)};

    do {
        data1 = data1 + magic_data[0];
        data2 = data2 + ((magic_data[1] + (data3 << 4)) ^
                         ((magic_data[2] + (data3 >> 5)) ^ (data1 + data3)));
        data3 = data3 + ((magic_data[3] + (data2 << 4)) ^
                         ((magic_data[4] + (data2 >> 5)) ^ (data1 + data2)));
    } while(data1 != magic_data[5]);
    p[0] = (uint8_t)(data2 >> 24);
    p[1] = (uint8_t)(data2 >> 16);
    p[3] = (uint8_t)data2;
    p[4] = (uint8_t)(data3 >> 24);
    p[5] = (uint8_t)(data3 >> 16);
    p[2] = (uint8_t)(data2 >> 8);
    p[6] = (uint8_t)(data3 >> 8);
    p[7] = (uint8_t)data3;

    return data;
}

static bool subghz_protocol_alutech_at_4n_gen_data(
    SubGhzProtocolEncoderAlutech_at_4n* instance,
    uint8_t btn) {
    uint64_t data = subghz_protocol_blocks_reverse_key(instance->generic.data, 64);

    data = subghz_protocol_alutech_at_4n_decrypt(
        data, instance->alutech_at_4n_rainbow_table_file_name);
    uint8_t crc = data >> 56;
    if(crc == subghz_protocol_alutech_at_4n_decrypt_data_crc((uint8_t)((data >> 8) & 0xFF))) {
        instance->generic.btn = (uint8_t)data & 0xFF;
        instance->generic.cnt = (uint16_t)(data >> 8) & 0xFFFF;
        instance->generic.serial = (uint32_t)(data >> 24) & 0xFFFFFFFF;
    }

    if(instance->generic.cnt < 0xFFFF) {
        if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else {
            instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
        }
    } else if(instance->generic.cnt >= 0xFFFF) {
        instance->generic.cnt = 0;
    }
    crc = subghz_protocol_alutech_at_4n_decrypt_data_crc((uint8_t)(instance->generic.cnt & 0xFF));
    data = (uint64_t)crc << 56 | (uint64_t)instance->generic.serial << 24 |
           (uint32_t)instance->generic.cnt << 8 | btn;

    data = subghz_protocol_alutech_at_4n_encrypt(
        data, instance->alutech_at_4n_rainbow_table_file_name);
    crc = subghz_protocol_alutech_at_4n_crc(data);
    instance->generic.data = subghz_protocol_blocks_reverse_key(data, 64);
    instance->crc = subghz_protocol_blocks_reverse_key(crc, 8);
    return true;
}

bool subghz_protocol_alutech_at_4n_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.data_count_bit = 72;
    bool res = subghz_protocol_alutech_at_4n_gen_data(instance, btn);
    if(res) {
        if((subghz_block_generic_serialize(&instance->generic, flipper_format, preset) !=
            SubGhzProtocolStatusOk) ||
           !flipper_format_write_uint32(flipper_format, "CRC", &instance->crc, 1)) {
            FURI_LOG_E(TAG, "Unable to add CRC");
            res = false;
        }
    }
    return res;
}

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x11 | 0x22 | 0xFF | 0x44 | 0x33 |
 * @return Button code
 */
static uint8_t subghz_protocol_alutech_at_4n_get_btn_code();

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderAlutech instance
 * @return true On success
 */
static bool subghz_protocol_encoder_alutech_at_4n_get_upload(
    SubGhzProtocolEncoderAlutech_at_4n* instance,
    uint8_t btn) {
    furi_assert(instance);

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(btn);
    }

    btn = subghz_protocol_alutech_at_4n_get_btn_code();

    // Gen new key
    if(!subghz_protocol_alutech_at_4n_gen_data(instance, btn)) {
        return false;
    }

    size_t index = 0;
    // Send preambula
    for(uint8_t i = 0; i < 12; ++i) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short); // 1
        instance->encoder.upload[index++] = level_duration_make(
            false, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short); // 0
    }

    instance->encoder.upload[index - 1].duration +=
        (uint32_t)subghz_protocol_alutech_at_4n_const.te_short * 9;

    // Send key data
    for(uint8_t i = 64; i > 0; --i) {
        if(bit_read(instance->generic.data, i - 1)) {
            //1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_alutech_at_4n_const.te_long);
        } else {
            //0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_alutech_at_4n_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short);
        }
    }
    // Send crc
    for(uint8_t i = 8; i > 0; --i) {
        if(bit_read(instance->crc, i - 1)) {
            //1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_alutech_at_4n_const.te_long);
        } else {
            //0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_alutech_at_4n_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_alutech_at_4n_const.te_short);
        }
    }
    // Inter-frame silence
    instance->encoder.upload[index - 1].duration +=
        (uint32_t)subghz_protocol_alutech_at_4n_const.te_long * 20;

    size_t size_upload = index;

    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    return true;
}

SubGhzProtocolStatus subghz_protocol_encoder_alutech_at_4n_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        if(!flipper_format_read_uint32(flipper_format, "CRC", (uint32_t*)&instance->crc, 1)) {
            FURI_LOG_E(TAG, "Missing CRC");
            break;
        }

        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        subghz_protocol_alutech_at_4n_remote_controller(
            &instance->generic, instance->crc, instance->alutech_at_4n_rainbow_table_file_name);

        subghz_protocol_encoder_alutech_at_4n_get_upload(instance, instance->generic.btn);

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
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_update_uint32(flipper_format, "CRC", &instance->crc, 1)) {
            FURI_LOG_E(TAG, "Unable to add CRC");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

void* subghz_protocol_decoder_alutech_at_4n_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderAlutech_at_4n* instance =
        malloc(sizeof(SubGhzProtocolDecoderAlutech_at_4n));
    instance->base.protocol = &subghz_protocol_alutech_at_4n;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->alutech_at_4n_rainbow_table_file_name =
        subghz_environment_get_alutech_at_4n_rainbow_table_file_name(environment);
    if(instance->alutech_at_4n_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->alutech_at_4n_rainbow_table_file_name);
    }
    return instance;
}

void subghz_protocol_decoder_alutech_at_4n_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    instance->alutech_at_4n_rainbow_table_file_name = NULL;
    free(instance);
}

void subghz_protocol_decoder_alutech_at_4n_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
}

void subghz_protocol_decoder_alutech_at_4n_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;

    switch(instance->decoder.parser_step) {
    case Alutech_at_4nDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                          subghz_protocol_alutech_at_4n_const.te_delta) {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepCheckPreambula;
            instance->header_count++;
        }
        break;
    case Alutech_at_4nDecoderStepCheckPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                        subghz_protocol_alutech_at_4n_const.te_delta)) {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            break;
        }
        if((instance->header_count > 2) &&
           (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short * 10) <
            subghz_protocol_alutech_at_4n_const.te_delta * 10)) {
            // Found header
            instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    case Alutech_at_4nDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = Alutech_at_4nDecoderStepCheckDuration;
        }
        break;
    case Alutech_at_4nDecoderStepCheckDuration:
        if(!level) {
            if(duration >= ((uint32_t)subghz_protocol_alutech_at_4n_const.te_short * 2 +
                            subghz_protocol_alutech_at_4n_const.te_delta)) {
                //add last bit
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_short) <
                   subghz_protocol_alutech_at_4n_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_long) <
                    subghz_protocol_alutech_at_4n_const.te_delta * 2) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                }

                // Found end TX
                instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_alutech_at_4n_const.min_count_bit_for_found) {
                    if(instance->generic.data != instance->data) {
                        instance->generic.data = instance->data;

                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        instance->crc = instance->decoder.decode_data;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
                    instance->data = 0;
                    instance->decoder.decode_count_bit = 0;
                    instance->header_count = 0;
                }
                break;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_short) <
                 subghz_protocol_alutech_at_4n_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_long) <
                 subghz_protocol_alutech_at_4n_const.te_delta * 2)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                if(instance->decoder.decode_count_bit == 64) {
                    instance->data = instance->decoder.decode_data;
                    instance->decoder.decode_data = 0;
                }
                instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_alutech_at_4n_const.te_long) <
                 subghz_protocol_alutech_at_4n_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short) <
                 subghz_protocol_alutech_at_4n_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                if(instance->decoder.decode_count_bit == 64) {
                    instance->data = instance->decoder.decode_data;
                    instance->decoder.decode_data = 0;
                }
                instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
                instance->header_count = 0;
            }
        } else {
            instance->decoder.parser_step = Alutech_at_4nDecoderStepReset;
            instance->header_count = 0;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file
 */
static void subghz_protocol_alutech_at_4n_remote_controller(
    SubGhzBlockGeneric* instance,
    uint8_t crc,
    const char* file_name) {
    /**
 *  Message format 72bit LSB first
 *           data        crc
 *      XXXXXXXXXXXXXXXX  CC
 *  
 *  For analysis, you need to turn the package MSB
 *  in decoded messages format
 * 
 *     crc1 serial  cnt  key
 *      cc SSSSSSSS XXxx BB 
 * 
 *  crc1 is calculated from the lower part of cnt
 *  key 1=0xff, 2=0x11, 3=0x22, 4=0x33, 5=0x44
 * 
 */

    bool status = false;
    uint64_t data = subghz_protocol_blocks_reverse_key(instance->data, 64);
    crc = subghz_protocol_blocks_reverse_key(crc, 8);

    if(crc == subghz_protocol_alutech_at_4n_crc(data)) {
        data = subghz_protocol_alutech_at_4n_decrypt(data, file_name);
        status = true;
    }

    if(status && ((uint8_t)(data >> 56) ==
                  subghz_protocol_alutech_at_4n_decrypt_data_crc((uint8_t)((data >> 8) & 0xFF)))) {
        instance->btn = (uint8_t)data & 0xFF;
        instance->cnt = (uint16_t)(data >> 8) & 0xFFFF;
        instance->serial = (uint32_t)(data >> 24) & 0xFFFFFFFF;
    }

    if(!status) {
        instance->btn = 0;
        instance->cnt = 0;
        instance->serial = 0;
    }

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(4);
}

uint8_t subghz_protocol_decoder_alutech_at_4n_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    return (uint8_t)instance->crc;
}

SubGhzProtocolStatus subghz_protocol_decoder_alutech_at_4n_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((res == SubGhzProtocolStatusOk) &&
       !flipper_format_write_uint32(flipper_format, "CRC", &instance->crc, 1)) {
        FURI_LOG_E(TAG, "Unable to add CRC");
        res = SubGhzProtocolStatusErrorParserOthers;
    }
    return res;
}

SubGhzProtocolStatus subghz_protocol_decoder_alutech_at_4n_deserialize(
    void* context,
    FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_alutech_at_4n_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "CRC", (uint32_t*)&instance->crc, 1)) {
            FURI_LOG_E(TAG, "Missing CRC");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
    } while(false);
    return ret;
}

static uint8_t subghz_protocol_alutech_at_4n_get_btn_code() {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x11:
            btn = 0x22;
            break;
        case 0x22:
            btn = 0x11;
            break;
        case 0xFF:
            btn = 0x11;
            break;
        case 0x44:
            btn = 0x11;
            break;
        case 0x33:
            btn = 0x11;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x11:
            btn = 0x44;
            break;
        case 0x22:
            btn = 0x44;
            break;
        case 0xFF:
            btn = 0x44;
            break;
        case 0x44:
            btn = 0xFF;
            break;
        case 0x33:
            btn = 0x44;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x11:
            btn = 0x33;
            break;
        case 0x22:
            btn = 0x33;
            break;
        case 0xFF:
            btn = 0x33;
            break;
        case 0x44:
            btn = 0x33;
            break;
        case 0x33:
            btn = 0x22;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_RIGHT) {
        switch(original_btn_code) {
        case 0x11:
            btn = 0xFF;
            break;
        case 0x22:
            btn = 0xFF;
            break;
        case 0xFF:
            btn = 0x22;
            break;
        case 0x44:
            btn = 0x22;
            break;
        case 0x33:
            btn = 0xFF;
            break;

        default:
            break;
        }
    }

    return btn;
}

void subghz_protocol_decoder_alutech_at_4n_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderAlutech_at_4n* instance = context;
    subghz_protocol_alutech_at_4n_remote_controller(
        &instance->generic, instance->crc, instance->alutech_at_4n_rainbow_table_file_name);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s\r\n"
        "Key:0x%08lX%08lX\nCRC:%02X  %dbit\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%04lX\r\n",
        instance->generic.protocol_name,
        code_found_hi,
        code_found_lo,
        (uint8_t)instance->crc,
        instance->generic.data_count_bit,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
