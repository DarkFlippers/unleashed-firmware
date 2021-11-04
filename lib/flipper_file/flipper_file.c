#include <furi.h>
#include "file_helper.h"
#include "flipper_file_helper.h"
#include "flipper_file.h"
#include "flipper_file_i.h"
#include <inttypes.h>
#include <toolbox/hex.h>

FlipperFile* flipper_file_alloc(Storage* storage) {
    // furi_assert(storage);

    FlipperFile* flipper_file = malloc(sizeof(FlipperFile));
    flipper_file->storage = storage;
    flipper_file->file = storage_file_alloc(flipper_file->storage);

    return flipper_file;
}

void flipper_file_free(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    if(storage_file_is_open(flipper_file->file)) {
        storage_file_close(flipper_file->file);
    }
    storage_file_free(flipper_file->file);
    free(flipper_file);
}

bool flipper_file_open_existing(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);
    bool result = storage_file_open(
        flipper_file->file, filename, FSAM_READ | FSAM_WRITE, FSOM_OPEN_EXISTING);
    return result;
}

bool flipper_file_open_append(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);

    bool result =
        storage_file_open(flipper_file->file, filename, FSAM_READ | FSAM_WRITE, FSOM_OPEN_APPEND);

    // Add EOL if it is not there
    if(storage_file_size(flipper_file->file) >= 1) {
        do {
            char last_char;
            result = false;

            if(!file_helper_seek(flipper_file->file, -1)) break;

            uint16_t bytes_were_read = storage_file_read(flipper_file->file, &last_char, 1);
            if(bytes_were_read != 1) break;

            if(last_char != flipper_file_eoln) {
                if(!file_helper_write_eol(flipper_file->file)) break;
            }

            result = true;
        } while(false);
    }

    return result;
}

bool flipper_file_open_always(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);
    bool result = storage_file_open(
        flipper_file->file, filename, FSAM_READ | FSAM_WRITE, FSOM_CREATE_ALWAYS);
    return result;
}

bool flipper_file_open_new(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);
    bool result = storage_file_open(
        flipper_file->file, filename, FSAM_READ | FSAM_WRITE, FSOM_CREATE_NEW);
    return result;
}

bool flipper_file_close(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    if(storage_file_is_open(flipper_file->file)) {
        return storage_file_close(flipper_file->file);
    }
    return true;
}

bool flipper_file_rewind(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    return storage_file_seek(flipper_file->file, 0, true);
}

bool flipper_file_read_header(FlipperFile* flipper_file, string_t filetype, uint32_t* version) {
    bool result = false;
    do {
        result = flipper_file_read_string(flipper_file, flipper_file_filetype_key, filetype);
        if(!result) break;
        result = flipper_file_read_uint32(flipper_file, flipper_file_version_key, version, 1);
        if(!result) break;
    } while(false);

    return result;
}

bool flipper_file_write_header(
    FlipperFile* flipper_file,
    string_t filetype,
    const uint32_t version) {
    bool result = false;
    do {
        result = flipper_file_write_string(flipper_file, flipper_file_filetype_key, filetype);
        if(!result) break;
        result = flipper_file_write_uint32(flipper_file, flipper_file_version_key, &version, 1);
        if(!result) break;
    } while(false);

    return result;
}

bool flipper_file_write_header_cstr(
    FlipperFile* flipper_file,
    const char* filetype,
    const uint32_t version) {
    bool result = false;
    string_t value;
    string_init_set(value, filetype);
    result = flipper_file_write_header(flipper_file, value, version);
    string_clear(value);
    return result;
}

bool flipper_file_get_value_count(FlipperFile* flipper_file, const char* key, uint32_t* count) {
    furi_assert(flipper_file);
    bool result = false;
    bool last = false;

    string_t value;
    string_init(value);

    uint32_t position = storage_file_tell(flipper_file->file);
    do {
        if(!flipper_file_seek_to_key(flipper_file->file, key)) break;

        // Balance between speed and memory consumption
        // I prefer lower speed but less memory consumption
        *count = 0;

        result = true;
        while(true) {
            if(!file_helper_read_value(flipper_file->file, value, &last)) {
                result = false;
                break;
            }

            *count = *count + 1;
            if(last) break;
        }

    } while(true);

    if(!storage_file_seek(flipper_file->file, position, true)) {
        result = false;
    }

    string_clear(value);
    return result;
}

bool flipper_file_write_comment(FlipperFile* flipper_file, string_t data) {
    furi_assert(flipper_file);

    bool result = false;
    do {
        const char comment_buffer[2] = {flipper_file_comment, ' '};
        result = file_helper_write(flipper_file->file, comment_buffer, sizeof(comment_buffer));
        if(!result) break;

        result = file_helper_write(flipper_file->file, string_get_cstr(data), string_size(data));
        if(!result) break;

        result = file_helper_write_eol(flipper_file->file);
    } while(false);

    return result;
}

bool flipper_file_write_comment_cstr(FlipperFile* flipper_file, const char* data) {
    bool result = false;
    string_t value;
    string_init_set(value, data);
    result = flipper_file_write_comment(flipper_file, value);
    string_clear(value);
    return result;
}

bool flipper_file_delete_key_and_call(
    FlipperFile* flipper_file,
    const char* key,
    flipper_file_cb call,
    const char* cb_key,
    const void* cb_data,
    const uint16_t cb_data_size) {
    bool result = false;
    File* scratch_file = storage_file_alloc(flipper_file->storage);

    do {
        // get size
        uint64_t file_size = storage_file_size(flipper_file->file);
        if(file_size == 0) break;

        if(!storage_file_seek(flipper_file->file, 0, true)) break;

        // find key
        if(!flipper_file_seek_to_key(flipper_file->file, key)) break;
        // get key start position
        uint64_t start_position = storage_file_tell(flipper_file->file) - strlen(key);
        if(start_position >= 2) {
            start_position -= 2;
        } else {
            // something wrong
            break;
        }

        // get value end position
        if(!file_helper_seek_to_next_line(flipper_file->file)) break;
        uint64_t end_position = storage_file_tell(flipper_file->file);
        // newline symbol
        if(end_position < file_size) {
            end_position += 1;
        }

        // open scratchpad
        const char* scratch_name = "";
        if(!flipper_file_get_scratchpad_name(&scratch_name)) break;

        if(!storage_file_open(
               scratch_file, scratch_name, FSAM_READ | FSAM_WRITE, FSOM_CREATE_ALWAYS))
            break;

        // copy key file before key to scratchpad
        if(!file_helper_copy(flipper_file->file, scratch_file, 0, start_position)) break;

        // do something in between if needed
        if(call != NULL) {
            if(!call(scratch_file, cb_key, cb_data, cb_data_size)) break;
        };

        // copy key file after key value to scratchpad
        if(!file_helper_copy(flipper_file->file, scratch_file, end_position, file_size)) break;

        file_size = storage_file_tell(scratch_file);
        if(file_size == 0) break;

        if(!storage_file_seek(flipper_file->file, 0, true)) break;

        // copy whole scratchpad file to the original file
        if(!file_helper_copy(scratch_file, flipper_file->file, 0, file_size)) break;

        // and truncate original file
        if(!storage_file_truncate(flipper_file->file)) break;

        // close and remove scratchpad file
        if(!storage_file_close(scratch_file)) break;
        if(storage_common_remove(flipper_file->storage, scratch_name) != FSE_OK) break;
        result = true;
    } while(false);

    storage_file_free(scratch_file);

    return result;
}

bool flipper_file_delete_key(FlipperFile* flipper_file, const char* key) {
    furi_assert(flipper_file);
    return flipper_file_delete_key_and_call(flipper_file, key, NULL, NULL, NULL, 0);
}

bool flipper_file_write_internal(
    File* file,
    const char* key,
    const void* _data,
    const uint16_t data_size,
    FlipperFileValueType type) {
    bool result = false;
    string_t value;
    string_init(value);

    do {
        result = flipper_file_write_key(file, key);
        if(!result) break;

        for(uint16_t i = 0; i < data_size; i++) {
            switch(type) {
            case FlipperFileValueHex: {
                const uint8_t* data = _data;
                string_printf(value, "%02X", data[i]);
            }; break;
            case FlipperFileValueFloat: {
                const float* data = _data;
                string_printf(value, "%f", data[i]);
            }; break;
            case FlipperFileValueInt32: {
                const int32_t* data = _data;
                string_printf(value, "%" PRIi32, data[i]);
            }; break;
            case FlipperFileValueUint32: {
                const uint32_t* data = _data;
                string_printf(value, "%" PRId32, data[i]);
            }; break;
            }

            if((i + 1) < data_size) {
                string_cat(value, " ");
            }

            result = file_helper_write(file, string_get_cstr(value), string_size(value));
            if(!result) break;
        }

        result = file_helper_write_eol(file);
    } while(false);

    string_clear(value);
    return result;
}

bool flipper_file_read_internal(
    File* file,
    const char* key,
    void* _data,
    const uint16_t data_size,
    FlipperFileValueType type) {
    bool result = false;
    string_t value;
    string_init(value);

    if(flipper_file_seek_to_key(file, key)) {
        result = true;
        for(uint16_t i = 0; i < data_size; i++) {
            bool last = false;
            result = file_helper_read_value(file, value, &last);
            if(result) {
                int scan_values = 0;
                switch(type) {
                case FlipperFileValueHex: {
                    uint8_t* data = _data;
                    // sscanf "%02X" does not work here
                    if(hex_chars_to_uint8(
                           string_get_char(value, 0), string_get_char(value, 1), &data[i])) {
                        scan_values = 1;
                    }
                }; break;
                case FlipperFileValueFloat: {
                    float* data = _data;
                    // newlib-nano does not have sscanf for floats
                    // scan_values = sscanf(string_get_cstr(value), "%f", &data[i]);
                    char* end_char;
                    data[i] = strtof(string_get_cstr(value), &end_char);
                    if(*end_char == 0) {
                        // very probably ok
                        scan_values = 1;
                    }
                }; break;
                case FlipperFileValueInt32: {
                    int32_t* data = _data;
                    scan_values = sscanf(string_get_cstr(value), "%" PRIi32, &data[i]);
                }; break;
                case FlipperFileValueUint32: {
                    uint32_t* data = _data;
                    scan_values = sscanf(string_get_cstr(value), "%" PRId32, &data[i]);
                }; break;
                }

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
    }

    string_clear(value);
    return result;
}

File* flipper_file_get_file(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    furi_assert(flipper_file->file);

    return flipper_file->file;
}