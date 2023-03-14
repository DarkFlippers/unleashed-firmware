#include "weather_station_history.h"
#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/receiver.h>
#include "protocols/ws_generic.h"

#include <furi.h>

#define WS_HISTORY_MAX 50
#define TAG "WSHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    uint8_t type;
    uint32_t id;
    SubGhzRadioPreset* preset;
} WSHistoryItem;

ARRAY_DEF(WSHistoryItemArray, WSHistoryItem, M_POD_OPLIST)

#define M_OPL_WSHistoryItemArray_t() ARRAY_OPLIST(WSHistoryItemArray, M_POD_OPLIST)

typedef struct {
    WSHistoryItemArray_t data;
} WSHistoryStruct;

struct WSHistory {
    uint32_t last_update_timestamp;
    uint16_t last_index_write;
    uint8_t code_last_hash_data;
    FuriString* tmp_string;
    WSHistoryStruct* history;
};

WSHistory* ws_history_alloc(void) {
    WSHistory* instance = malloc(sizeof(WSHistory));
    instance->tmp_string = furi_string_alloc();
    instance->history = malloc(sizeof(WSHistoryStruct));
    WSHistoryItemArray_init(instance->history->data);
    return instance;
}

void ws_history_free(WSHistory* instance) {
    furi_assert(instance);
    furi_string_free(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, WSHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    WSHistoryItemArray_clear(instance->history->data);
    free(instance->history);
    free(instance);
}

uint32_t ws_history_get_frequency(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    return item->preset->frequency;
}

SubGhzRadioPreset* ws_history_get_radio_preset(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    return item->preset;
}

const char* ws_history_get_preset(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    return furi_string_get_cstr(item->preset->name);
}

void ws_history_reset(WSHistory* instance) {
    furi_assert(instance);
    furi_string_reset(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, WSHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    WSHistoryItemArray_reset(instance->history->data);
    instance->last_index_write = 0;
    instance->code_last_hash_data = 0;
}

uint16_t ws_history_get_item(WSHistory* instance) {
    furi_assert(instance);
    return instance->last_index_write;
}

uint8_t ws_history_get_type_protocol(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    return item->type;
}

const char* ws_history_get_protocol_name(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    flipper_format_rewind(item->flipper_string);
    if(!flipper_format_read_string(item->flipper_string, "Protocol", instance->tmp_string)) {
        FURI_LOG_E(TAG, "Missing Protocol");
        furi_string_reset(instance->tmp_string);
    }
    return furi_string_get_cstr(instance->tmp_string);
}

FlipperFormat* ws_history_get_raw_data(WSHistory* instance, uint16_t idx) {
    furi_assert(instance);
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    if(item->flipper_string) {
        return item->flipper_string;
    } else {
        return NULL;
    }
}
bool ws_history_get_text_space_left(WSHistory* instance, FuriString* output) {
    furi_assert(instance);
    if(instance->last_index_write == WS_HISTORY_MAX) {
        if(output != NULL) furi_string_printf(output, "Memory is FULL");
        return true;
    }
    if(output != NULL)
        furi_string_printf(output, "%02u/%02u", instance->last_index_write, WS_HISTORY_MAX);
    return false;
}

void ws_history_get_text_item_menu(WSHistory* instance, FuriString* output, uint16_t idx) {
    WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, idx);
    furi_string_set(output, item->item_str);
}

WSHistoryStateAddKey
    ws_history_add_to_history(WSHistory* instance, void* context, SubGhzRadioPreset* preset) {
    furi_assert(instance);
    furi_assert(context);

    if(instance->last_index_write >= WS_HISTORY_MAX) return WSHistoryStateAddKeyOverflow;

    SubGhzProtocolDecoderBase* decoder_base = context;
    if((instance->code_last_hash_data ==
        subghz_protocol_decoder_base_get_hash_data(decoder_base)) &&
       ((furi_get_tick() - instance->last_update_timestamp) < 500)) {
        instance->last_update_timestamp = furi_get_tick();
        return WSHistoryStateAddKeyTimeOut;
    }

    instance->code_last_hash_data = subghz_protocol_decoder_base_get_hash_data(decoder_base);
    instance->last_update_timestamp = furi_get_tick();

    FlipperFormat* fff = flipper_format_string_alloc();
    uint32_t id = 0;
    subghz_protocol_decoder_base_serialize(decoder_base, fff, preset);

    do {
        if(!flipper_format_rewind(fff)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(fff, "Id", (uint32_t*)&id, 1)) {
            FURI_LOG_E(TAG, "Missing Id");
            break;
        }
    } while(false);
    flipper_format_free(fff);

    //Update record if found
    bool sensor_found = false;
    for(size_t i = 0; i < WSHistoryItemArray_size(instance->history->data); i++) {
        WSHistoryItem* item = WSHistoryItemArray_get(instance->history->data, i);
        if(item->id == id) {
            sensor_found = true;
            Stream* flipper_string_stream = flipper_format_get_raw_stream(item->flipper_string);
            stream_clean(flipper_string_stream);
            subghz_protocol_decoder_base_serialize(decoder_base, item->flipper_string, preset);
            return WSHistoryStateAddKeyUpdateData;
        }
    }

    // or add new record
    if(!sensor_found) { //-V547
        WSHistoryItem* item = WSHistoryItemArray_push_raw(instance->history->data);
        item->preset = malloc(sizeof(SubGhzRadioPreset));
        item->type = decoder_base->protocol->type;
        item->preset->frequency = preset->frequency;
        item->preset->name = furi_string_alloc();
        furi_string_set(item->preset->name, preset->name);
        item->preset->data = preset->data;
        item->preset->data_size = preset->data_size;
        item->id = id;

        item->item_str = furi_string_alloc();
        item->flipper_string = flipper_format_string_alloc();
        subghz_protocol_decoder_base_serialize(decoder_base, item->flipper_string, preset);

        do {
            if(!flipper_format_rewind(item->flipper_string)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            if(!flipper_format_read_string(
                   item->flipper_string, "Protocol", instance->tmp_string)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }

            if(!flipper_format_rewind(item->flipper_string)) {
                FURI_LOG_E(TAG, "Rewind error");
                break;
            }
            uint8_t key_data[sizeof(uint64_t)] = {0};
            if(!flipper_format_read_hex(item->flipper_string, "Data", key_data, sizeof(uint64_t))) {
                FURI_LOG_E(TAG, "Missing Data");
                break;
            }
            uint64_t data = 0;
            for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
                data = (data << 8) | key_data[i];
            }
            uint32_t temp_data = 0;
            if(!flipper_format_read_uint32(item->flipper_string, "Ch", (uint32_t*)&temp_data, 1)) {
                FURI_LOG_E(TAG, "Missing Channel");
                break;
            }
            if(temp_data != WS_NO_CHANNEL) {
                furi_string_cat_printf(instance->tmp_string, " Ch:%X", (uint8_t)temp_data);
            }

            furi_string_printf(
                item->item_str, "%s %llX", furi_string_get_cstr(instance->tmp_string), data);

        } while(false);
        instance->last_index_write++;
        return WSHistoryStateAddKeyNewDada;
    }
    return WSHistoryStateAddKeyUnknown;
}
