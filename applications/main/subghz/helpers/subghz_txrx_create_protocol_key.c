#include "subghz_txrx_i.h" // IWYU pragma: keep
#include "subghz_txrx_create_protocol_key.h"
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/secplus_v1.h>
#include <lib/subghz/protocols/secplus_v2.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/raw.h>

#define TAG "SubGhzCreateProtocolKey"

SubGhzProtocolStatus subghz_txrx_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit) {
    furi_assert(context);
    SubGhzTxRx* instance = context;

    SubGhzProtocolStatus ret = SubGhzProtocolStatusOk;

    subghz_txrx_set_preset(instance, preset_name, frequency, NULL, 0);
    instance->decoder_result =
        subghz_receiver_search_decoder_base_by_name(instance->receiver, protocol_name);

    if(instance->decoder_result == NULL) {
        FURI_LOG_E(TAG, "Protocol not found!");
        ret = SubGhzProtocolStatusErrorProtocolNotFound;
        return ret;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(instance->fff_data);
        stream_clean(fff_data_stream);
        ret = subghz_protocol_decoder_base_serialize(
            instance->decoder_result, instance->fff_data, instance->preset);
        if(ret != SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(instance->fff_data, "Bit", &bit, 1)) {
            ret = SubGhzProtocolStatusErrorParserOthers;
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(instance->fff_data, "Key", key_data, sizeof(uint64_t))) {
            ret = SubGhzProtocolStatusErrorParserOthers;
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
    } while(false);
    return ret;
}

SubGhzProtocolStatus subghz_txrx_gen_data_protocol_and_te(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te) {
    furi_assert(instance);
    SubGhzProtocolStatus ret =
        subghz_txrx_gen_data_protocol(instance, preset_name, frequency, protocol_name, key, bit);
    if(ret == SubGhzProtocolStatusOk) {
        if(!flipper_format_update_uint32(instance->fff_data, "TE", (uint32_t*)&te, 1)) {
            ret = SubGhzProtocolStatusErrorParserOthers;
            FURI_LOG_E(TAG, "Unable to update Te");
        }
    }
    if(ret == SubGhzProtocolStatusOk) {
        uint32_t guard_time = 30;
        if(!flipper_format_update_uint32(
               instance->fff_data, "Guard_time", (uint32_t*)&guard_time, 1)) {
            ret = SubGhzProtocolStatusErrorParserOthers;
            FURI_LOG_E(TAG, "Unable to update Guard_time");
        }
    }
    return ret;
}

SubGhzProtocolStatus subghz_txrx_gen_keeloq_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency,
    const char* name_sysmem,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt) {
    furi_assert(instance);

    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    serial &= 0x0FFFFFFF;
    instance->transmitter =
        subghz_transmitter_alloc_init(instance->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_txrx_set_preset(instance, name_preset, frequency, NULL, 0);
    if(instance->transmitter) {
        subghz_protocol_keeloq_create_data(
            subghz_transmitter_get_protocol_instance(instance->transmitter),
            instance->fff_data,
            serial,
            btn,
            cnt,
            name_sysmem,
            instance->preset);
        ret = SubGhzProtocolStatusOk;
    }
    subghz_transmitter_free(instance->transmitter);
    return ret;
}

SubGhzProtocolStatus subghz_txrx_gen_secplus_v2_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt) {
    furi_assert(instance);

    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
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
        ret = SubGhzProtocolStatusOk;
    }
    return ret;
}

SubGhzProtocolStatus subghz_txrx_gen_secplus_v1_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency) {
    furi_assert(instance);

    uint32_t serial = (uint32_t)rand();
    while(!subghz_protocol_secplus_v1_check_fixed(serial)) {
        serial = (uint32_t)rand();
    }

    return subghz_txrx_gen_data_protocol(
        instance,
        name_preset,
        frequency,
        SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
        (uint64_t)serial << 32 | 0xE6000000,
        42);
}
