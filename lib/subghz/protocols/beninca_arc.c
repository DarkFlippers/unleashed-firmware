#include "beninca_arc.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "core/log.h"
#include <stddef.h>
#include <stdint.h>
#include "aes_common.h"

#include "../blocks/custom_btn_i.h"

#define TAG "BenincaARC"

#define BENINCA_ARC_KEY_TYPE 9u

static const SubGhzBlockConst subghz_protocol_beninca_arc_const = {
    .te_short = 300,
    .te_long = 600,
    .te_delta = 155,
    .min_count_bit_for_found = 128,
};

typedef enum {
    BenincaARCDecoderStart = 0,
    BenincaARCDecoderHighLevel,
    BenincaARCDecoderLowLevel,
} BenincaARCDecoderState;

struct SubGhzProtocolDecoderBenincaARC {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;

    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
};

struct SubGhzProtocolEncoderBenincaARC {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;

    SubGhzBlockGeneric generic;

    SubGhzKeystore* keystore;
};

const SubGhzProtocolDecoder subghz_protocol_beninca_arc_decoder = {
    .alloc = subghz_protocol_decoder_beninca_arc_alloc,
    .free = subghz_protocol_decoder_beninca_arc_free,

    .feed = subghz_protocol_decoder_beninca_arc_feed,
    .reset = subghz_protocol_decoder_beninca_arc_reset,

    .get_hash_data = subghz_protocol_decoder_beninca_arc_get_hash_data,
    .serialize = subghz_protocol_decoder_beninca_arc_serialize,
    .deserialize = subghz_protocol_decoder_beninca_arc_deserialize,
    .get_string = subghz_protocol_decoder_beninca_arc_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_beninca_arc_encoder = {
    .alloc = subghz_protocol_encoder_beninca_arc_alloc,
    .free = subghz_protocol_encoder_beninca_arc_free,

    .deserialize = subghz_protocol_encoder_beninca_arc_deserialize,
    .stop = subghz_protocol_encoder_beninca_arc_stop,
    .yield = subghz_protocol_encoder_beninca_arc_yield,
};

const SubGhzProtocol subghz_protocol_beninca_arc = {
    .name = SUBGHZ_PROTOCOL_BENINCA_ARC_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_beninca_arc_decoder,
    .encoder = &subghz_protocol_beninca_arc_encoder,
};

// Get custom button code
static uint8_t subghz_protocol_beninca_arc_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x02:
            btn = 0x04;
            break;
        case 0x04:
            btn = 0x02;
            break;
        case 0x00:
            btn = 0x04;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x02:
            btn = 0x00;
            break;
        case 0x04:
            btn = 0x00;
            break;
        case 0x00:
            btn = 0x02;
            break;

        default:
            break;
        }
    }

    return btn;
}

static void get_subghz_protocol_beninca_arc_aes_key(SubGhzKeystore* keystore, uint8_t* aes_key) {
    uint64_t mfkey = 0;
    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(keystore), SubGhzKeyArray_t) {
            if(manufacture_code->type == BENINCA_ARC_KEY_TYPE) {
                mfkey = manufacture_code->key;
                break;
            }
        }

    uint32_t derived_lo = (uint32_t)(mfkey & 0xFFFFFFFF);
    uint32_t derived_hi = (uint32_t)((mfkey >> 32) & 0xFFFFFFFF);

    uint64_t val64_a = ((uint64_t)derived_hi << 32) | derived_lo;
    for(uint8_t i = 0; i < 8; i++) {
        aes_key[i] = (val64_a >> (56 - i * 8)) & 0xFF;
    }

    uint32_t new_lo = ((derived_hi >> 24) & 0xFF) | ((derived_hi >> 8) & 0xFF00) |
                      ((derived_hi << 8) & 0xFF0000) | ((derived_hi << 24) & 0xFF000000);
    uint32_t new_hi = ((derived_lo >> 24) & 0xFF) | ((derived_lo >> 8) & 0xFF00) |
                      ((derived_lo << 8) & 0xFF0000) | ((derived_lo << 24) & 0xFF000000);

    uint64_t val64_b = ((uint64_t)new_hi << 32) | new_lo;
    for(uint8_t i = 0; i < 8; i++) {
        aes_key[i + 8] = (val64_b >> (56 - i * 8)) & 0xFF;
    }
}

static uint64_t
    subghz_protocol_beninca_arc_decrypt(SubGhzBlockGeneric* generic, SubGhzKeystore* keystore) {
    // Beninca ARC Decoder
    // 01.2026 - @xMasterX (MMX) & @zero-mega

    // Decrypt data
    uint8_t encrypted_data[16];

    for(uint8_t i = 0; i < 8; i++) {
        encrypted_data[i] = (generic->data >> (56 - i * 8)) & 0xFF;
        encrypted_data[i + 8] = (generic->data_2 >> (56 - i * 8)) & 0xFF;
    }

    reverse_bits_in_bytes(encrypted_data, 16);

    uint8_t aes_key[16];
    get_subghz_protocol_beninca_arc_aes_key(keystore, aes_key);

    uint8_t expanded_key[176];
    aes_key_expansion(aes_key, expanded_key);

    aes128_decrypt(expanded_key, encrypted_data);

    // Serial number of remote
    generic->serial = ((uint32_t)encrypted_data[0] << 24) | ((uint32_t)encrypted_data[1] << 16) |
                      ((uint32_t)encrypted_data[2] << 8) | encrypted_data[3];

    // Button code
    generic->btn = encrypted_data[4];

    // Middle bytes contains mini counter that is increased while button is held
    // its value mostly stored in encrypted_data[9] but might be in other bytes as well
    // In order to support all variants we read all middle bytes as uint64_t
    // In case you have the remote with ARC rolling code please share RAW recording where you hold button for 15+ sec with us to improve this part!
    uint64_t middle_bytes = 0;
    middle_bytes = ((uint64_t)encrypted_data[5] << 32) | ((uint64_t)encrypted_data[6] << 24) |
                   ((uint64_t)encrypted_data[7] << 16) | ((uint64_t)encrypted_data[8] << 8) |
                   encrypted_data[9];

    // 32-bit counter
    generic->cnt = ((uint32_t)encrypted_data[10] << 24) | ((uint32_t)encrypted_data[11] << 16) |
                   ((uint32_t)encrypted_data[12] << 8) | encrypted_data[13];
    // Fixed constant value AA 55
    generic->seed = ((uint16_t)encrypted_data[14] << 8) | encrypted_data[15];

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(generic->btn);
    }
    subghz_custom_btn_set_max(2);

    return middle_bytes;
}

static void subghz_protocol_beninca_arc_encrypt(
    SubGhzBlockGeneric* generic,
    SubGhzKeystore* keystore,
    uint64_t middle_bytes) {
    // Beninca ARC Encoder
    // 01.2026 - @xMasterX (MMX) & @zero-mega
    // Encrypt data
    uint8_t plaintext[16];

    plaintext[0] = (generic->serial >> 24) & 0xFF;
    plaintext[1] = (generic->serial >> 16) & 0xFF;
    plaintext[2] = (generic->serial >> 8) & 0xFF;
    plaintext[3] = generic->serial & 0xFF;
    plaintext[4] = generic->btn;
    plaintext[5] = (middle_bytes >> 32) & 0xFF;
    plaintext[6] = (middle_bytes >> 24) & 0xFF;
    plaintext[7] = (middle_bytes >> 16) & 0xFF;
    plaintext[8] = (middle_bytes >> 8) & 0xFF;
    plaintext[9] = middle_bytes & 0xFF;
    plaintext[10] = (generic->cnt >> 24) & 0xFF;
    plaintext[11] = (generic->cnt >> 16) & 0xFF;
    plaintext[12] = (generic->cnt >> 8) & 0xFF;
    plaintext[13] = generic->cnt & 0xFF;
    plaintext[14] = (generic->seed >> 8) & 0xFF;
    plaintext[15] = generic->seed & 0xFF;

    uint8_t aes_key[16];
    get_subghz_protocol_beninca_arc_aes_key(keystore, aes_key);

    uint8_t expanded_key[176];
    aes_key_expansion(aes_key, expanded_key);

    aes128_encrypt(expanded_key, plaintext);

    reverse_bits_in_bytes(plaintext, 16);

    for(uint8_t i = 0; i < 8; i++) {
        generic->data = (generic->data << 8) | plaintext[i];
        generic->data_2 = (generic->data_2 << 8) | plaintext[i + 8];
    }
    return;
}

void* subghz_protocol_encoder_beninca_arc_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderBenincaARC* instance = malloc(sizeof(SubGhzProtocolEncoderBenincaARC));

    instance->base.protocol = &subghz_protocol_beninca_arc;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);

    instance->encoder.repeat = 1;
    instance->encoder.size_upload = 800;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;

    return instance;
}

void subghz_protocol_encoder_beninca_arc_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderBenincaARC* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_encoder_beninca_arc_stop(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderBenincaARC* instance = context;
    instance->encoder.is_running = false;
}

static void subghz_protocol_beninca_arc_encoder_get_upload(
    SubGhzProtocolEncoderBenincaARC* instance,
    size_t* index) {
    furi_assert(instance);
    size_t index_local = *index;

    // First part of data 64 bits
    for(uint8_t i = 64; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index_local++] =
                level_duration_make(true, (uint32_t)subghz_protocol_beninca_arc_const.te_short);
            instance->encoder.upload[index_local++] =
                level_duration_make(false, (uint32_t)subghz_protocol_beninca_arc_const.te_long);
        } else {
            // Send bit 0
            instance->encoder.upload[index_local++] =
                level_duration_make(true, (uint32_t)subghz_protocol_beninca_arc_const.te_long);
            instance->encoder.upload[index_local++] =
                level_duration_make(false, (uint32_t)subghz_protocol_beninca_arc_const.te_short);
        }
    }
    // Second part of data 64 bits - total 128bits data
    for(uint8_t i = 64; i > 0; i--) {
        if(bit_read(instance->generic.data_2, i - 1)) {
            // Send bit 1
            instance->encoder.upload[index_local++] =
                level_duration_make(true, (uint32_t)subghz_protocol_beninca_arc_const.te_short);
            instance->encoder.upload[index_local++] =
                level_duration_make(false, (uint32_t)subghz_protocol_beninca_arc_const.te_long);
        } else {
            // Send bit 0
            instance->encoder.upload[index_local++] =
                level_duration_make(true, (uint32_t)subghz_protocol_beninca_arc_const.te_long);
            instance->encoder.upload[index_local++] =
                level_duration_make(false, (uint32_t)subghz_protocol_beninca_arc_const.te_short);
        }
    }
    // Add stop bit
    instance->encoder.upload[index_local++] =
        level_duration_make(true, (uint32_t)subghz_protocol_beninca_arc_const.te_short);
    // Add gap between packets
    instance->encoder.upload[index_local++] =
        level_duration_make(false, (uint32_t)subghz_protocol_beninca_arc_const.te_long * 15);

    *index = index_local;
}

static void subghz_protocol_beninca_arc_encoder_prepare_packets(
    SubGhzProtocolEncoderBenincaARC* instance) {
    furi_assert(instance);

    // Counter increment
    // check OFEX mode
    if(furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF) {
        // standart counter mode. PULL data from subghz_block_generic_global variables
        if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
            // if counter_override_get return FALSE then counter was not changed and we increase counter by standart mult value
            if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFFFFFF) {
                instance->generic.cnt = 0;
            } else {
                instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        }
    } else {
        // TODO: OFEX mode
        instance->generic.cnt += 1;
    }
    // Index for upload array
    size_t index = 0;
    // Generate new key using custom or default button
    instance->generic.btn = subghz_protocol_beninca_arc_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&instance->generic.btn))
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", instance->generic.btn);

    // Make 3 packets with different mini counter values - 2, 4, 6
    for(uint8_t i = 0; i < 3; i++) {
        subghz_protocol_beninca_arc_encrypt(
            &instance->generic, instance->keystore, (uint64_t)((i + 1) * 2));
        subghz_protocol_beninca_arc_encoder_get_upload(instance, &index);
    }
    // Set final size of upload array
    instance->encoder.size_upload = index;
}

bool subghz_protocol_beninca_arc_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    // UwU
    SubGhzProtocolEncoderBenincaARC* instance = context;
    instance->generic.serial = serial;
    instance->generic.btn = btn; // 02 / 04
    instance->generic.cnt = cnt;
    instance->generic.seed = 0xAA55; // Fixed value constant
    instance->generic.data_count_bit = 128;

    subghz_protocol_beninca_arc_encrypt(&instance->generic, instance->keystore, 0x1);

    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> (i * 8)) & 0xFF;
    }

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        res = SubGhzProtocolStatusErrorParserOthers;
    }

    if((res == SubGhzProtocolStatusOk) &&
       !flipper_format_insert_or_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Data2");
        res = SubGhzProtocolStatusErrorParserOthers;
    }

    return res == SubGhzProtocolStatusOk;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_beninca_arc_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderBenincaARC* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        if(SubGhzProtocolStatusOk !=
           subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }

        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            break;
        }

        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }

        // TODO: if minicounter having larger value use it instead of fixed values
        subghz_protocol_beninca_arc_decrypt(&instance->generic, instance->keystore);

        subghz_protocol_beninca_arc_encoder_prepare_packets(instance);

        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Data");
            break;
        }

        instance->encoder.is_running = true;

        res = SubGhzProtocolStatusOk;
    } while(false);

    return res;
}

LevelDuration subghz_protocol_encoder_beninca_arc_yield(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderBenincaARC* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        if(!subghz_block_generic_global.endless_tx) instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_beninca_arc_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderBenincaARC* instance = malloc(sizeof(SubGhzProtocolDecoderBenincaARC));
    instance->base.protocol = &subghz_protocol_beninca_arc;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->keystore = subghz_environment_get_keystore(environment);
    instance->decoder.parser_step = BenincaARCDecoderStart;
    return instance;
}

void subghz_protocol_decoder_beninca_arc_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;
    free(instance);
}

void subghz_protocol_decoder_beninca_arc_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;
    instance->decoder.parser_step = BenincaARCDecoderStart;
}

void subghz_protocol_decoder_beninca_arc_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;

    switch(instance->decoder.parser_step) {
    case BenincaARCDecoderStart:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_beninca_arc_const.te_long * 16) <
                        subghz_protocol_beninca_arc_const.te_delta * 15)) {
            // GAP (9300 +- 2325 us) found switch to next state
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = BenincaARCDecoderHighLevel;
            break;
        }
        // No GAP so stay in current state
        break;
    case BenincaARCDecoderHighLevel:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = BenincaARCDecoderLowLevel;
            // Check if we have collected enough bits
            if((instance->decoder.decode_count_bit ==
                (subghz_protocol_beninca_arc_const.min_count_bit_for_found / 2)) &&
               (instance->decoder.decode_data != 0)) {
                // Half data captured 64 bits
                instance->generic.data = instance->decoder.decode_data;
                instance->decoder.decode_data = 0;
            } else if(
                instance->decoder.decode_count_bit ==
                subghz_protocol_beninca_arc_const.min_count_bit_for_found) {
                // Full data captured 128 bits
                instance->generic.data_2 = instance->decoder.decode_data;
                instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                instance->decoder.parser_step = BenincaARCDecoderStart;

                if(instance->base.callback) {
                    instance->base.callback(&instance->base, instance->base.context);
                }

                break;
            }
        } else {
            instance->decoder.parser_step = BenincaARCDecoderStart;
        }
        break;
    case BenincaARCDecoderLowLevel:
        if(!level) {
            // Bit 1 is short and long timing = 300us HIGH (te_last) and 600us LOW
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_beninca_arc_const.te_short) <
                subghz_protocol_beninca_arc_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_beninca_arc_const.te_long) <
                subghz_protocol_beninca_arc_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = BenincaARCDecoderHighLevel;
                // Bit 0 is long and short timing = 600us HIGH (te_last) and 300us LOW
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_beninca_arc_const.te_long) <
                 subghz_protocol_beninca_arc_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_beninca_arc_const.te_short) <
                 subghz_protocol_beninca_arc_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = BenincaARCDecoderHighLevel;
            } else {
                instance->decoder.parser_step = BenincaARCDecoderStart;
            }
            break;
        } else {
            instance->decoder.parser_step = BenincaARCDecoderStart;
            break;
        }
    }
}

uint8_t subghz_protocol_decoder_beninca_arc_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_beninca_arc_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data_2 >> (i * 8)) & 0xFF;
    }

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }

    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_insert_or_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Data");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }
    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_beninca_arc_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;

    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_beninca_arc_const.min_count_bit_for_found);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Data");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }

        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->generic.data_2 = instance->generic.data_2 << 8 | key_data[i];
        }
    } while(false);
    return ret;
}

void subghz_protocol_decoder_beninca_arc_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderBenincaARC* instance = context;

    uint64_t middle_bytes_dec =
        subghz_protocol_beninca_arc_decrypt(&instance->generic, instance->keystore);

    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 32;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 8;
    //

    furi_string_printf(
        output,
        "%s %db\r\n"
        "Key1:%08llX\r\n"
        "Key2:%08llX\r\n"
        "Sn:%08lX Btn:%02X\r\n"
        "Mc:%0lX Cnt:%0lX\r\n"
        "Fx:%04lX",
        instance->base.protocol->name,
        instance->generic.data_count_bit,
        instance->generic.data,
        instance->generic.data_2,
        instance->generic.serial,
        instance->generic.btn,
        (uint32_t)(middle_bytes_dec & 0xFFFFFFFF),
        instance->generic.cnt,
        instance->generic.seed & 0xFFFF);
}
