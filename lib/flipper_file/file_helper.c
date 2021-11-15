#include "file_helper.h"

const char flipper_file_eoln = '\n';
const char flipper_file_eolr = '\r';

bool file_helper_seek(File* file, int32_t offset) {
    uint64_t position = storage_file_tell(file);
    return storage_file_seek(file, position + offset, true);
}

bool file_helper_write_hex(File* file, const uint8_t* data, const uint16_t data_size) {
    const uint8_t byte_text_size = 3;
    char byte_text[byte_text_size];

    bool result = true;
    uint16_t bytes_written;
    for(uint8_t i = 0; i < data_size; i++) {
        snprintf(byte_text, byte_text_size, "%02X", data[i]);

        if(i != 0) {
            // space
            const char space = ' ';
            bytes_written = storage_file_write(file, &space, sizeof(char));
            if(bytes_written != sizeof(char)) {
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

bool file_helper_read_line(File* file, string_t str_result) {
    string_reset(str_result);
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];

    do {
        uint16_t bytes_were_read = storage_file_read(file, buffer, buffer_size);
        // TODO process EOF
        if(bytes_were_read == 0) break;

        bool result = false;
        bool error = false;
        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_file_eoln) {
                if(!file_helper_seek(file, i - bytes_were_read)) {
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

bool file_helper_seek_to_next_line(File* file) {
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    bool result = false;
    bool error = false;

    do {
        uint16_t bytes_were_read = storage_file_read(file, buffer, buffer_size);
        if(bytes_were_read == 0) {
            if(storage_file_eof(file)) {
                result = true;
                break;
            }
        }

        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_file_eoln) {
                if(!file_helper_seek(file, i - bytes_were_read)) {
                    error = true;
                    break;
                }

                result = true;
                break;
            }
        }

        if(result || error) {
            break;
        }
    } while(true);

    return result;
}

bool file_helper_read_value(File* file, string_t value, bool* last) {
    string_reset(value);
    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    bool result = false;
    bool error = false;

    while(true) {
        uint16_t bytes_were_read = storage_file_read(file, buffer, buffer_size);

        if(bytes_were_read == 0) {
            // check EOF
            if(storage_file_eof(file) && string_size(value) > 0) {
                result = true;
                *last = true;
                break;
            }
        }

        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_file_eoln) {
                if(string_size(value) > 0) {
                    if(!file_helper_seek(file, i - bytes_were_read)) {
                        error = true;
                        break;
                    }

                    result = true;
                    *last = true;
                    break;
                } else {
                    error = true;
                }
            } else if(buffer[i] == ' ') {
                if(string_size(value) > 0) {
                    if(!file_helper_seek(file, i - bytes_were_read)) {
                        error = true;
                        break;
                    }

                    result = true;
                    *last = false;
                    break;
                }

            } else if(buffer[i] == flipper_file_eolr) {
                // Ignore
            } else {
                string_push_back(value, buffer[i]);
            }
        }

        if(error || result) break;
    }

    return result;
}

bool file_helper_write(File* file, const void* data, uint16_t data_size) {
    uint16_t bytes_written = storage_file_write(file, data, data_size);
    return bytes_written == data_size;
}

bool file_helper_write_eol(File* file) {
    return file_helper_write(file, &flipper_file_eoln, sizeof(char));
}

bool file_helper_copy(File* file_from, File* file_to, uint64_t start_offset, uint64_t stop_offset) {
    bool result = false;

    const uint8_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    uint64_t current_offset = start_offset;

    if(storage_file_seek(file_from, start_offset, true)) {
        do {
            int32_t bytes_count = MIN(buffer_size, stop_offset - current_offset);
            if(bytes_count <= 0) {
                result = true;
                break;
            }

            uint16_t bytes_were_read = storage_file_read(file_from, buffer, bytes_count);
            if(bytes_were_read != bytes_count) break;

            uint16_t bytes_were_written = storage_file_write(file_to, buffer, bytes_count);
            if(bytes_were_written != bytes_count) break;

            current_offset += bytes_count;
        } while(true);
    }

    return result;
}