#include <furi.h>
#include "flipper-file.h"
#include <toolbox/hex.h>
#include <inttypes.h>

struct FlipperFile {
    File* file;
};

const char* flipper_file_filetype_key = "Filetype";
const char* flipper_file_version_key = "Version";
const char flipper_file_eoln = '\n';
const char flipper_file_eolr = '\r';
const char flipper_file_delimiter = ':';
const char flipper_file_comment = '#';

/**
 * Writes data to a file as a hexadecimal array.
 * @param file 
 * @param data 
 * @param data_size 
 * @return true on success write 
 */
bool flipper_file_write_hex_internal(File* file, const uint8_t* data, const uint16_t data_size) {
    const uint8_t byte_text_size = 3;
    char byte_text[byte_text_size];

    bool result = true;
    uint16_t bytes_written;
    for(uint8_t i = 0; i < data_size; i++) {
        snprintf(byte_text, byte_text_size, "%02X", data[i]);

        if(i != 0) {
            // space
            const char space = ' ';
            bytes_written = storage_file_write(file, &space, sizeof(space));
            if(bytes_written != sizeof(space)) {
                result = false;
                break;
            }
        }

        bytes_written = storage_file_write(file, &byte_text, strlen(byte_text));
        if(bytes_written != strlen(byte_text)) {
            result = false;
            break;
        }
    }

    return result;
}

/**
 * Reads a valid key from a file as a string.
 * After reading, the rw pointer will be on the flipper_file_delimiter symbol.
 * Optimized not to read comments and values into RAM.
 * @param file 
 * @param key 
 * @return true on success read 
 */
bool flipper_file_read_valid_key(File* file, string_t key) {
    string_clean(key);
    bool found = false;
    bool error = false;
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    bool accumulate = true;
    bool new_line = true;

    while(true) {
        uint16_t bytes_were_read = storage_file_read(file, buffer, buffer_size);
        if(bytes_were_read == 0) break;

        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_file_eoln) {
                // EOL found, clean data, start accumulating data and set the new_line flag
                string_clean(key);
                accumulate = true;
                new_line = true;
            } else if(buffer[i] == flipper_file_eolr) {
                // Ignore
            } else if(buffer[i] == flipper_file_comment && new_line) {
                // if there is a comment character and we are at the beginning of a new line
                // do not accumulate comment data and reset the new_line flag
                accumulate = false;
                new_line = false;
            } else if(buffer[i] == flipper_file_delimiter) {
                if(new_line) {
                    // we are on a "new line" and found the delimiter
                    // this can only be if we have previously found some kind of key, so
                    // clear the data, set the flag that we no longer want to accumulate data
                    // and reset the new_line flag
                    string_clean(key);
                    accumulate = false;
                    new_line = false;
                } else {
                    // parse the delimiter only if we are accumulating data
                    if(accumulate) {
                        // we found the delimiter, move the rw pointer to the correct location
                        // and signal that we have found something
                        // TODO negative seek
                        uint64_t position = storage_file_tell(file);
                        position = position - bytes_were_read + i;
                        if(!storage_file_seek(file, position, true)) {
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
                    string_push_back(key, buffer[i]);
                }
            }
        }

        if(found || error) break;
    }

    return found;
}

/**
 * Sets rw pointer to the data after the key
 * @param file 
 * @param key 
 * @return true if key was found 
 */
bool flipper_file_seek_to_key(File* file, const char* key) {
    bool found = false;
    string_t readed_key;

    string_init(readed_key);

    // TODO optimize this to search from a stored rw pointer
    if(storage_file_seek(file, 0, true)) {
        while(!storage_file_eof(file)) {
            if(flipper_file_read_valid_key(file, readed_key)) {
                if(string_cmp_str(readed_key, key) == 0) {
                    uint64_t position = storage_file_tell(file);
                    if(!storage_file_seek(file, position + 2, true)) break;

                    found = true;
                    break;
                }
            }
        }
    }
    string_clear(readed_key);

    return found;
}

/**
 * Reads data as a string from the stored rw pointer to the \r or \n symbol position
 * @param file 
 * @param str_result 
 * @return true on success read
 */
bool flipper_file_read_until(File* file, string_t str_result) {
    string_clean(str_result);
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];

    do {
        uint16_t bytes_were_read = storage_file_read(file, buffer, buffer_size);
        if(bytes_were_read == 0) break;

        bool result = false;
        bool error = false;
        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_file_eoln) {
                // TODO negative seek
                uint64_t position = storage_file_tell(file);
                position = position - bytes_were_read + i;
                if(!storage_file_seek(file, position, true)) {
                    error = true;
                    break;
                }

                result = true;
                break;
            } else if(buffer[i] == flipper_file_eolr) {
                // Ignore
            } else {
                string_push_back(str_result, buffer[i]);
            }
        }

        if(result || error) {
            break;
        }
    } while(true);

    return string_size(str_result) != 0;
}

/**
 * Reads single hexadecimal data from a file to byte
 * @param file 
 * @param byte 
 * @return bool 
 */
bool flipper_file_read_hex_byte(File* file, uint8_t* byte) {
    uint8_t hi_nibble_value, low_nibble_value;
    uint8_t text[3];
    bool result = false;

    uint16_t bytes_were_read = storage_file_read(file, text, 3);
    if(bytes_were_read >= 2) {
        if(text[0] != ' ') {
            if(hex_char_to_hex_nibble(text[0], &hi_nibble_value) &&
               hex_char_to_hex_nibble(text[1], &low_nibble_value)) {
                *byte = (hi_nibble_value << 4) | low_nibble_value;
                result = true;
            }
        } else {
            if(hex_char_to_hex_nibble(text[1], &hi_nibble_value) &&
               hex_char_to_hex_nibble(text[2], &low_nibble_value)) {
                *byte = (hi_nibble_value << 4) | low_nibble_value;
                result = true;
            }
        }
    }

    return result;
}

FlipperFile* flipper_file_alloc(Storage* storage) {
    FlipperFile* flipper_file = malloc(sizeof(FlipperFile));
    flipper_file->file = storage_file_alloc(storage);
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

bool flipper_file_open_read(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);
    bool result = storage_file_open(flipper_file->file, filename, FSAM_READ, FSOM_OPEN_EXISTING);
    return result;
}

bool flipper_file_new_write(FlipperFile* flipper_file, const char* filename) {
    furi_assert(flipper_file);
    bool result = storage_file_open(flipper_file->file, filename, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    return result;
}

bool flipper_file_close(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    if(storage_file_is_open(flipper_file->file)) {
        return storage_file_close(flipper_file->file);
    }
    return true;
}

bool flipper_file_read_header(FlipperFile* flipper_file, string_t filetype, uint32_t* version) {
    bool result = false;
    do {
        result = flipper_file_read_string(flipper_file, flipper_file_filetype_key, filetype);
        if(!result) break;
        result = flipper_file_read_uint32(flipper_file, flipper_file_version_key, version);
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
        result = flipper_file_write_uint32(flipper_file, flipper_file_version_key, version);
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

bool flipper_file_read_string(FlipperFile* flipper_file, const char* key, string_t data) {
    furi_assert(flipper_file);

    bool result = false;
    if(flipper_file_seek_to_key(flipper_file->file, key)) {
        if(flipper_file_read_until(flipper_file->file, data)) {
            result = true;
        }
    }
    return result;
}

bool flipper_file_write_string(FlipperFile* flipper_file, const char* key, string_t data) {
    furi_assert(flipper_file);

    bool result = false;
    do {
        uint16_t bytes_written;
        bytes_written = storage_file_write(flipper_file->file, key, strlen(key));
        if(bytes_written != strlen(key)) break;

        const char delimiter_buffer[2] = {flipper_file_delimiter, ' '};
        bytes_written =
            storage_file_write(flipper_file->file, delimiter_buffer, sizeof(delimiter_buffer));
        if(bytes_written != sizeof(delimiter_buffer)) break;

        bytes_written =
            storage_file_write(flipper_file->file, string_get_cstr(data), string_size(data));
        if(bytes_written != string_size(data)) break;

        bytes_written =
            storage_file_write(flipper_file->file, &flipper_file_eoln, sizeof(flipper_file_eoln));
        if(bytes_written != sizeof(flipper_file_eoln)) break;

        result = true;
    } while(false);

    return result;
}

bool flipper_file_write_string_cstr(FlipperFile* flipper_file, const char* key, const char* data) {
    bool result = false;
    string_t value;
    string_init_set(value, data);
    result = flipper_file_write_string(flipper_file, key, value);
    string_clear(value);
    return result;
}

bool flipper_file_read_uint32(FlipperFile* flipper_file, const char* key, uint32_t* data) {
    bool result = false;
    string_t value;
    string_init(value);

    result = flipper_file_read_string(flipper_file, key, value);
    if(result) {
        int ret = sscanf(string_get_cstr(value), "%" PRIu32, data);
        if(ret != 1) result = false;
    }

    string_clear(value);
    return result;
}

bool flipper_file_write_uint32(FlipperFile* flipper_file, const char* key, const uint32_t data) {
    bool result = false;
    string_t value;
    string_init_printf(value, "%" PRIu32, data);
    result = flipper_file_write_string(flipper_file, key, value);
    string_clear(value);
    return result;
}

bool flipper_file_write_comment(FlipperFile* flipper_file, string_t data) {
    furi_assert(flipper_file);

    bool result = false;
    do {
        uint16_t bytes_written;
        const char comment_buffer[2] = {flipper_file_comment, ' '};
        bytes_written =
            storage_file_write(flipper_file->file, comment_buffer, sizeof(comment_buffer));
        if(bytes_written != sizeof(comment_buffer)) break;

        bytes_written =
            storage_file_write(flipper_file->file, string_get_cstr(data), string_size(data));
        if(bytes_written != string_size(data)) break;

        bytes_written =
            storage_file_write(flipper_file->file, &flipper_file_eoln, sizeof(flipper_file_eoln));
        if(bytes_written != sizeof(flipper_file_eoln)) break;

        result = true;
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

bool flipper_file_write_hex_array(
    FlipperFile* flipper_file,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);

    bool result = false;
    do {
        uint16_t bytes_written;
        bytes_written = storage_file_write(flipper_file->file, key, strlen(key));
        if(bytes_written != strlen(key)) break;

        const char delimiter_buffer[2] = {flipper_file_delimiter, ' '};
        bytes_written =
            storage_file_write(flipper_file->file, delimiter_buffer, sizeof(delimiter_buffer));
        if(bytes_written != sizeof(delimiter_buffer)) break;

        if(!flipper_file_write_hex_internal(flipper_file->file, data, data_size)) break;

        bytes_written =
            storage_file_write(flipper_file->file, &flipper_file_eoln, sizeof(flipper_file_eoln));
        if(bytes_written != sizeof(flipper_file_eoln)) break;

        result = true;
    } while(false);

    return result;
}

bool flipper_file_read_hex_array(
    FlipperFile* flipper_file,
    const char* key,
    uint8_t* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);

    bool result = false;
    if(flipper_file_seek_to_key(flipper_file->file, key)) {
        result = true;
        for(uint16_t i = 0; i < data_size; i++) {
            if(!flipper_file_read_hex_byte(flipper_file->file, &data[i])) {
                result = false;
                break;
            }
        }
    }
    return result;
}

File* flipper_file_get_file(FlipperFile* flipper_file) {
    furi_assert(flipper_file);
    furi_assert(flipper_file->file);

    return flipper_file->file;
}
