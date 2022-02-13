#include "flipper_file_helper.h"

const char* const flipper_file_filetype_key = "Filetype";
const char* const flipper_file_version_key = "Version";
const char flipper_file_delimiter = ':';
const char flipper_file_comment = '#';

#ifdef __linux__
const char* flipper_file_scratchpad = ".scratch.pad";
#else
const char* flipper_file_scratchpad = "/any/.scratch.pad";
#endif

bool flipper_file_read_valid_key(File* file, string_t key) {
    string_reset(key);
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
                string_reset(key);
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
                    string_reset(key);
                    accumulate = false;
                    new_line = false;
                } else {
                    // parse the delimiter only if we are accumulating data
                    if(accumulate) {
                        // we found the delimiter, move the rw pointer to the correct location
                        // and signal that we have found something
                        if(!file_helper_seek(file, i - bytes_were_read)) {
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

bool flipper_file_seek_to_key(File* file, const char* key, bool strict_mode) {
    bool found = false;
    string_t readed_key;

    string_init(readed_key);

    while(!storage_file_eof(file)) {
        if(flipper_file_read_valid_key(file, readed_key)) {
            if(string_cmp_str(readed_key, key) == 0) {
                if(!file_helper_seek(file, 2)) break;

                found = true;
                break;
            } else if(strict_mode) {
                found = false;
                break;
            }
        }
    }
    string_clear(readed_key);

    return found;
}

bool flipper_file_write_key(File* file, const char* key) {
    bool result = false;

    do {
        result = file_helper_write(file, key, strlen(key));
        if(!result) break;

        const char delimiter_buffer[2] = {flipper_file_delimiter, ' '};
        result = file_helper_write(file, delimiter_buffer, sizeof(delimiter_buffer));
    } while(false);

    return result;
}

bool flipper_file_get_scratchpad_name(const char** name) {
    // TODO do not rewrite existing file
    *name = flipper_file_scratchpad;
    return true;
}
