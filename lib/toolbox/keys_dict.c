#include "keys_dict.h"

#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <toolbox/args.h>

#define TAG "KeysDict"

struct KeysDict {
    Stream* stream;
    size_t key_size;
    size_t key_size_symbols;
    size_t total_keys;
};

static inline void keys_dict_add_ending_new_line(KeysDict* instance) {
    if(stream_seek(instance->stream, -1, StreamOffsetFromEnd)) {
        uint8_t last_char = 0;

        // Check if the last char is new line or add a new line
        if(stream_read(instance->stream, &last_char, 1) == 1 && last_char != '\n') {
            FURI_LOG_D(TAG, "Adding new line ending");
            stream_write_char(instance->stream, '\n');
        }

        stream_rewind(instance->stream);
    }
}

static bool keys_dict_read_key_line(KeysDict* instance, FuriString* line, bool* is_endfile) {
    if(stream_read_line(instance->stream, line) == false) {
        *is_endfile = true;
    }

    else {
        FURI_LOG_T(
            TAG, "Read line: %s, len: %zu", furi_string_get_cstr(line), furi_string_size(line));

        bool is_comment = furi_string_get_char(line, 0) == '#';

        if(!is_comment) {
            furi_string_left(line, instance->key_size_symbols - 1);
        }

        bool is_correct_size = furi_string_size(line) == instance->key_size_symbols - 1;

        return !is_comment && is_correct_size;
    }

    return false;
}

bool keys_dict_check_presence(const char* path) {
    furi_check(path);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool dict_present = storage_common_stat(storage, path, NULL) == FSE_OK;

    furi_record_close(RECORD_STORAGE);

    return dict_present;
}

KeysDict* keys_dict_alloc(const char* path, KeysDictMode mode, size_t key_size) {
    furi_check(path);
    furi_check(key_size > 0);

    KeysDict* instance = malloc(sizeof(KeysDict));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->stream = buffered_file_stream_alloc(storage);

    FS_OpenMode open_mode = (mode == KeysDictModeOpenAlways) ? FSOM_OPEN_ALWAYS :
                                                               FSOM_OPEN_EXISTING;

    // Byte = 2 symbols + 1 end of line
    instance->key_size = key_size;
    instance->key_size_symbols = key_size * 2 + 1;

    instance->total_keys = 0;

    bool file_exists =
        buffered_file_stream_open(instance->stream, path, FSAM_READ_WRITE, open_mode);

    if(!file_exists) {
        buffered_file_stream_close(instance->stream);
    } else {
        // Eventually add new line character in the last line to avoid skipping keys
        keys_dict_add_ending_new_line(instance);
    }

    FuriString* line = furi_string_alloc();

    bool is_endfile = false;

    // In this loop we only count the entries in the file
    // We prefer not to load the whole file in memory for space reasons
    while(file_exists && !is_endfile) {
        bool read_key = keys_dict_read_key_line(instance, line, &is_endfile);
        if(read_key) {
            instance->total_keys++;
        }
    }
    stream_rewind(instance->stream);
    FURI_LOG_I(TAG, "Loaded dictionary with %zu keys", instance->total_keys);

    furi_string_free(line);

    return instance;
}

void keys_dict_free(KeysDict* instance) {
    furi_check(instance);
    furi_check(instance->stream);

    buffered_file_stream_close(instance->stream);
    stream_free(instance->stream);
    free(instance);

    furi_record_close(RECORD_STORAGE);
}

static void keys_dict_int_to_str(KeysDict* instance, const uint8_t* key_int, FuriString* key_str) {
    furi_assert(instance);
    furi_assert(key_str);
    furi_assert(key_int);

    furi_string_reset(key_str);

    for(size_t i = 0; i < instance->key_size; i++)
        furi_string_cat_printf(key_str, "%02X", key_int[i]);
}

static void keys_dict_str_to_int(KeysDict* instance, FuriString* key_str, uint64_t* key_int) {
    furi_assert(instance);
    furi_assert(key_str);
    furi_assert(key_int);

    uint8_t key_byte_tmp;
    char h, l;

    *key_int = 0ULL;

    for(size_t i = 0; i < instance->key_size_symbols - 1; i += 2) {
        h = furi_string_get_char(key_str, i);
        l = furi_string_get_char(key_str, i + 1);

        args_char_to_hex(h, l, &key_byte_tmp);
        *key_int |= (uint64_t)key_byte_tmp << (8 * (instance->key_size - 1 - i / 2));
    }
}

size_t keys_dict_get_total_keys(KeysDict* instance) {
    furi_check(instance);

    return instance->total_keys;
}

bool keys_dict_rewind(KeysDict* instance) {
    furi_check(instance);
    furi_check(instance->stream);

    return stream_rewind(instance->stream);
}

static bool keys_dict_get_next_key_str(KeysDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);
    furi_assert(key);

    bool key_read = false;
    bool is_endfile = false;

    furi_string_reset(key);

    while(!key_read && !is_endfile)
        key_read = keys_dict_read_key_line(instance, key, &is_endfile);

    return key_read;
}

bool keys_dict_get_next_key(KeysDict* instance, uint8_t* key, size_t key_size) {
    furi_check(instance);
    furi_check(instance->stream);
    furi_check(instance->key_size == key_size);
    furi_check(key);

    FuriString* temp_key = furi_string_alloc();

    bool key_read = keys_dict_get_next_key_str(instance, temp_key);

    if(key_read) {
        size_t tmp_len = key_size;
        uint64_t key_int = 0;

        keys_dict_str_to_int(instance, temp_key, &key_int);

        while(tmp_len--) {
            key[tmp_len] = (uint8_t)key_int;
            key_int >>= 8;
        }
    }

    furi_string_free(temp_key);
    return key_read;
}

static bool keys_dict_is_key_present_str(KeysDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);
    furi_assert(key);

    FuriString* line = furi_string_alloc();

    bool is_endfile = false;
    bool line_found = false;

    uint32_t actual_pos = stream_tell(instance->stream);
    stream_rewind(instance->stream);

    while(!line_found && !is_endfile)
        line_found = // The line is found if the line was read and the key is equal to the line
            (keys_dict_read_key_line(instance, line, &is_endfile)) &&
            (furi_string_equal(key, line));

    furi_string_free(line);

    // Restore the position of the stream
    stream_seek(instance->stream, actual_pos, StreamOffsetFromStart);

    return line_found;
}

bool keys_dict_is_key_present(KeysDict* instance, const uint8_t* key, size_t key_size) {
    furi_check(instance);
    furi_check(instance->stream);
    furi_check(instance->key_size == key_size);
    furi_check(key);

    FuriString* temp_key = furi_string_alloc();

    keys_dict_int_to_str(instance, key, temp_key);
    bool key_found = keys_dict_is_key_present_str(instance, temp_key);
    furi_string_free(temp_key);

    return key_found;
}

static bool keys_dict_add_key_str(KeysDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);
    furi_assert(key);

    furi_string_cat_str(key, "\n");

    bool key_added = false;

    uint32_t actual_pos = stream_tell(instance->stream);

    if(stream_seek(instance->stream, 0, StreamOffsetFromEnd) &&
       stream_insert_string(instance->stream, key)) {
        instance->total_keys++;
        key_added = true;
    }

    stream_seek(instance->stream, actual_pos, StreamOffsetFromStart);

    return key_added;
}

bool keys_dict_add_key(KeysDict* instance, const uint8_t* key, size_t key_size) {
    furi_check(instance);
    furi_check(instance->stream);
    furi_check(instance->key_size == key_size);
    furi_check(key);

    FuriString* temp_key = furi_string_alloc();

    keys_dict_int_to_str(instance, key, temp_key);
    bool key_added = keys_dict_add_key_str(instance, temp_key);

    FURI_LOG_I(TAG, "Added key %s", furi_string_get_cstr(temp_key));

    furi_string_free(temp_key);

    return key_added;
}

bool keys_dict_delete_key(KeysDict* instance, const uint8_t* key, size_t key_size) {
    furi_check(instance);
    furi_check(instance->stream);
    furi_check(instance->key_size == key_size);
    furi_check(key);

    bool key_removed = false;

    uint8_t* temp_key = malloc(key_size);

    stream_rewind(instance->stream);

    while(!key_removed) {
        if(!keys_dict_get_next_key(instance, temp_key, key_size)) {
            break;
        }

        if(memcmp(temp_key, key, key_size) == 0) {
            stream_seek(instance->stream, -instance->key_size_symbols, StreamOffsetFromCurrent);
            if(stream_delete(instance->stream, instance->key_size_symbols) == false) {
                break;
            }
            instance->total_keys--;
            key_removed = true;
        }
    }

    FuriString* tmp = furi_string_alloc();

    keys_dict_int_to_str(instance, key, tmp);

    FURI_LOG_I(TAG, "Removed key %s", furi_string_get_cstr(tmp));

    furi_string_free(tmp);

    stream_rewind(instance->stream);
    free(temp_key);

    return key_removed;
}
