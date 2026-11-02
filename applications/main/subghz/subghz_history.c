#include "subghz_history.h"
#include <lib/subghz/receiver.h>

#include <furi.h>

#define TAG "SubGhzHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    uint16_t tx_block;
    uint8_t type;
    SubGhzRadioPreset* preset;
    DateTime datetime;
} SubGhzHistoryItem;

ARRAY_DEF(SubGhzHistoryItemArray, SubGhzHistoryItem, M_POD_OPLIST) //-V658

#define M_OPL_SubGhzHistoryItemArray_t() ARRAY_OPLIST(SubGhzHistoryItemArray, M_POD_OPLIST)

typedef struct {
    SubGhzHistoryItemArray_t data;
} SubGhzHistoryStruct;

static void subghz_history_tx_block_refresh(SubGhzHistory* instance);

struct SubGhzHistory {
    uint32_t last_update_air_time;
    uint16_t last_index_write;
    uint8_t code_last_hash_data;
    bool code_last_hash_data_set;
    bool full;
    FuriString* tmp_string;
    SubGhzHistoryStruct* history;
};

SubGhzHistory* subghz_history_alloc(void) {
    SubGhzHistory* instance = malloc(sizeof(SubGhzHistory));
    instance->tmp_string = furi_string_alloc();
    instance->history = malloc(sizeof(SubGhzHistoryStruct));
    SubGhzHistoryItemArray_init(instance->history->data);
    instance->full = false;
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
    subghz_txrx_tx_block_reserve(SUBGHZ_TX_MIN_FREE_BLOCK_DEFAULT);
    instance->full = false;
    instance->last_index_write = 0;
    instance->code_last_hash_data = 0;
    instance->code_last_hash_data_set = false;
    instance->last_update_air_time = 0;
}

void subghz_history_delete_item(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);

    if(idx < SubGhzHistoryItemArray_size(instance->history->data)) {
        SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
        furi_string_free(item->item_str);
        furi_string_free(item->preset->name);
        free(item->preset);
        flipper_format_free(item->flipper_string);
        item->type = 0;
        SubGhzHistoryItemArray_remove_v(instance->history->data, idx, idx + 1);
        instance->last_index_write--;
        // Something real changed, so the next check measures the heap as it is now
        instance->full = false;
        subghz_history_tx_block_refresh(instance);
    }
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
    if(!item || !item->flipper_string) {
        FURI_LOG_E(TAG, "Missing Item");
        furi_string_reset(instance->tmp_string);
        return furi_string_get_cstr(instance->tmp_string);
    }
    flipper_format_rewind(item->flipper_string);
    if(!flipper_format_read_string(item->flipper_string, "Protocol", instance->tmp_string)) {
        FURI_LOG_E(TAG, "Missing Protocol");
        furi_string_reset(instance->tmp_string);
    }
    return furi_string_get_cstr(instance->tmp_string);
}

DateTime subghz_history_get_datetime(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    if(item) {
        return item->datetime;
    } else {
        return (DateTime){};
    }
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
// The held send block only has to cover the widest protocol still in the list, so it is
// resized whenever that changes. Growing can fail, which is what turns a signal away
static void subghz_history_tx_block_refresh(SubGhzHistory* instance) {
    size_t need = 0;
    for(size_t i = 0; i < SubGhzHistoryItemArray_size(instance->history->data); i++) {
        const uint16_t item_need =
            SubGhzHistoryItemArray_get(instance->history->data, i)->tx_block;
        if(item_need > need) need = item_need;
    }
    subghz_txrx_tx_block_reserve((need != 0) ? need : SUBGHZ_TX_MIN_FREE_BLOCK_DEFAULT);
}

// Is there heap left for one more signal of this type, with a send still possible. The
// slack is what a history that already reads full has to get back before it reads otherwise
static bool subghz_history_has_room_for(uint8_t type, size_t slack) {
    const bool raw = (type == SubGhzProtocolTypeBinRAW);
    const size_t reserve =
        (raw ? SUBGHZ_HISTORY_RESERVE_RAW : SUBGHZ_HISTORY_RESERVE_ITEM) + slack;
    size_t block = raw ? SUBGHZ_HISTORY_BLOCK_RAW : SUBGHZ_HISTORY_BLOCK_ITEM;
    // Only what the record itself asks for is measured here. Whether a send is still
    // possible is the send block's business and the popup's: a receiver that stops
    // receiving because a send might not fit is the wrong way round, and the block it
    // would be waiting for is one that dropping records cannot produce
    block += slack + SUBGHZ_HISTORY_BLOCK_OVERHEAD;
    return (memmgr_get_free_heap() >= reserve) && (memmgr_heap_get_max_free_block() >= block);
}

bool subghz_history_is_full(SubGhzHistory* instance, uint8_t type) {
    furi_assert(instance);
    if(instance->last_index_write >= SUBGHZ_HISTORY_CAPACITY) return true;
    instance->full =
        !subghz_history_has_room_for(type, instance->full ? SUBGHZ_HISTORY_FULL_CLEAR_SLACK : 0);
    return instance->full;
}

bool subghz_history_needs_heap(SubGhzHistory* instance, uint8_t type) {
    furi_assert(instance);
    const size_t reserve = (type == SubGhzProtocolTypeBinRAW) ? SUBGHZ_HISTORY_RESERVE_RAW :
                                                                SUBGHZ_HISTORY_RESERVE_ITEM;
    return memmgr_get_free_heap() < reserve;
}

bool subghz_history_get_text_space_left(
    SubGhzHistory* instance,
    FuriString* output,
    uint8_t type,
    bool unlimited) {
    furi_assert(instance);
    if(!unlimited && subghz_history_is_full(instance, type)) {
        if(output != NULL) furi_string_printf(output, "   Memory is FULL");
        return true;
    }
    if(output != NULL) furi_string_printf(output, "%02u", instance->last_index_write);
    return false;
}

uint16_t subghz_history_get_last_index(SubGhzHistory* instance) {
    return instance->last_index_write;
}
void subghz_history_get_text_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx) {
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    furi_string_set(output, item->item_str);
}

void subghz_history_get_time_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx) {
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    DateTime* t = &item->datetime;
    furi_string_printf(output, "%.2d:%.2d:%.2d ", t->hour, t->minute, t->second);
}

bool subghz_history_add_to_history(
    SubGhzHistory* instance,
    void* context,
    SubGhzRadioPreset* preset,
    uint32_t air_time) {
    furi_assert(instance);
    furi_assert(context);

    SubGhzProtocolDecoderBase* decoder_base = context;
    if(subghz_history_is_full(instance, decoder_base->protocol->type)) return false;

    uint8_t code_hash_data = subghz_protocol_decoder_base_get_hash_data(decoder_base);
    //the "have we seen anything at all" flag matters: without it a signal whose hash is
    //zero would match the value the filter starts out with and never reach the history
    //
    //the window is measured in air time, not wall time. a transmitter repeats its frame
    //for as long as the button is held, and the decoders only report a frame once they
    //see the gap that ends it - for the last repeat of a burst that gap is the silence
    //afterwards, which is not measured until the receiver picks up an edge again, so the
    //repeat can be reported seconds after it was actually sent. air time counts what was
    //decoded, so it puts the repeat right next to the ones before it
    if(instance->code_last_hash_data_set && (instance->code_last_hash_data == code_hash_data) &&
       ((air_time - instance->last_update_air_time) < 500)) {
        instance->last_update_air_time = air_time;
        return false;
    }

    // Widen the held block so this signal stays sendable. A repeat that was just suppressed
    // never gets here, so it cannot widen it for a record nobody keeps. Failing is allowed:
    // the record is kept either way and the send says so if it comes to that, which beats
    // turning away signals over a block no amount of deleting can produce. The size is
    // remembered with the record so a later delete can try again
    const size_t tx_block = subghz_txrx_tx_block_need(decoder_base->protocol->name);
    if(tx_block > subghz_txrx_tx_block_size()) {
        subghz_txrx_tx_block_reserve(tx_block);
    }

    instance->code_last_hash_data = code_hash_data;
    instance->code_last_hash_data_set = true;
    instance->last_update_air_time = air_time;

    FuriString* text = furi_string_alloc();
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_push_raw(instance->history->data);
    item->preset = malloc(sizeof(SubGhzRadioPreset));
    item->type = decoder_base->protocol->type;
    item->tx_block = (uint16_t)tx_block;
    item->preset->frequency = preset->frequency;
    item->preset->name = furi_string_alloc();
    furi_string_set(item->preset->name, preset->name);
    item->preset->data = preset->data;
    item->preset->data_size = preset->data_size;
    furi_hal_rtc_get_datetime(&item->datetime);

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
