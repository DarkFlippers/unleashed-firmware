#include "subghz_history.h"
#include <lib/subghz/receiver.h>
#include <lib/subghz/protocols/came.h>

#include <furi.h>

#define SUBGHZ_HISTORY_MAX       50
#define SUBGHZ_HISTORY_FREE_HEAP 20480

#define TAG "SubGhzHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    uint8_t type;
    SubGhzRadioPreset* preset;
} SubGhzHistoryItem;

ARRAY_DEF(SubGhzHistoryItemArray, SubGhzHistoryItem, M_POD_OPLIST)

#define M_OPL_SubGhzHistoryItemArray_t() ARRAY_OPLIST(SubGhzHistoryItemArray, M_POD_OPLIST)

typedef struct {
    SubGhzHistoryItemArray_t data;
} SubGhzHistoryStruct;

struct SubGhzHistory {
    uint32_t last_update_timestamp;
    uint16_t last_index_write;
    uint8_t code_last_hash_data;
    FuriString* tmp_string;
    SubGhzHistoryStruct* history;
};

SubGhzHistory* subghz_history_alloc(void) {
    SubGhzHistory* instance = malloc(sizeof(SubGhzHistory));
    instance->tmp_string = furi_string_alloc();
    instance->history = malloc(sizeof(SubGhzHistoryStruct));
    SubGhzHistoryItemArray_init(instance->history->data);
    return instance;
}

void subghz_history_free(SubGhzHistory* instance) {
    furi_assert(instance);
    furi_string_free(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, SubGhzHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    SubGhzHistoryItemArray_clear(instance->history->data);
    free(instance->history);
    free(instance);
}

uint32_t subghz_history_get_frequency(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    return item->preset->frequency;
}

SubGhzRadioPreset* subghz_history_get_radio_preset(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    return item->preset;
}

const char* subghz_history_get_preset(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    return furi_string_get_cstr(item->preset->name);
}

void subghz_history_reset(SubGhzHistory* instance) {
    furi_assert(instance);
    furi_string_reset(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, SubGhzHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    SubGhzHistoryItemArray_reset(instance->history->data);
    instance->last_index_write = 0;
    instance->code_last_hash_data = 0;
}

uint16_t subghz_history_get_item(SubGhzHistory* instance) {
    furi_assert(instance);
    return instance->last_index_write;
}

uint8_t subghz_history_get_type_protocol(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    return item->type;
}

const char* subghz_history_get_protocol_name(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    flipper_format_rewind(item->flipper_string);
    if(!flipper_format_read_string(item->flipper_string, "Protocol", instance->tmp_string)) {
        FURI_LOG_E(TAG, "Missing Protocol");
        furi_string_reset(instance->tmp_string);
    }
    return furi_string_get_cstr(instance->tmp_string);
}

FlipperFormat* subghz_history_get_raw_data(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    if(item->flipper_string) {
        return item->flipper_string;
    } else {
        return NULL;
    }
}
bool subghz_history_get_text_space_left(SubGhzHistory* instance, FuriString* output) {
    furi_assert(instance);
    if(memmgr_get_free_heap() < SUBGHZ_HISTORY_FREE_HEAP) {
        if(output != NULL) furi_string_printf(output, "    Free heap LOW");
        return true;
    }
    if(instance->last_index_write == SUBGHZ_HISTORY_MAX) {
        if(output != NULL) furi_string_printf(output, "   Memory is FULL");
        return true;
    }
    if(output != NULL)
        furi_string_printf(output, "%02u/%02u", instance->last_index_write, SUBGHZ_HISTORY_MAX);
    return false;
}

void subghz_history_get_text_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx) {
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    furi_string_set(output, item->item_str);
}

bool subghz_history_add_to_history(
    SubGhzHistory* instance,
    void* context,
    SubGhzRadioPreset* preset) {
    furi_assert(instance);
    furi_assert(context);

    if(memmgr_get_free_heap() < SUBGHZ_HISTORY_FREE_HEAP) return false;
    if(instance->last_index_write >= SUBGHZ_HISTORY_MAX) return false;

    SubGhzProtocolDecoderBase* decoder_base = context;
    if((instance->code_last_hash_data ==
        subghz_protocol_decoder_base_get_hash_data(decoder_base)) &&
       ((furi_get_tick() - instance->last_update_timestamp) < 500)) {
        instance->last_update_timestamp = furi_get_tick();
        return false;
    }

    instance->code_last_hash_data = subghz_protocol_decoder_base_get_hash_data(decoder_base);
    instance->last_update_timestamp = furi_get_tick();

    FuriString* text;
    text = furi_string_alloc();
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_push_raw(instance->history->data);
    item->preset = malloc(sizeof(SubGhzRadioPreset));
    item->type = decoder_base->protocol->type;
    item->preset->frequency = preset->frequency;
    item->preset->name = furi_string_alloc();
    furi_string_set(item->preset->name, preset->name);
    item->preset->data = preset->data;
    item->preset->data_size = preset->data_size;

    item->item_str = furi_string_alloc();
    item->flipper_string = flipper_format_string_alloc();
    subghz_protocol_decoder_base_serialize(decoder_base, item->flipper_string, preset);

    do {
        if(!flipper_format_rewind(item->flipper_string)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_string(item->flipper_string, "Protocol", instance->tmp_string)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        if(!strcmp(furi_string_get_cstr(instance->tmp_string), "KeeLoq")) {
            furi_string_set(instance->tmp_string, "KL ");
            if(!flipper_format_read_string(item->flipper_string, "Manufacture", text)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }
            furi_string_cat(instance->tmp_string, text);
        } else if(!strcmp(furi_string_get_cstr(instance->tmp_string), "Star Line")) {
            furi_string_set(instance->tmp_string, "SL ");
            if(!flipper_format_read_string(item->flipper_string, "Manufacture", text)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }
            furi_string_cat(instance->tmp_string, text);
        }
        if(!flipper_format_rewind(item->flipper_string)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(item->flipper_string, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_D(TAG, "No Key");
        }
        uint64_t data = 0;
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            data = (data << 8) | key_data[i];
        }
        if(data != 0) {
            if(!(uint32_t)(data >> 32)) {
                furi_string_printf(
                    item->item_str,
                    "%s %lX",
                    furi_string_get_cstr(instance->tmp_string),
                    (uint32_t)(data & 0xFFFFFFFF));
            } else {
                furi_string_printf(
                    item->item_str,
                    "%s %lX%08lX",
                    furi_string_get_cstr(instance->tmp_string),
                    (uint32_t)(data >> 32),
                    (uint32_t)(data & 0xFFFFFFFF));
            }
        } else {
            furi_string_printf(item->item_str, "%s", furi_string_get_cstr(instance->tmp_string));
        }

    } while(false);

    furi_string_free(text);
    instance->last_index_write++;
    return true;
}
