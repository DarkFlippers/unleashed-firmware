#include "pocsag_pager_history.h"
#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/receiver.h>
#include "protocols/pcsg_generic.h"

#include <furi.h>

#define PCSG_HISTORY_MAX 50
#define TAG "PCSGHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    uint8_t type;
    SubGhzRadioPreset* preset;
} PCSGHistoryItem;

ARRAY_DEF(PCSGHistoryItemArray, PCSGHistoryItem, M_POD_OPLIST)

#define M_OPL_PCSGHistoryItemArray_t() ARRAY_OPLIST(PCSGHistoryItemArray, M_POD_OPLIST)

typedef struct {
    PCSGHistoryItemArray_t data;
} PCSGHistoryStruct;

struct PCSGHistory {
    uint32_t last_update_timestamp;
    uint16_t last_index_write;
    uint8_t code_last_hash_data;
    FuriString* tmp_string;
    PCSGHistoryStruct* history;
};

PCSGHistory* pcsg_history_alloc(void) {
    PCSGHistory* instance = malloc(sizeof(PCSGHistory));
    instance->tmp_string = furi_string_alloc();
    instance->history = malloc(sizeof(PCSGHistoryStruct));
    PCSGHistoryItemArray_init(instance->history->data);
    return instance;
}

void pcsg_history_free(PCSGHistory* instance) {
    furi_assert(instance);
    furi_string_free(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, PCSGHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    PCSGHistoryItemArray_clear(instance->history->data);
    free(instance->history);
    free(instance);
}

uint32_t pcsg_history_get_frequency(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    return item->preset->frequency;
}

SubGhzRadioPreset* pcsg_history_get_radio_preset(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    return item->preset;
}

const char* pcsg_history_get_preset(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    return furi_string_get_cstr(item->preset->name);
}

void pcsg_history_reset(PCSGHistory* instance) {
    furi_assert(instance);
    furi_string_reset(instance->tmp_string);
    for
        M_EACH(item, instance->history->data, PCSGHistoryItemArray_t) {
            furi_string_free(item->item_str);
            furi_string_free(item->preset->name);
            free(item->preset);
            flipper_format_free(item->flipper_string);
            item->type = 0;
        }
    PCSGHistoryItemArray_reset(instance->history->data);
    instance->last_index_write = 0;
    instance->code_last_hash_data = 0;
}

uint16_t pcsg_history_get_item(PCSGHistory* instance) {
    furi_assert(instance);
    return instance->last_index_write;
}

uint8_t pcsg_history_get_type_protocol(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    return item->type;
}

const char* pcsg_history_get_protocol_name(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    flipper_format_rewind(item->flipper_string);
    if(!flipper_format_read_string(item->flipper_string, "Protocol", instance->tmp_string)) {
        FURI_LOG_E(TAG, "Missing Protocol");
        furi_string_reset(instance->tmp_string);
    }
    return furi_string_get_cstr(instance->tmp_string);
}

FlipperFormat* pcsg_history_get_raw_data(PCSGHistory* instance, uint16_t idx) {
    furi_assert(instance);
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    if(item->flipper_string) {
        return item->flipper_string;
    } else {
        return NULL;
    }
}
bool pcsg_history_get_text_space_left(PCSGHistory* instance, FuriString* output) {
    furi_assert(instance);
    if(instance->last_index_write == PCSG_HISTORY_MAX) {
        if(output != NULL) furi_string_printf(output, "Memory is FULL");
        return true;
    }
    if(output != NULL)
        furi_string_printf(output, "%02u/%02u", instance->last_index_write, PCSG_HISTORY_MAX);
    return false;
}

void pcsg_history_get_text_item_menu(PCSGHistory* instance, FuriString* output, uint16_t idx) {
    PCSGHistoryItem* item = PCSGHistoryItemArray_get(instance->history->data, idx);
    furi_string_set(output, item->item_str);
}

PCSGHistoryStateAddKey
    pcsg_history_add_to_history(PCSGHistory* instance, void* context, SubGhzRadioPreset* preset) {
    furi_assert(instance);
    furi_assert(context);

    if(instance->last_index_write >= PCSG_HISTORY_MAX) return PCSGHistoryStateAddKeyOverflow;

    SubGhzProtocolDecoderBase* decoder_base = context;
    if((instance->code_last_hash_data ==
        subghz_protocol_decoder_base_get_hash_data(decoder_base)) &&
       ((furi_get_tick() - instance->last_update_timestamp) < 500)) {
        instance->last_update_timestamp = furi_get_tick();
        return PCSGHistoryStateAddKeyTimeOut;
    }

    instance->code_last_hash_data = subghz_protocol_decoder_base_get_hash_data(decoder_base);
    instance->last_update_timestamp = furi_get_tick();

    FlipperFormat* fff = flipper_format_string_alloc();
    subghz_protocol_decoder_base_serialize(decoder_base, fff, preset);

    do {
        if(!flipper_format_rewind(fff)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

    } while(false);
    flipper_format_free(fff);

    PCSGHistoryItem* item = PCSGHistoryItemArray_push_raw(instance->history->data);
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

        if(!flipper_format_rewind(item->flipper_string)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        FuriString* temp_ric = furi_string_alloc();
        if(!flipper_format_read_string(item->flipper_string, "Ric", temp_ric)) {
            FURI_LOG_E(TAG, "Missing Ric");
            break;
        }

        FuriString* temp_message = furi_string_alloc();
        if(!flipper_format_read_string(item->flipper_string, "Message", temp_message)) {
            FURI_LOG_E(TAG, "Missing Message");
            break;
        }

        furi_string_printf(
            item->item_str,
            "%s%s",
            furi_string_get_cstr(temp_ric),
            furi_string_get_cstr(temp_message));

        furi_string_free(temp_message);
        furi_string_free(temp_ric);

    } while(false);
    instance->last_index_write++;
    return PCSGHistoryStateAddKeyNewDada;

    return PCSGHistoryStateAddKeyUnknown;
}
