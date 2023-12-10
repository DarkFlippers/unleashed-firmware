#include "nfc_dict.h"

#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>
#include <toolbox/args.h>

#include <nfc/helpers/nfc_util.h>

#define TAG "NfcDict"

struct NfcDict {
    Stream* stream;
    size_t key_size;
    size_t key_size_symbols;
    uint32_t total_keys;
};

typedef struct {
    const char* path;
    FS_OpenMode open_mode;
} NfcDictFile;

bool nfc_dict_check_presence(const char* path) {
    furi_assert(path);

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool dict_present = storage_common_stat(storage, path, NULL) == FSE_OK;

    furi_record_close(RECORD_STORAGE);

    return dict_present;
}

NfcDict* nfc_dict_alloc(const char* path, NfcDictMode mode, size_t key_size) {
    furi_assert(path);

    NfcDict* instance = malloc(sizeof(NfcDict));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    instance->stream = buffered_file_stream_alloc(storage);
    furi_record_close(RECORD_STORAGE);

    FS_OpenMode open_mode = FSOM_OPEN_EXISTING;
    if(mode == NfcDictModeOpenAlways) {
        open_mode = FSOM_OPEN_ALWAYS;
    }
    instance->key_size = key_size;
    // Byte = 2 symbols + 1 end of line
    instance->key_size_symbols = key_size * 2 + 1;

    bool dict_loaded = false;
    do {
        if(!buffered_file_stream_open(instance->stream, path, FSAM_READ_WRITE, open_mode)) {
            buffered_file_stream_close(instance->stream);
            break;
        }

        // Check for new line ending
        if(!stream_eof(instance->stream)) {
            if(!stream_seek(instance->stream, -1, StreamOffsetFromEnd)) break;
            uint8_t last_char = 0;
            if(stream_read(instance->stream, &last_char, 1) != 1) break;
            if(last_char != '\n') {
                FURI_LOG_D(TAG, "Adding new line ending");
                if(stream_write_char(instance->stream, '\n') != 1) break;
            }
            if(!stream_rewind(instance->stream)) break;
        }

        // Read total amount of keys
        FuriString* next_line;
        next_line = furi_string_alloc();
        while(true) {
            if(!stream_read_line(instance->stream, next_line)) {
                FURI_LOG_T(TAG, "No keys left in dict");
                break;
            }
            FURI_LOG_T(
                TAG,
                "Read line: %s, len: %zu",
                furi_string_get_cstr(next_line),
                furi_string_size(next_line));
            if(furi_string_get_char(next_line, 0) == '#') continue;
            if(furi_string_size(next_line) != instance->key_size_symbols) continue;
            instance->total_keys++;
        }
        furi_string_free(next_line);
        stream_rewind(instance->stream);

        dict_loaded = true;
        FURI_LOG_I(TAG, "Loaded dictionary with %lu keys", instance->total_keys);
    } while(false);

    if(!dict_loaded) {
        buffered_file_stream_close(instance->stream);
        free(instance);
        instance = NULL;
    }

    return instance;
}

void nfc_dict_free(NfcDict* instance) {
    furi_assert(instance);
    furi_assert(instance->stream);

    buffered_file_stream_close(instance->stream);
    stream_free(instance->stream);
    free(instance);
}

static void nfc_dict_int_to_str(NfcDict* instance, const uint8_t* key_int, FuriString* key_str) {
    furi_string_reset(key_str);
    for(size_t i = 0; i < instance->key_size; i++) {
        furi_string_cat_printf(key_str, "%02X", key_int[i]);
    }
}

static void nfc_dict_str_to_int(NfcDict* instance, FuriString* key_str, uint64_t* key_int) {
    uint8_t key_byte_tmp;

    *key_int = 0ULL;
    for(uint8_t i = 0; i < instance->key_size * 2; i += 2) {
        args_char_to_hex(
            furi_string_get_char(key_str, i), furi_string_get_char(key_str, i + 1), &key_byte_tmp);
        *key_int |= (uint64_t)key_byte_tmp << (8 * (instance->key_size - 1 - i / 2));
    }
}

uint32_t nfc_dict_get_total_keys(NfcDict* instance) {
    furi_assert(instance);

    return instance->total_keys;
}

bool nfc_dict_rewind(NfcDict* instance) {
    furi_assert(instance);
    furi_assert(instance->stream);

    return stream_rewind(instance->stream);
}

static bool nfc_dict_get_next_key_str(NfcDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);

    bool key_read = false;
    furi_string_reset(key);
    while(!key_read) {
        if(!stream_read_line(instance->stream, key)) break;
        if(furi_string_get_char(key, 0) == '#') continue;
        if(furi_string_size(key) != instance->key_size_symbols) continue;
        furi_string_left(key, instance->key_size_symbols - 1);
        key_read = true;
    }

    return key_read;
}

bool nfc_dict_get_next_key(NfcDict* instance, uint8_t* key, size_t key_size) {
    furi_assert(instance);
    furi_assert(instance->stream);
    furi_assert(instance->key_size == key_size);

    FuriString* temp_key = furi_string_alloc();
    uint64_t key_int = 0;
    bool key_read = nfc_dict_get_next_key_str(instance, temp_key);
    if(key_read) {
        nfc_dict_str_to_int(instance, temp_key, &key_int);
        nfc_util_num2bytes(key_int, key_size, key);
    }
    furi_string_free(temp_key);
    return key_read;
}

static bool nfc_dict_is_key_present_str(NfcDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);

    FuriString* next_line;
    next_line = furi_string_alloc();

    bool key_found = false;
    stream_rewind(instance->stream);
    while(!key_found) { //-V654
        if(!stream_read_line(instance->stream, next_line)) break;
        if(furi_string_get_char(next_line, 0) == '#') continue;
        if(furi_string_size(next_line) != instance->key_size_symbols) continue;
        furi_string_left(next_line, instance->key_size_symbols - 1);
        if(!furi_string_equal(key, next_line)) continue;
        key_found = true;
    }

    furi_string_free(next_line);
    return key_found;
}

bool nfc_dict_is_key_present(NfcDict* instance, const uint8_t* key, size_t key_size) {
    furi_assert(instance);
    furi_assert(key);
    furi_assert(instance->stream);
    furi_assert(instance->key_size == key_size);

    FuriString* temp_key = furi_string_alloc();
    nfc_dict_int_to_str(instance, key, temp_key);
    bool key_found = nfc_dict_is_key_present_str(instance, temp_key);
    furi_string_free(temp_key);

    return key_found;
}

static bool nfc_dict_add_key_str(NfcDict* instance, FuriString* key) {
    furi_assert(instance);
    furi_assert(instance->stream);

    furi_string_cat_printf(key, "\n");

    bool key_added = false;
    do {
        if(!stream_seek(instance->stream, 0, StreamOffsetFromEnd)) break;
        if(!stream_insert_string(instance->stream, key)) break;
        instance->total_keys++;
        key_added = true;
    } while(false);

    furi_string_left(key, instance->key_size_symbols - 1);
    return key_added;
}

bool nfc_dict_add_key(NfcDict* instance, const uint8_t* key, size_t key_size) {
    furi_assert(instance);
    furi_assert(key);
    furi_assert(instance->stream);
    furi_assert(instance->key_size == key_size);

    FuriString* temp_key = furi_string_alloc();
    nfc_dict_int_to_str(instance, key, temp_key);
    bool key_added = nfc_dict_add_key_str(instance, temp_key);
    furi_string_free(temp_key);

    return key_added;
}

bool nfc_dict_delete_key(NfcDict* instance, const uint8_t* key, size_t key_size) {
    furi_assert(instance);
    furi_assert(instance->stream);
    furi_assert(key);
    furi_assert(instance->key_size == key_size);

    bool key_removed = false;
    uint8_t* temp_key = malloc(key_size);

    nfc_dict_rewind(instance);
    while(!key_removed) {
        if(!nfc_dict_get_next_key(instance, temp_key, key_size)) break;
        if(memcmp(temp_key, key, key_size) == 0) {
            int32_t offset = (-1) * (instance->key_size_symbols);
            stream_seek(instance->stream, offset, StreamOffsetFromCurrent);
            if(!stream_delete(instance->stream, instance->key_size_symbols)) break;
            instance->total_keys--;
            key_removed = true;
        }
    }
    nfc_dict_rewind(instance);
    free(temp_key);

    return key_removed;
}
