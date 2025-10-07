#include "subghz_txrx_i.h" // IWYU pragma: keep
#include "subghz_txrx_create_protocol_key.h"
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/secplus_v1.h>
#include <lib/subghz/protocols/secplus_v2.h>
#include <lib/subghz/protocols/nice_flor_s.h>
#include <lib/subghz/protocols/marantec.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/raw.h>

#define TAG "SubGhzCreateProtocolKey"

bool subghz_txrx_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit) {
    furi_assert(context);
    SubGhzTxRx* instance = context;

    bool res = false;

    subghz_txrx_set_preset(instance, preset_name, frequency, NULL, 0);
    instance->decoder_result =
        subghz_receiver_search_decoder_base_by_name(instance->receiver, protocol_name);

    if(instance->decoder_result == NULL) {
        //TODO FL-3502: Error
        // furi_string_set(error_str, "Protocol not\nfound!");
        // scene_manager_next_scene(scene_manager, SubGhzSceneShowErrorSub);
        FURI_LOG_E(TAG, "Protocol not found!");
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(instance->fff_data);
        stream_clean(fff_data_stream);
        if(subghz_protocol_decoder_base_serialize(
               instance->decoder_result, instance->fff_data, instance->preset) !=
           SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(instance->fff_data, "Bit", &bit, 1)) {
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(instance->fff_data, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
        res = true;
    } while(false);
    return res;
}

bool subghz_txrx_gen_data_protocol_and_te(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te) {
    furi_assert(instance);
    bool ret = false;
    if(subghz_txrx_gen_data_protocol(instance, preset_name, frequency, protocol_name, key, bit)) {
        if(!flipper_format_update_uint32(instance->fff_data, "TE", (uint32_t*)&te, 1)) {
            FURI_LOG_E(TAG, "Unable to update Te");
        } else {
            ret = true;
        }
    }
    if(ret) {
        uint32_t guard_time = 30;
        if(!flipper_format_update_uint32(
               instance->fff_data, "Guard_time", (uint32_t*)&guard_time, 1)) {
            ret = false;
            FURI_LOG_E(TAG, "Unable to update Guard_time");
        }
    }
    return ret;
}

bool subghz_txrx_gen_keeloq_protocol( //TODO lead to a general appearance
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name) {
    furi_assert(instance);

    bool res = false;

    instance->transmitter =
        subghz_transmitter_alloc_init(instance->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_txrx_set_preset(instance, preset_name, frequency, NULL, 0);

    if(instance->transmitter &&
       subghz_protocol_keeloq_create_data(
           subghz_transmitter_get_protocol_instance(instance->transmitter),
           instance->fff_data,
           serial,
           btn,
           cnt,
           manufacture_name,
           instance->preset)) {
        flipper_format_write_string_cstr(instance->fff_data, "Manufacture", manufacture_name);
        res = true;
    }
    subghz_transmitter_free(instance->transmitter);
    return res;
}

bool subghz_txrx_gen_keeloq_bft_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_keeloq_bft_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                seed,
                                manufacture_name,
                                txrx->preset)) {
        res = true;
    }

    if(res) {
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
        }

        flipper_format_write_hex(txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));

        flipper_format_write_string_cstr(txrx->fff_data, "Manufacture", "BFT");
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_nice_flor_s_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    bool nice_one) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_nice_flor_s_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset,
                                nice_one)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_faac_slh_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    uint32_t seed,
    const char* manufacture_name) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_FAAC_SLH_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_faac_slh_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                seed,
                                manufacture_name,
                                txrx->preset)) {
        res = true;
    }

    if(res) {
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
        }

        bool tmp_allow_zero_seed = true;
        flipper_format_write_hex(txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));
        flipper_format_write_bool(txrx->fff_data, "AllowZeroSeed", &tmp_allow_zero_seed, 1);
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_alutech_at_4n_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_alutech_at_4n_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_came_atomo_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_CAME_ATOMO_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_came_atomo_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_somfy_telis_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_somfy_telis_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_secplus_v2_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt) {
    furi_assert(instance);

    bool ret = false;
    instance->transmitter =
        subghz_transmitter_alloc_init(instance->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
    subghz_txrx_set_preset(instance, name_preset, frequency, NULL, 0);
    if(instance->transmitter) {
        subghz_protocol_secplus_v2_create_data(
            subghz_transmitter_get_protocol_instance(instance->transmitter),
            instance->fff_data,
            serial,
            btn,
            cnt,
            instance->preset);
        ret = true;
    }
    return ret;
}

bool subghz_txrx_gen_secplus_v1_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency) {
    furi_assert(instance);

    bool ret = false;
    uint32_t serial = (uint32_t)rand();
    while(!subghz_protocol_secplus_v1_check_fixed(serial)) {
        serial = (uint32_t)rand();
    }
    if(subghz_txrx_gen_data_protocol(
           instance,
           name_preset,
           frequency,
           SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
           (uint64_t)serial << 32 | 0xE6000000,
           42)) {
        ret = true;
    }
    return ret;
}

bool subghz_txrx_gen_phoenix_v2_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_PHOENIX_V2_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_phoenix_v2_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

void subghz_txrx_gen_serial_gangqi(uint64_t* result_key) {
    uint64_t randkey = (uint64_t)rand();
    uint16_t serial = (uint16_t)((randkey) & 0xFFFF);
    uint8_t const_and_button = (uint8_t)(0xD0 | 0xD);
    uint8_t serial_high = (uint8_t)(serial >> 8);
    uint8_t serial_low = (uint8_t)(serial & 0xFF);
    uint8_t bytesum = (uint8_t)(0xC8 - serial_high - serial_low - const_and_button);

    // Add bytesum to the end
    // serial | const_and_button
    *result_key = ((uint64_t)serial << 18) | ((uint64_t)const_and_button << 10) |
                  ((uint64_t)bytesum << 2);
}

void subghz_txrx_gen_key_marantec(uint64_t* result_key) {
    uint64_t randkey = (uint64_t)rand();
    uint32_t serial = (uint32_t)((randkey) & 0xFFFFF);
    // 0x130 is the constant
    // 0x4 is the button code
    // 0x86 is the serial constant
    // serial is random value that we pre generate above
    // At the end we will put the crc sum
    uint64_t full_key_no_crc = (uint64_t)((uint64_t)0x130 << 40 | (uint64_t)serial << 20 |
                                          (uint64_t)0x4 << 16 | (uint64_t)0x86 << 8);

    uint8_t tdata[6] = {
        full_key_no_crc >> 48,
        full_key_no_crc >> 40,
        full_key_no_crc >> 32,
        full_key_no_crc >> 24,
        full_key_no_crc >> 16,
        full_key_no_crc >> 8};

    uint8_t crc = subghz_protocol_marantec_crc8(tdata, sizeof(tdata));

    *result_key = ((full_key_no_crc >> 8) << 8) | crc;
}
