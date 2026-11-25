#include "alutech_at_4n.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

#define TAG "SubGhzProtocoAlutech_at_4n"

#define SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE         0xFFFFFFFFFFFFFFFF
#define SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES 32

// Upload entries for one 72-bit frame: 24 preamble + 128 data + 16 crc.
// Smart modes store only the unique counters (at most 6 = 1008 timings = 4032 B,
// inside SUBGHZ_TX_MIN_FREE_BLOCK_DEFAULT) and repeat each frame in yield().
#define ALUTECH_AT_4N_UPLOAD_FRAME    168u
#define ALUTECH_AT_4N_PACK_MAX_FRAMES 6u
// Extra silence between consecutive counter steps inside a smart pack (~18.4ms total vs ~16.4ms intra-frame)
#define ALUTECH_AT_4N_PACK_STEP_GAP_EXTRA_US 2000u

//variable used to bypass CounterMode settings if user just change Counter or Button
static bool bypass = false;

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

    uint32_t crc;
    uint16_t header_count;

    const char* alutech_at_4n_rainbow_table_file_name;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderAlutech_at_4n);

struct SubGhzProtocolEncoderAlutech_at_4n {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
    const char* alutech_at_4n_rainbow_table_file_name;
    uint32_t crc;
    size_t upload_max;
    uint8_t pack_repeats[ALUTECH_AT_4N_PACK_MAX_FRAMES];
    uint8_t pack_count;
    uint8_t pack_frame;
    uint8_t pack_repeat;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderAlutech_at_4n);

typedef enum {
    Alutech_at_4nDecoderStepReset = 0,
    Alutech_at_4nDecoderStepCheckPreambula,
    Alutech_at_4nDecoderStepSaveDuration,
    Alutech_at_4nDecoderStepCheckDuration,
} Alutech_at_4nDecoderStep;

static uint8_t alutech_at4n_counter_mode = 0;

const SubGhzProtocolDecoder subghz_protocol_alutech_at_4n_decoder = {
    .alloc = subghz_protocol_decoder_alutech_at_4n_alloc,
    .free = subghz_protocol_decoder_alutech_at_4n_free,

    .feed = subghz_protocol_decoder_alutech_at_4n_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_alutech_at_4n_get_hash_data,
    .serialize = subghz_protocol_decoder_alutech_at_4n_serialize,
    .deserialize = subghz_protocol_decoder_alutech_at_4n_deserialize,
    .get_string = subghz_protocol_decoder_alutech_at_4n_get_string,
};

static LevelDuration subghz_protocol_encoder_alutech_at_4n_yield(void* context);

const SubGhzProtocolEncoder subghz_protocol_alutech_at_4n_encoder = {
    .alloc = subghz_protocol_encoder_alutech_at_4n_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_alutech_at_4n_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
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
    SubGhzProtocolEncoderAlutech_at_4n* instance = subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderAlutech_at_4n),
        &subghz_protocol_alutech_at_4n,
        3,
        ALUTECH_AT_4N_UPLOAD_FRAME);
    instance->upload_max = ALUTECH_AT_4N_UPLOAD_FRAME;
    instance->pack_count = 1;
    instance->pack_repeats[0] = 1;
    instance->pack_frame = 0;
    instance->pack_repeat = 1;
    instance->alutech_at_4n_rainbow_table_file_name =
        subghz_environment_get_alutech_at_4n_rainbow_table_file_name(environment);
    if(instance->alutech_at_4n_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->alutech_at_4n_rainbow_table_file_name);
    }
    return instance;
}

static void subghz_protocol_encoder_alutech_at_4n_upload_reserve(
    SubGhzProtocolEncoderAlutech_at_4n* instance,
    size_t size) {
    if(instance->upload_max >= size) {
        return;
    }
    free(instance->encoder.upload);
    instance->encoder.upload = malloc(size * sizeof(LevelDuration));
    instance->upload_max = size;
}

static LevelDuration subghz_protocol_encoder_alutech_at_4n_yield(void* context) {
    SubGhzProtocolEncoderAlutech_at_4n* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    const size_t frame_start = (size_t)instance->pack_frame * ALUTECH_AT_4N_UPLOAD_FRAME;
    LevelDuration ret = instance->encoder.upload[frame_start + instance->encoder.front];

    if(++instance->encoder.front == ALUTECH_AT_4N_UPLOAD_FRAME) {
        instance->encoder.front = 0;
        if(--instance->pack_repeat == 0) {
            if(++instance->pack_frame == instance->pack_count) {
                if(!subghz_block_generic_global.endless_tx) instance->encoder.repeat--;
                instance->pack_frame = 0;
            } else {
                ret.duration += ALUTECH_AT_4N_PACK_STEP_GAP_EXTRA_US;
            }
            instance->pack_repeat = instance->pack_repeats[instance->pack_frame];
        }
    }

    return ret;
}

/**
 * Read bytes from buffer array with rainbow table 
 * @param buffer Pointer to decrypted magic data buffer
 * @param number_alutech_at_4n_magic_data number in the array
 * @return alutech_at_4n_magic_data
 */
static uint32_t subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(
    uint8_t* buffer,
    uint8_t number_alutech_at_4n_magic_data) {
    uint32_t address = number_alutech_at_4n_magic_data * sizeof(uint32_t);
    uint32_t alutech_at_4n_magic_data = 0;

    for(size_t i = address; i < (address + sizeof(uint32_t)); i++) {
        alutech_at_4n_magic_data = (alutech_at_4n_magic_data << 8) | buffer[i];
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
    // load and decrypt rainbow table from file to buffer array in RAM
    if(!file_name) return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;

    uint8_t buffer[SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES] = {0};
    uint8_t* buffer_ptr = (uint8_t*)&buffer;

    if(subghz_keystore_raw_get_data(
           file_name, 0, buffer, SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES)) {
    } else {
        return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;
    }

    uint8_t* p = (uint8_t*)&data;
    uint32_t data1 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    uint32_t data2 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
    uint32_t data3 = 0;
    uint32_t magic_data[] = {
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 0),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 1),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 2),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 3),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 4),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 5)};

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

static uint64_t subghz_protocol_alutech_at_4n_encrypt_buf(uint64_t data, uint8_t* buffer_ptr) {
    uint8_t* p = (uint8_t*)&data;
    uint32_t data1 = 0;
    uint32_t data2 = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    uint32_t data3 = p[4] << 24 | p[5] << 16 | p[6] << 8 | p[7];
    uint32_t magic_data[] = {
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 6),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 4),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 5),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 1),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 2),
        subghz_protocol_alutech_at_4n_get_magic_data_from_buffer(buffer_ptr, 0)};

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

static uint64_t subghz_protocol_alutech_at_4n_encrypt(uint64_t data, const char* file_name) {
    // load and decrypt rainbow table from file to buffer array in RAM
    if(!file_name) return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;

    uint8_t buffer[SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES] = {0};

    if(!subghz_keystore_raw_get_data(
           file_name, 0, buffer, SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES)) {
        return SUBGHZ_NO_ALUTECH_AT_4N_RAINBOW_TABLE;
    }

    return subghz_protocol_alutech_at_4n_encrypt_buf(data, buffer);
}

static void subghz_protocol_alutech_at_4n_pack_key(
    SubGhzProtocolEncoderAlutech_at_4n* instance,
    uint8_t btn,
    uint8_t* table_buf) {
    uint8_t crc =
        subghz_protocol_alutech_at_4n_decrypt_data_crc((uint8_t)(instance->generic.cnt & 0xFF));
    uint64_t data = (uint64_t)crc << 56 | (uint64_t)instance->generic.serial << 24 |
                    (uint32_t)instance->generic.cnt << 8 | btn;

    data = table_buf ? subghz_protocol_alutech_at_4n_encrypt_buf(data, table_buf) :
                       subghz_protocol_alutech_at_4n_encrypt(
                           data, instance->alutech_at_4n_rainbow_table_file_name);
    crc = subghz_protocol_alutech_at_4n_crc(data);
    instance->generic.data = subghz_protocol_blocks_reverse_key(data, 64);
    instance->crc = subghz_protocol_blocks_reverse_key(crc, 8);
}

static void subghz_protocol_alutech_at_4n_step_counter_default(
    SubGhzProtocolEncoderAlutech_at_4n* instance) {
    // Check for OFEX (overflow experimental) mode
    if((furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF) || bypass) {
        bypass = false;
        // standart counter mode. PULL data from subghz_block_generic_global variables
        if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
            // if counter_override_get return FALSE then counter was not changed and we increase counter by standart mult value
            if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFF) {
                instance->generic.cnt = 0;
            } else {
                instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        }
    } else {
        //OFFEX mode
        if((instance->generic.cnt + 0x1) > 0xFFFF) {
            instance->generic.cnt = 0;
        } else if(instance->generic.cnt >= 0x1 && instance->generic.cnt != 0xFFFE) {
            instance->generic.cnt = 0xFFFE;
        } else {
            instance->generic.cnt++;
        }
    }
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

    // if we change counter/button in SignalSettings menu then we must bypass counter_modes, just gen and save signal file.
    if(subghz_block_generic_global.cnt_need_override) bypass = true;

    subghz_protocol_alutech_at_4n_step_counter_default(instance);
    subghz_protocol_alutech_at_4n_pack_key(instance, btn, NULL);
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
    if(subghz_protocol_alutech_at_4n_gen_data(instance, btn)) {
        if((subghz_block_generic_serialize(&instance->generic, flipper_format, preset) !=
            SubGhzProtocolStatusOk)) {
            FURI_LOG_E(TAG, "Serialize error");
            return false;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            return false;
        }
        if(!flipper_format_insert_or_update_uint32(flipper_format, "CRC", &instance->crc, 1)) {
            FURI_LOG_E(TAG, "Unable to add CRC");
            return false;
        }
    }
    return true;
}

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x11 | 0x22 | 0xFF | 0x44 | 0x33 |
 * @return Button code
 */
static uint8_t subghz_protocol_alutech_at_4n_get_btn_code(void);

static size_t subghz_protocol_encoder_alutech_at_4n_encode_frame(
    SubGhzProtocolEncoderAlutech_at_4n* instance,
    size_t index) {
    furi_check(index + ALUTECH_AT_4N_UPLOAD_FRAME <= instance->upload_max);

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

    return index;
}

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

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&btn)) {
        bypass = true;
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", btn);
    }

    if(subghz_block_generic_global.cnt_need_override) {
        bypass = true;
    }

    instance->encoder.front = 0;

    if((alutech_at4n_counter_mode == 0) || bypass) {
        if(!subghz_protocol_alutech_at_4n_gen_data(instance, btn)) {
            return false;
        }
        subghz_protocol_encoder_alutech_at_4n_upload_reserve(instance, ALUTECH_AT_4N_UPLOAD_FRAME);
        instance->pack_count = 1;
        instance->pack_repeats[0] = 1;
        instance->pack_frame = 0;
        instance->pack_repeat = 1;
        instance->encoder.size_upload =
            subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, 0);
        return true;
    }

    uint8_t table_buf[SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES] = {0};
    uint8_t* table_ptr = NULL;
    if(instance->alutech_at_4n_rainbow_table_file_name &&
       subghz_keystore_raw_get_data(
           instance->alutech_at_4n_rainbow_table_file_name,
           0,
           table_buf,
           SUBGHZ_ALUTECH_AT_4N_RAINBOW_TABLE_SIZE_BYTES)) {
        table_ptr = table_buf;
    }

    size_t index = 0;
    if(alutech_at4n_counter_mode == 1) {
        // Mode 1 (MCSW-3.3M smart clone, 3-press cycle):
        // Press 1: 0x0000 (x4)
        // Press 2: 0x0001 (x4)
        // Press 3: 0xFFFE (x4) -> 0xFFFF (x4) in a single press to resync across wrap
        instance->encoder.repeat = 1;
        if(instance->generic.cnt == 0xFFFF) {
            instance->generic.cnt = 0x0000;
            subghz_protocol_encoder_alutech_at_4n_upload_reserve(
                instance, ALUTECH_AT_4N_UPLOAD_FRAME);
            subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
            index = subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, 0);
            instance->pack_count = 1;
            instance->pack_repeats[0] = 4;
        } else if(instance->generic.cnt == 0x0000) {
            instance->generic.cnt = 0x0001;
            subghz_protocol_encoder_alutech_at_4n_upload_reserve(
                instance, ALUTECH_AT_4N_UPLOAD_FRAME);
            subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
            index = subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, 0);
            instance->pack_count = 1;
            instance->pack_repeats[0] = 4;
        } else {
            subghz_protocol_encoder_alutech_at_4n_upload_reserve(
                instance, 2u * ALUTECH_AT_4N_UPLOAD_FRAME);
            instance->generic.cnt = 0xFFFE;
            subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
            index = subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, 0);
            instance->generic.cnt = 0xFFFF;
            subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
            index = subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, index);
            instance->pack_count = 2;
            instance->pack_repeats[0] = 4;
            instance->pack_repeats[1] = 4;
        }
    } else {
        // Mode 2 (smart pack, every press sends 0x0000..0x0005 back to back):
        // 0000(x2) -> 0001(x2) -> 0002(x2) -> 0003(x2) -> 0004(x3) -> 0005(x3)
        // Base counter is preserved in the file.
        static const uint8_t smart2_repeats[ALUTECH_AT_4N_PACK_MAX_FRAMES] = {2, 2, 2, 2, 3, 3};
        const uint16_t base_cnt = instance->generic.cnt;
        instance->encoder.repeat = 1;
        subghz_protocol_encoder_alutech_at_4n_upload_reserve(
            instance, ALUTECH_AT_4N_PACK_MAX_FRAMES * ALUTECH_AT_4N_UPLOAD_FRAME);
        for(uint8_t i = 0; i < ALUTECH_AT_4N_PACK_MAX_FRAMES; i++) {
            instance->generic.cnt = i;
            subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
            index = subghz_protocol_encoder_alutech_at_4n_encode_frame(instance, index);
            instance->pack_repeats[i] = smart2_repeats[i];
        }
        instance->pack_count = ALUTECH_AT_4N_PACK_MAX_FRAMES;
        // Restore base counter into the key written back to the file
        instance->generic.cnt = base_cnt;
        subghz_protocol_alutech_at_4n_pack_key(instance, btn, table_ptr);
    }

    instance->pack_frame = 0;
    instance->pack_repeat = instance->pack_repeats[0];
    instance->encoder.size_upload = index;
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

        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint32_t tmp_counter_mode;
        if(flipper_format_read_uint32(flipper_format, "CounterMode", &tmp_counter_mode, 1)) {
            alutech_at4n_counter_mode = (uint8_t)tmp_counter_mode;
        } else {
            alutech_at4n_counter_mode = 0;
        }

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
    SubGhzProtocolDecoderAlutech_at_4n* instance = subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderAlutech_at_4n), &subghz_protocol_alutech_at_4n);
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
        if((instance->header_count > 9) &&
           (DURATION_DIFF(duration, subghz_protocol_alutech_at_4n_const.te_short * 10) <
            subghz_protocol_alutech_at_4n_const.te_delta * 10)) {
            // Found header
            instance->decoder.parser_step = Alutech_at_4nDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
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
                    if(instance->generic.data != instance->generic.data_2) {
                        instance->generic.data = instance->generic.data_2;

                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        instance->crc = instance->decoder.decode_data;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                    instance->decoder.decode_data = 0;
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
                    instance->generic.data_2 = instance->decoder.decode_data;
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
                    instance->generic.data_2 = instance->decoder.decode_data;
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
    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        res = SubGhzProtocolStatusErrorParserOthers;
    }
    if((res == SubGhzProtocolStatusOk) &&
       !flipper_format_insert_or_update_uint32(flipper_format, "CRC", &instance->crc, 1)) {
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
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint32_t tmp_counter_mode;
        if(flipper_format_read_uint32(flipper_format, "CounterMode", &tmp_counter_mode, 1)) {
            alutech_at4n_counter_mode = (uint8_t)tmp_counter_mode;
        } else {
            alutech_at4n_counter_mode = 0;
        }

    } while(false);
    return ret;
}

static uint8_t subghz_protocol_alutech_at_4n_get_btn_code(void) {
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

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 16;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 8;
    //

    furi_string_cat_printf(
        output,
        "%s\r\n"
        "Key:0x%08lX%08lX\nCRC:%02X  %dbit\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:%04lX\r\n",
        instance->generic.protocol_name,
        code_found_hi,
        code_found_lo,
        (uint8_t)instance->crc,
        instance->generic.data_count_bit,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
