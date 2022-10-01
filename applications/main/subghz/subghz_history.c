#include "subghz_history.h"
#include <lib/subghz/receiver.h>
#include <flipper_format/flipper_format_i.h>

#define SUBGHZ_HISTORY_MAX 65
#define SUBGHZ_HISTORY_TMP_DIR EXT_PATH("subghz/tmp_history")
#define SUBGHZ_HISTORY_TMP_EXTENSION ".tmp"
#define SUBGHZ_HISTORY_TMP_FILE_KEY "Filename"
#define TAG "SubGhzHistory"

typedef struct {
    string_t item_str;
    FlipperFormat* flipper_string;
    string_t protocol_name;
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
    string_t tmp_string;
    bool write_tmp_files;
    Storage* storage;
    SubGhzHistoryStruct* history;
};

#if FURI_DEBUG
#define LOG_DELAY 1
#endif

/**
 * @brief Generate filename like 000.tmp
 * 
 * @param filename 
 * @param index - index of file, timestamp doesn't accepted!
 */
void subghz_history_generate_temp_filename(string_t filename, uint32_t index) {
    FuriHalRtcDateTime datetime = {0};
    furi_hal_rtc_get_datetime(&datetime);
    string_init_printf(filename, "%03d%s", index, SUBGHZ_HISTORY_TMP_EXTENSION);
}

/**
 * @brief Check SD card, recursive delete dir and files and create new dir
 * 
 * @param instance 
 * @param only_remove_dir 
 * @return true - if all ok
 * @return false - if something failed
 */
bool subghz_history_clear_dir_or_create(SubGhzHistory* instance, bool only_remove_dir) {
#if FURI_DEBUG
    FURI_LOG_D(TAG, "subghz_history_clear_dir_or_create: %s", only_remove_dir ? "true" : "false");
    furi_delay_ms(LOG_DELAY);
#endif

    // Stage 0 - SD installed?
    FS_Error status = storage_sd_status(instance->storage);
    if(status != FSE_OK) {
        FURI_LOG_W(TAG, "SD storage not installed! Status: %d", status);
        return false;
    }

    // Stage 1 - delete all content if exists
    FileInfo fileinfo;
    storage_common_stat(instance->storage, SUBGHZ_HISTORY_TMP_DIR, &fileinfo);

    // This is temp
    bool res = false; //instance->write_tmp_files = true;

    // Uncomment it
    if(fileinfo.flags & FSF_DIRECTORY) {
        res = storage_simply_remove_recursive(instance->storage, SUBGHZ_HISTORY_TMP_DIR);
    } else {
        res = (storage_common_remove(instance->storage, SUBGHZ_HISTORY_TMP_DIR) == FSE_OK);
    }

#if FURI_DEBUG
    FURI_LOG_D(TAG, "storage_common_remove done: %s", res ? "true" : "false");
    furi_delay_ms(LOG_DELAY);
#endif

    // Uncomment it
    // Stage 2 - create dir
    if(!only_remove_dir && res) {
        res = storage_simply_mkdir(instance->storage, SUBGHZ_HISTORY_TMP_DIR);
#if FURI_DEBUG
        FURI_LOG_D(TAG, "storage_simply_mkdir done: %s", res ? "true" : "false");
        furi_delay_ms(LOG_DELAY);
#endif
    }

    return res;
}

SubGhzHistory* subghz_history_alloc(void) {
    SubGhzHistory* instance = malloc(sizeof(SubGhzHistory));
    string_init(instance->tmp_string);
    instance->history = malloc(sizeof(SubGhzHistoryStruct));
    SubGhzHistoryItemArray_init(instance->history->data);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->write_tmp_files = false;

#if FURI_DEBUG
    FURI_LOG_D(TAG, "BEFORE subghz_history_clear_dir_or_create");
    furi_delay_ms(LOG_DELAY);
#endif
    // Check if we can write files on SD
    instance->write_tmp_files = subghz_history_clear_dir_or_create(instance, false);

    return instance;
}

/**
 * @brief free all items in array
 * 
 * @param instance 
 */
void subghz_history_clean_item_array(SubGhzHistory* instance) {
    for
        M_EACH(item, instance->history->data, SubGhzHistoryItemArray_t) {
            string_clear(item->item_str);
            string_clear(item->preset->name);
            string_clear(item->protocol_name);

            free(item->preset);
            item->type = 0;
            item->is_file = false;

            if(item->flipper_string != NULL) {
                flipper_format_free(item->flipper_string);
            }
        }
}

void subghz_history_free(SubGhzHistory* instance) {
    furi_assert(instance);
    string_clear(instance->tmp_string);

    subghz_history_clean_item_array(instance);
    SubGhzHistoryItemArray_clear(instance->history->data);
    free(instance->history);

    if(instance->write_tmp_files) {
        instance->write_tmp_files = subghz_history_clear_dir_or_create(instance, true);
    }

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
    return string_get_cstr(item->preset->name);
}

void subghz_history_reset(SubGhzHistory* instance) {
    furi_assert(instance);
    string_reset(instance->tmp_string);

    subghz_history_clean_item_array(instance);

    SubGhzHistoryItemArray_reset(instance->history->data);
    instance->last_index_write = 0;
    instance->code_last_hash_data = 0;

    if(instance->write_tmp_files) {
        instance->write_tmp_files = subghz_history_clear_dir_or_create(instance, false);
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

    return string_get_cstr(item->protocol_name);
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
#if FURI_DEBUG
            FURI_LOG_D(TAG, "We have files!");
            furi_delay_ms(LOG_DELAY);
#endif
            string_t filename;
            string_t dir_path;
            string_init(filename);
            string_init(dir_path);
            subghz_history_generate_temp_filename(filename, idx);
            string_cat_printf(
                dir_path, "%s/%s", SUBGHZ_HISTORY_TMP_DIR, string_get_cstr(filename));
            // string_init_printf(
            //     dir_path, "%s/%s", SUBGHZ_HISTORY_TMP_DIR, string_get_cstr(filename));

            if(storage_file_exists(instance->storage, string_get_cstr(dir_path))) {
#if FURI_DEBUG
                FURI_LOG_D(TAG, "Exist: %s", dir_path);
                furi_delay_ms(LOG_DELAY);
#endif
                // Set to current anyway it has NULL value
                item->flipper_string = flipper_format_string_alloc();
                Stream* dst_stream = flipper_format_get_raw_stream(item->flipper_string);
                stream_clean(dst_stream);

                size_t size = stream_load_from_file(
                    dst_stream, instance->storage, string_get_cstr(dir_path));
                if(size > 0) {
#if FURI_DEBUG
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

            string_clear(filename);
            string_clear(dir_path);
        } else {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "Write TMP files failed!");
            furi_delay_ms(LOG_DELAY);
#endif
        }
        return result_ok ? item->flipper_string : NULL;
    }
}

bool subghz_history_get_text_space_left(SubGhzHistory* instance, string_t output) {
    furi_assert(instance);
    if(instance->last_index_write == SUBGHZ_HISTORY_MAX) {
        if(output != NULL) string_printf(output, "Memory is FULL");
        return true;
    }
    if(output != NULL) {
        string_printf(output, "%02u/%02u", instance->last_index_write, SUBGHZ_HISTORY_MAX);
    }
    return false;
}

void subghz_history_get_text_item_menu(SubGhzHistory* instance, string_t output, uint16_t idx) {
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_get(instance->history->data, idx);
    string_set(output, item->item_str);
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

    string_t text;
    string_init(text);
    SubGhzHistoryItem* item = SubGhzHistoryItemArray_push_raw(instance->history->data);
    item->preset = malloc(sizeof(SubGhzPresetDefinition));
    item->type = decoder_base->protocol->type;
    item->preset->frequency = preset->frequency;
    string_init(item->preset->name);
    string_set(item->preset->name, preset->name);
    item->preset->data = preset->data;
    item->preset->data_size = preset->data_size;

    string_init(item->item_str);
    string_init(item->protocol_name);

    // At this point file mapped to memory otherwise file cannot decoded
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
            string_init_printf(item->protocol_name, "%s", string_get_cstr(instance->tmp_string));
#if FURI_DEBUG
            FURI_LOG_I(TAG, "File protocol: %s", string_get_cstr(item->protocol_name));
#endif
        }
        if(!strcmp(string_get_cstr(instance->tmp_string), "RAW")) {
            string_printf(
                item->item_str,
                "RAW %03ld.%02ld",
                preset->frequency / 1000000 % 1000,
                preset->frequency / 10000 % 100);

            if(!flipper_format_rewind(item->flipper_string)) {
                FURI_LOG_E(TAG, "Rewind error");
            }

            break;
        } else if(!strcmp(string_get_cstr(instance->tmp_string), "KeeLoq")) {
            string_set_str(instance->tmp_string, "KL ");
            if(!flipper_format_read_string(item->flipper_string, "Manufacture", text)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }
            string_cat(instance->tmp_string, text);
        } else if(!strcmp(string_get_cstr(instance->tmp_string), "Star Line")) {
            string_set_str(instance->tmp_string, "SL ");
            if(!flipper_format_read_string(item->flipper_string, "Manufacture", text)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }
            string_cat(instance->tmp_string, text);
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
            string_printf(
                item->item_str,
                "%s %lX",
                string_get_cstr(instance->tmp_string),
                (uint32_t)(data & 0xFFFFFFFF));
        } else {
            string_printf(
                item->item_str,
                "%s %lX%08lX",
                string_get_cstr(instance->tmp_string),
                (uint32_t)(data >> 32),
                (uint32_t)(data & 0xFFFFFFFF));
        }
    } while(false);

    // Copy streams
    // Thinking that some data may be saved
    // Stream* src = flipper_format_get_raw_stream(flipper_string);
    // stream_seek(src, 0, StreamOffsetFromStart);

    // Stream* dst = string_stream_alloc();
    // stream_clean(dst);
    // stream_copy_full(src, dst);

    // If we can write to files
    //bool no_close = false;
    if(instance->write_tmp_files) {
        string_t filename;
        string_t dir_path;
        string_init(filename);
        string_init(dir_path);

        subghz_history_generate_temp_filename(filename, instance->last_index_write);
        string_cat_printf(dir_path, "%s/%s", SUBGHZ_HISTORY_TMP_DIR, string_get_cstr(filename));
#if FURI_DEBUG
        FURI_LOG_I(TAG, "Let's do some hack. Create file %s", string_get_cstr(dir_path));
#endif
        Stream* dst = flipper_format_get_raw_stream(item->flipper_string);
        stream_rewind(dst);
        // stream_seek(dst, 0, StreamOffsetFromStart);
        if(stream_save_to_file(
               dst, instance->storage, string_get_cstr(dir_path), FSOM_CREATE_ALWAYS) > 0) {
            // Free flipper_format
            //flipper_format_free(flipper_string);
            //flipper_string = NULL;
            //stream_free(dst);
            flipper_format_free(item->flipper_string);
            item->flipper_string = NULL;
#if FURI_DEBUG
            FURI_LOG_I(TAG, "Save done!");
#endif
            // This item contains fake data to load from SD
            item->is_file = true;
        } else {
            FURI_LOG_E(TAG, "Stream copy failed!");
        }
        string_clear(filename);
        string_clear(dir_path);

        /* }*/
    } else {
#if FURI_DEBUG
        FURI_LOG_I(TAG, "Old fashion way");
#endif
        // Old fashion way
        //item->flipper_string = dst;
    }
    //flipper_format_free(flipper_string);

    string_clear(text);

    instance->last_index_write++;
    return true;
}
