#include "subghz_history.h"
#include "subghz_history_private.h"
#include <lib/subghz/receiver.h>
#include <flipper_format/flipper_format_i.h>

#define SUBGHZ_HISTORY_MAX 65

/**
 * @brief Settings for temporary files
 * 
 */
#define SUBGHZ_HISTORY_TMP_DIR EXT_PATH("subghz/tmp_history")
#define SUBGHZ_HISTORY_TMP_EXTENSION ".tmp"
#define SUBGHZ_HISTORY_TMP_SIGNAL_MAX_LEVEL_DURATION 700
#define SUBGHZ_HISTORY_TMP_SIGNAL_MIN_LEVEL_DURATION 100
#define SUBGHZ_HISTORY_TMP_REMOVE_FILES true
#define SUBGHZ_HISTORY_TMP_RAW_KEY "RAW_Data"

#define TAG "SubGhzHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    FuriString* protocol_name;
    bool is_file;
    uint8_t type;
    SubGhzPresetDefinition* preset;
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
    bool write_tmp_files;
    Storage* storage;
    SubGhzHistoryStruct* history;
};

#ifdef FURI_DEBUG
#define LOG_DELAY 0
#endif

FuriString* subghz_history_generate_temp_filename(uint32_t index) {
    FuriHalRtcDateTime datetime = {0};
    furi_hal_rtc_get_datetime(&datetime);
    return furi_string_alloc_printf("%03d%s", index, SUBGHZ_HISTORY_TMP_EXTENSION);
}

bool subghz_history_is_tmp_dir_exists(SubGhzHistory* instance) {
    FileInfo file_info;
    storage_common_stat(instance->storage, SUBGHZ_HISTORY_TMP_DIR, &file_info);

    if(storage_common_stat(instance->storage, SUBGHZ_HISTORY_TMP_DIR, &file_info) == FSE_OK) {
        if(file_info.flags & FSF_DIRECTORY) {
            return true;
        }
    }

    return false;
}

bool subghz_history_check_sdcard(SubGhzHistory* instance) {
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "check_sdcard");
    uint32_t start_time = furi_get_tick();
#endif

    bool result = false;
    // Stage 0 - check SD Card
    FS_Error status = storage_sd_status(instance->storage);
    if(status == FSE_OK) {
        result = subghz_history_is_tmp_dir_exists(instance);
        if(!subghz_history_is_tmp_dir_exists(instance)) {
            result = storage_simply_mkdir(instance->storage, SUBGHZ_HISTORY_TMP_DIR);
        }
    } else {
        FURI_LOG_W(TAG, "SD storage not installed! Status: %d", status);
    }
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Running time (check_sdcard): %d ms", furi_get_tick() - start_time);
#endif

    return result;
}

void subghz_history_clear_tmp_dir(SubGhzHistory* instance) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "clear_tmp_dir");
#endif

    if(!instance->write_tmp_files) {
        // Nothing to do here!
        return;
    }
    //uint32_t start_time = furi_get_tick();
#ifdef SUBGHZ_HISTORY_TMP_REMOVE_FILES
    // Stage 0 - Dir exists?
    bool res = subghz_history_is_tmp_dir_exists(instance);
    if(res) {
        // Stage 1 - delete all content if exists
        FileInfo fileinfo;
        storage_common_stat(instance->storage, SUBGHZ_HISTORY_TMP_DIR, &fileinfo);

        res = fileinfo.flags & FSF_DIRECTORY ?
                  storage_simply_remove_recursive(instance->storage, SUBGHZ_HISTORY_TMP_DIR) :
                  (storage_common_remove(instance->storage, SUBGHZ_HISTORY_TMP_DIR) == FSE_OK);
    }

    // Stage 2 - create dir if necessary
    res = !storage_simply_mkdir(instance->storage, SUBGHZ_HISTORY_TMP_DIR);
    if(!res) {
        FURI_LOG_E(TAG, "Cannot process temp dir!");
    }
#endif
    /* uint32_t stop_time = furi_get_tick() - start_time;
    FURI_LOG_I(TAG, "Running time (clear_tmp_dir): %d ms", stop_time);*/
}

SubGhzHistory* subghz_history_alloc(void) {
    SubGhzHistory* instance = malloc(sizeof(SubGhzHistory));
    instance->tmp_string = furi_string_alloc();
    instance->history = malloc(sizeof(SubGhzHistoryStruct));
    SubGhzHistoryItemArray_init(instance->history->data);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->write_tmp_files = subghz_history_check_sdcard(instance);

    if(!instance->write_tmp_files) {
        FURI_LOG_E(TAG, "Unstable work! Cannot use SD Card!");
    }

    return instance;
}

void subghz_history_item_free(void* current_item) {
    furi_assert(current_item);
    SubGhzHistoryItem* item = (SubGhzHistoryItem*)current_item;
    furi_string_free(item->item_str);
    furi_string_free(item->preset->name);
    furi_string_free(item->protocol_name);

    free(item->preset);
    item->type = 0;
    item->is_file = false;

    if(item->flipper_string != NULL) {
        flipper_format_free(item->flipper_string);
    }
}

void subghz_history_clean_item_array(SubGhzHistory* instance) {
    for
        M_EACH(item, instance->history->data, SubGhzHistoryItemArray_t) {
            subghz_history_item_free(item);
        }
}

void subghz_history_free(SubGhzHistory* instance) {
    furi_assert(instance);
    furi_string_free(instance->tmp_string);

    subghz_history_clean_item_array(instance);
    SubGhzHistoryItemArray_clear(instance->history->data);
    free(instance->history);

    // Delete all temporary file, on exit it's ok
    subghz_history_clear_tmp_dir(instance);

    furi_record_close(RECORD_STORAGE);

    free(instance);
}

uint32_t subghz_history_get_frequency(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    return item->preset->frequency;
}

SubGhzPresetDefinition* subghz_history_get_preset_def(SubGhzHistory* instance, uint16_t idx) {
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

    subghz_history_clean_item_array(instance);

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

    return furi_string_get_cstr(item->protocol_name);
}

FlipperFormat* subghz_history_get_raw_data(SubGhzHistory* instance, uint16_t idx) {
    furi_assert(instance);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    if(item->flipper_string) {
        return item->flipper_string;
    } else {
        bool result_ok = false;
        if(instance->write_tmp_files && item->is_file) {
            // We have files!
            FuriString* filename = subghz_history_generate_temp_filename(idx);
            FuriString* dir_path;

            dir_path = furi_string_alloc_printf(
                "%s/%s", SUBGHZ_HISTORY_TMP_DIR, furi_string_get_cstr(filename));

            if(storage_file_exists(instance->storage, furi_string_get_cstr(dir_path))) {
#ifdef FURI_DEBUG
                FURI_LOG_D(TAG, "Exist: %s", dir_path);
                furi_delay_ms(LOG_DELAY);
#endif
                // Set to current anyway it has NULL value
                item->flipper_string = flipper_format_string_alloc();
                Stream* dst_stream = flipper_format_get_raw_stream(item->flipper_string);
                stream_clean(dst_stream);

                size_t size = stream_load_from_file(
                    dst_stream, instance->storage, furi_string_get_cstr(dir_path));
                if(size > 0) {
#ifdef FURI_DEBUG
                    FURI_LOG_I(TAG, "Save ok!");
                    furi_delay_ms(LOG_DELAY);
#endif
                    // We changed contents of file, so we no needed to load
                    // content from disk for the next time
                    item->is_file = false;
                    result_ok = true;
                } else {
                    FURI_LOG_E(TAG, "Stream copy failed!");
                    flipper_format_free(item->flipper_string);
                }
            } else {
                FURI_LOG_E(TAG, "Can't convert filename to file");
            }

            furi_string_free(filename);
            furi_string_free(dir_path);
        } else {
#ifdef FURI_DEBUG
            FURI_LOG_W(TAG, "Write TMP files failed!");
            furi_delay_ms(LOG_DELAY);
#endif
        }
        return result_ok ? item->flipper_string : NULL;
    }
}

bool subghz_history_get_text_space_left(SubGhzHistory* instance, FuriString* output) {
    furi_assert(instance);
    if(instance->last_index_write == SUBGHZ_HISTORY_MAX) {
        if(output != NULL) furi_string_printf(output, "Memory is FULL");
        return true;
    }
    if(output != NULL) {
        furi_string_printf(output, "%02u/%02u", instance->last_index_write, SUBGHZ_HISTORY_MAX);
    }
    return false;
}

void subghz_history_get_text_item_menu(SubGhzHistory* instance, FuriString* output, uint16_t idx) {
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    furi_string_set(output, item->item_str);
}

bool subghz_history_add_to_history(
    SubGhzHistory* instance,
    void* context,
    SubGhzPresetDefinition* preset) {
    furi_assert(instance);
    furi_assert(context);

    if(instance->last_index_write >= SUBGHZ_HISTORY_MAX) {
        return false;
    }

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
    item->preset = malloc(sizeof(SubGhzPresetDefinition));
    item->type = decoder_base->protocol->type;
    item->preset->frequency = preset->frequency;
    item->preset->name = furi_string_alloc();
    furi_string_set(item->preset->name, preset->name);
    item->preset->data = preset->data;
    item->preset->data_size = preset->data_size;

    item->item_str = furi_string_alloc();
    item->protocol_name = furi_string_alloc();

    bool tmp_file_for_raw = false;

    // At this point file mapped to memory otherwise file cannot decode
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
        } else {
            furi_string_printf(
                item->protocol_name, "%s", furi_string_get_cstr(instance->tmp_string));
        }
        if(!strcmp(furi_string_get_cstr(instance->tmp_string), "RAW")) {
            furi_string_printf(
                item->item_str,
                "RAW %03ld.%02ld",
                preset->frequency / 1000000 % 1000,
                preset->frequency / 10000 % 100);

            if(!flipper_format_rewind(item->flipper_string)) {
                FURI_LOG_E(TAG, "Rewind error");
            }
            tmp_file_for_raw = true;
            break;
        } else if(!strcmp(furi_string_get_cstr(instance->tmp_string), "KeeLoq")) {
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
            FURI_LOG_E(TAG, "Missing Key");
            break;
        }
        uint64_t data = 0;
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            data = (data << 8) | key_data[i];
        }
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
    } while(false);

    // If we can write to files
    if(instance->write_tmp_files && tmp_file_for_raw) {
        FuriString* filename = subghz_history_generate_temp_filename(instance->last_index_write);
        FuriString* dir_path;
        dir_path = furi_string_alloc();

        furi_string_cat_printf(
            dir_path, "%s/%s", SUBGHZ_HISTORY_TMP_DIR, furi_string_get_cstr(filename));
#ifdef FURI_DEBUG
        FURI_LOG_I(TAG, "Save temp file: %s", furi_string_get_cstr(dir_path));
#endif
        if(!subghz_history_tmp_write_file_split(instance, item, dir_path)) {
            // Plan B!
            subghz_history_tmp_write_file_full(instance, item, dir_path);
        }
        furi_string_free(filename);
        furi_string_free(dir_path);

    } else {
#ifdef FURI_DEBUG
        FURI_LOG_I(TAG, "Old fashion way");
#endif
    }

    furi_string_free(text);

    instance->last_index_write++;
    return true;
}

bool subghz_history_tmp_write_file_split(
    SubGhzHistory* instance,
    void* current_item,
    FuriString* dir_path) {
    UNUSED(instance);
    UNUSED(current_item);
    UNUSED(dir_path);
    /*furi_assert(instance);
    furi_assert(current_item);
    furi_assert(dir_path);*/
    //SubGhzHistoryItem* item = (SubGhzHistoryItem*)current_item;

    return false;
}

void subghz_history_tmp_write_file_full(
    SubGhzHistory* instance,
    void* current_item,
    FuriString* dir_path) {
    SubGhzHistoryItem* item = (SubGhzHistoryItem*)current_item;
#ifdef FURI_DEBUG
    FURI_LOG_W(TAG, "Save temp file full: %s", furi_string_get_cstr(dir_path));
#endif
    Stream* dst = flipper_format_get_raw_stream(item->flipper_string);
    stream_rewind(dst);
    if(stream_save_to_file(
           dst, instance->storage, furi_string_get_cstr(dir_path), FSOM_CREATE_ALWAYS) > 0) {
        flipper_format_free(item->flipper_string);
        item->flipper_string = NULL;
#ifdef FURI_DEBUG
        FURI_LOG_I(TAG, "Save done!");
#endif
        // This item contains fake data to load from SD
        item->is_file = true;
    } else {
        FURI_LOG_E(TAG, "Stream copy failed!");
    }
}