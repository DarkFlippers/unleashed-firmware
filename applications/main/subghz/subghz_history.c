#include "subghz_history.h"
#include "subghz_history_private.h"
#include <lib/subghz/receiver.h>
#include <toolbox/path.h>
#include <flipper_format/flipper_format_i.h>
#include "flipper_format_stream_i.h"
#include <inttypes.h>

#define SUBGHZ_HISTORY_MAX 60

/**
 * @brief Settings for temporary files
 * 
 */
#define SUBGHZ_HISTORY_TMP_DIR EXT_PATH("subghz/tmp_history")
#define SUBGHZ_HISTORY_TMP_EXTENSION ".tmp"
#define SUBGHZ_HISTORY_TMP_SIGNAL_MAX 700
#define SUBGHZ_HISTORY_TMP_SIGNAL_MIN 100
#define SUBGHZ_HISTORY_TMP_REMOVE_FILES true
#define SUBGHZ_HISTORY_TMP_RAW_KEY "RAW_Data"
#define MAX_LINE 500
const size_t buffer_size = 32;

#define TAG "SubGhzHistory"

typedef struct {
    FuriString* item_str;
    FlipperFormat* flipper_string;
    FuriString* protocol_name;
    bool is_file;
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
    return furi_string_alloc_printf("%03ld%s", index, SUBGHZ_HISTORY_TMP_EXTENSION);
}

bool subghz_history_is_tmp_dir_exists(SubGhzHistory* instance) {
    FileInfo file_info;
    FS_Error error = storage_common_stat(instance->storage, SUBGHZ_HISTORY_TMP_DIR, &file_info);

    if(error == FSE_OK) {
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
    FURI_LOG_I(TAG, "Running time (check_sdcard): %ld ms", furi_get_tick() - start_time);
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
    res = storage_simply_mkdir(instance->storage, SUBGHZ_HISTORY_TMP_DIR);
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
                FURI_LOG_D(TAG, "Exist: %s", furi_string_get_cstr(dir_path));
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

uint16_t subghz_history_get_last_index(SubGhzHistory* instance) {
    return instance->last_index_write;
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
    item->preset = malloc(sizeof(SubGhzRadioPreset));
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
            // Enable writing temp files to micro sd
            tmp_file_for_raw = true;
            // Write display name
            furi_string_printf(
                item->item_str,
                "RAW %03ld.%02ld",
                preset->frequency / 1000000 % 1000,
                preset->frequency / 10000 % 100);
            // Rewind
            if(!flipper_format_rewind(item->flipper_string)) {
                FURI_LOG_E(TAG, "Rewind error");
            }

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
        if(!subghz_history_tmp_write_file_split(instance, item, furi_string_get_cstr(dir_path))) {
            // Plan B!
            subghz_history_tmp_write_file_full(instance, item, dir_path);
        }
        if(item->is_file) {
            flipper_format_free(item->flipper_string);
            item->flipper_string = NULL;
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

static inline bool is_space_playground(char c) {
    return c == ' ' || c == '\t' || c == flipper_format_eolr;
}

bool subghz_history_stream_read_valid_key(Stream* stream, FuriString* key) {
    furi_string_reset(key);
    uint8_t buffer[buffer_size];

    bool found = false;
    bool error = false;
    bool accumulate = true;
    bool new_line = true;

    while(true) {
        size_t was_read = stream_read(stream, buffer, buffer_size);
        if(was_read == 0) break;

        for(size_t i = 0; i < was_read; i++) {
            uint8_t data = buffer[i];
            if(data == flipper_format_eoln) {
                // EOL found, clean data, start accumulating data and set the new_line flag
                furi_string_reset(key);
                accumulate = true;
                new_line = true;
            } else if(data == flipper_format_eolr) {
                // ignore
            } else if(data == flipper_format_comment && new_line) {
                // if there is a comment character and we are at the beginning of a new line
                // do not accumulate comment data and reset the new_line flag
                accumulate = false;
                new_line = false;
            } else if(data == flipper_format_delimiter) {
                if(new_line) {
                    // we are on a "new line" and found the delimiter
                    // this can only be if we have previously found some kind of key, so
                    // clear the data, set the flag that we no longer want to accumulate data
                    // and reset the new_line flag
                    furi_string_reset(key);
                    accumulate = false;
                    new_line = false;
                } else {
                    // parse the delimiter only if we are accumulating data
                    if(accumulate) {
                        // we found the delimiter, move the rw pointer to the delimiter location
                        // and signal that we have found something
                        if(!stream_seek(stream, i - was_read, StreamOffsetFromCurrent)) {
                            error = true;
                            break;
                        }

                        found = true;
                        break;
                    }
                }
            } else {
                // just new symbol, reset the new_line flag
                new_line = false;
                if(accumulate) {
                    // and accumulate data if we want
                    furi_string_push_back(key, data);
                }
            }
        }

        if(found || error) break;
    }

    return found;
}

bool subghz_history_stream_seek_to_key(Stream* stream, const char* key, bool strict_mode) {
    bool found = false;
    FuriString* read_key;

    read_key = furi_string_alloc();

    while(!stream_eof(stream)) {
        if(subghz_history_stream_read_valid_key(stream, read_key)) {
            if(furi_string_cmp_str(read_key, key) == 0) {
                if(!stream_seek(stream, 2, StreamOffsetFromCurrent)) {
                    break;
                }
                found = true;
                break;
            } else if(strict_mode) {
                found = false;
                break;
            }
        }
    }
    furi_string_free(read_key);

    return found;
}

bool subghz_history_stream_read_value(Stream* stream, FuriString* value, bool* last) {
    enum { LeadingSpace, ReadValue, TrailingSpace } state = LeadingSpace;
    const size_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    bool result = false;
    bool error = false;

    furi_string_reset(value);

    while(true) {
        size_t was_read = stream_read(stream, buffer, buffer_size);

        if(was_read == 0) {
            if(state != LeadingSpace && stream_eof(stream)) {
                result = true;
                *last = true;
            } else {
                error = true;
            }
        }

        for(uint16_t i = 0; i < was_read; i++) {
            const uint8_t data = buffer[i];

            if(state == LeadingSpace) {
                if(is_space_playground(data)) {
                    continue;
                } else if(data == flipper_format_eoln) {
                    stream_seek(stream, i - was_read, StreamOffsetFromCurrent);
                    error = true;
                    break;
                } else {
                    state = ReadValue;
                    furi_string_push_back(value, data);
                }
            } else if(state == ReadValue) {
                if(is_space_playground(data)) {
                    state = TrailingSpace;
                } else if(data == flipper_format_eoln) {
                    if(!stream_seek(stream, i - was_read, StreamOffsetFromCurrent)) {
                        error = true;
                    } else {
                        result = true;
                        *last = true;
                    }
                    break;
                } else {
                    furi_string_push_back(value, data);
                }
            } else if(state == TrailingSpace) {
                if(is_space_playground(data)) {
                    continue;
                } else if(!stream_seek(stream, i - was_read, StreamOffsetFromCurrent)) {
                    error = true;
                } else {
                    *last = (data == flipper_format_eoln);
                    result = true;
                }
                break;
            }
        }

        if(error || result) break;
    }

    return result;
}

bool subghz_history_read_int32(Stream* stream, int32_t* _data, const uint16_t data_size) {
    bool result = false;
    result = true;
    FuriString* value;
    value = furi_string_alloc();

    for(size_t i = 0; i < data_size; i++) {
        bool last = false;
        result = subghz_history_stream_read_value(stream, value, &last);
        if(result) {
            int scan_values = 0;

            int32_t* data = _data;
            scan_values = sscanf(furi_string_get_cstr(value), "%" PRIi32, &data[i]);

            if(scan_values != 1) {
                result = false;
                break;
            }
        } else {
            break;
        }

        if(last && ((i + 1) != data_size)) {
            result = false;
            break;
        }
    }

    furi_string_free(value);
    return result;
}

uint32_t subghz_history_rand_range(uint32_t min, uint32_t max) {
    // size of range, inclusive
    const uint32_t length_of_range = max - min + 1;

    // add n so that we don't return a number below our range
    return (uint32_t)(rand() % length_of_range + min);
}

bool subghz_history_write_file_noise(
    Stream* file,
    bool is_negative_start,
    size_t current_position,
    bool empty_line) {
    size_t was_write = 0;
    if(empty_line) {
        was_write = stream_write_format(file, "%s: ", SUBGHZ_HISTORY_TMP_RAW_KEY);

        if(was_write <= 0) {
            FURI_LOG_E(TAG, "Can't write key!");
            return false;
        }
    }

    int8_t first;
    int8_t second;
    if(is_negative_start) {
        first = -1;
        second = 1;
    } else {
        first = 1;
        second = -1;
    }
    while(current_position < MAX_LINE) {
        was_write = stream_write_format(
            file,
            "%ld %ld ",
            subghz_history_rand_range(
                SUBGHZ_HISTORY_TMP_SIGNAL_MIN, SUBGHZ_HISTORY_TMP_SIGNAL_MAX) *
                first,
            subghz_history_rand_range(
                SUBGHZ_HISTORY_TMP_SIGNAL_MIN, SUBGHZ_HISTORY_TMP_SIGNAL_MAX) *
                second);

        if(was_write <= 0) {
            FURI_LOG_E(TAG, "Can't write random values!");
            return false;
        }

        current_position += was_write;
    }

    // Step back to write \n instead of space
    size_t offset = stream_tell(file);
    if(stream_seek(file, offset - 1, StreamOffsetFromCurrent)) {
        FURI_LOG_E(TAG, "Step back failed!");
        return false;
    }

    return stream_write_char(file, flipper_format_eoln) > 0;
}

bool subghz_history_write_file_data(
    Stream* src,
    Stream* file,
    bool* is_negative_start,
    size_t* current_position) {
    size_t offset_file = 0;
    bool result = false;
    int32_t value = 0;

    do {
        if(!subghz_history_read_int32(src, &value, 1)) {
            result = true;
            break;
        }
        offset_file = stream_tell(file);
        stream_write_format(file, "%ld ", value);
        *current_position += stream_tell(file) - offset_file;

        if(*current_position > MAX_LINE) {
            if((is_negative_start && value > 0) || (!is_negative_start && value < 0)) {
                // Align values
                continue;
            }

            if(stream_write_format(file, "\n%s: ", SUBGHZ_HISTORY_TMP_RAW_KEY) == 0) {
                FURI_LOG_E(TAG, "Can't write new line!");
                result = false;
                break;
            }
            *current_position = 0;
        }
    } while(true);

    *is_negative_start = value < 0;

    return result;
}

bool subghz_history_tmp_write_file_split(
    SubGhzHistory* instance,
    void* current_item,
    const char* dir_path) {
    furi_assert(instance);
    furi_assert(current_item);
    furi_assert(dir_path);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Save temp file splitted: %s", dir_path);
#endif
    SubGhzHistoryItem* item = (SubGhzHistoryItem*)current_item;

    uint8_t buffer[buffer_size];
    Stream* src = flipper_format_get_raw_stream(item->flipper_string);
    stream_rewind(src);

    FlipperFormat* flipper_format_file = flipper_format_file_alloc(instance->storage);
    bool result = false;
    FuriString* temp_str = furi_string_alloc();

    do {
        if(storage_file_exists(instance->storage, dir_path) &&
           storage_common_remove(instance->storage, dir_path) != FSE_OK) {
            FURI_LOG_E(TAG, "Can't delete old file!");
            break;
        }
        path_extract_dirname(dir_path, temp_str);
        FS_Error fs_result =
            storage_common_mkdir(instance->storage, furi_string_get_cstr(temp_str));
        if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
            FURI_LOG_E(TAG, "Can't create dir!");
            break;
        }
        result = flipper_format_file_open_always(flipper_format_file, dir_path);
        if(!result) {
            FURI_LOG_E(TAG, "Can't open file for write!");
            break;
        }
        Stream* file = flipper_format_get_raw_stream(flipper_format_file);

        if(!subghz_history_stream_seek_to_key(src, SUBGHZ_HISTORY_TMP_RAW_KEY, false)) {
            FURI_LOG_E(TAG, "Can't find key!");
            break;
        }
        bool is_negative_start = false;
        bool found = false;

        size_t offset_start;
        offset_start = stream_tell(src);

        // Check for negative value at the start and end to align file by correct values
        size_t was_read = stream_read(src, buffer, 1);
        if(was_read <= 0) {
            FURI_LOG_E(TAG, "Can't obtain first mark!");
            break;
        }

        is_negative_start = buffer[0] == '-';

        // Ready to write stream to file
        size_t current_position;
        stream_rewind(src);
        current_position = stream_copy(src, file, offset_start);
        if(current_position != offset_start) {
            FURI_LOG_E(TAG, "Invalid copy header data from one stream to another!");
            break;
        }

        found = true;

        current_position = 0;
        if(!subghz_history_write_file_noise(file, is_negative_start, current_position, false)) {
            FURI_LOG_E(TAG, "Add start noise failed!");
            break;
        }

        if(stream_write_format(file, "%s: ", SUBGHZ_HISTORY_TMP_RAW_KEY) == 0) {
            FURI_LOG_E(TAG, "Can't write new line!");
            result = false;
            break;
        }

        if(!subghz_history_write_file_data(src, file, &is_negative_start, &current_position)) {
            FURI_LOG_E(TAG, "Split by lines failed!");
            break;
        }

        if(!subghz_history_write_file_noise(file, is_negative_start, current_position, false)) {
            FURI_LOG_E(TAG, "Add end noise failed!");
            break;
        }

        if(!subghz_history_write_file_noise(file, is_negative_start, 0, true)) {
            FURI_LOG_E(TAG, "Add end noise failed!");
            break;
        }

        result = found;
    } while(false);
    flipper_format_file_close(flipper_format_file);
    flipper_format_free(flipper_format_file);
    furi_string_free(temp_str);

    item->is_file = result;

    return result;
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
#ifdef FURI_DEBUG
        FURI_LOG_I(TAG, "Save done!");
#endif
        // This item contains fake data to load from SD
        item->is_file = true;
    } else {
        FURI_LOG_E(TAG, "Stream copy failed!");
    }
}