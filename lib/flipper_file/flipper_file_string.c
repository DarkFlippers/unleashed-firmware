#include <furi.h>

#include "flipper_file.h"
#include "flipper_file_i.h"
#include "flipper_file_helper.h"

static bool flipper_file_write_string_internal(
    File* file,
    const char* key,
    const void* data,
    const uint16_t data_size) {
    bool result = false;
    (void)data_size;

    do {
        result = flipper_file_write_key(file, key);
        if(!result) break;

        result = file_helper_write(file, string_get_cstr(data), string_size(data));
        if(!result) break;

        result = file_helper_write_eol(file);
    } while(false);

    return result;
};

bool flipper_file_read_string(FlipperFile* flipper_file, const char* key, string_t data) {
    furi_assert(flipper_file);

    bool result = false;
    if(flipper_file_seek_to_key(flipper_file->file, key, flipper_file->strict_mode)) {
        if(file_helper_read_line(flipper_file->file, data)) {
            result = true;
        }
    }
    return result;
}

bool flipper_file_write_string(FlipperFile* flipper_file, const char* key, string_t data) {
    furi_assert(flipper_file);
    return flipper_file_write_string_internal(flipper_file->file, key, data, 0);
}

bool flipper_file_write_string_cstr(FlipperFile* flipper_file, const char* key, const char* data) {
    bool result = false;
    string_t value;
    string_init_set(value, data);
    result = flipper_file_write_string(flipper_file, key, value);
    string_clear(value);
    return result;
}

bool flipper_file_update_string(FlipperFile* flipper_file, const char* key, string_t data) {
    furi_assert(flipper_file);
    return flipper_file_delete_key_and_call(
        flipper_file, key, flipper_file_write_string_internal, key, data, 0);
}

bool flipper_file_update_string_cstr(FlipperFile* flipper_file, const char* key, const char* data) {
    bool result = false;
    string_t value;
    string_init_set(value, data);
    result = flipper_file_update_string(flipper_file, key, value);
    string_clear(value);
    return result;
}
