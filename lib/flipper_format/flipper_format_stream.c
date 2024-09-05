#include <inttypes.h>
#include <toolbox/hex.h>
#include <toolbox/strint.h>
#include <core/check.h>
#include "flipper_format_stream.h"
#include "flipper_format_stream_i.h"

static inline bool flipper_format_stream_is_space(char c) {
    return c == ' ' || c == '\t' || c == flipper_format_eolr;
}

static bool flipper_format_stream_write(Stream* stream, const void* data, size_t data_size) {
    size_t bytes_written = stream_write(stream, data, data_size);
    return bytes_written == data_size;
}

static bool flipper_format_stream_write_key(Stream* stream, const char* key) {
    bool result = false;

    do {
        if(!flipper_format_stream_write(stream, key, strlen(key))) break;
        if(!flipper_format_stream_write(stream, &flipper_format_delimiter, 1)) break;
        if(!flipper_format_stream_write(stream, " ", 1)) break;
        result = true;
    } while(false);

    return result;
}

bool flipper_format_stream_write_eol(Stream* stream) {
    return flipper_format_stream_write(stream, &flipper_format_eoln, 1);
}

static bool flipper_format_stream_read_valid_key(Stream* stream, FuriString* key) {
    furi_string_reset(key);
    const size_t buffer_size = 32;
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

bool flipper_format_stream_seek_to_key(Stream* stream, const char* key, bool strict_mode) {
    bool found = false;
    FuriString* read_key;

    read_key = furi_string_alloc();

    while(!stream_eof(stream)) {
        if(flipper_format_stream_read_valid_key(stream, read_key)) {
            if(furi_string_cmp_str(read_key, key) == 0) {
                if(!stream_seek(stream, 2, StreamOffsetFromCurrent)) break;

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

static bool flipper_format_stream_read_value(Stream* stream, FuriString* value, bool* last) {
    enum {
        LeadingSpace,
        ReadValue,
        TrailingSpace
    } state = LeadingSpace;
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
                if(flipper_format_stream_is_space(data)) {
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
                if(flipper_format_stream_is_space(data)) {
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
                if(flipper_format_stream_is_space(data)) {
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

static bool flipper_format_stream_read_line(Stream* stream, FuriString* str_result) {
    furi_string_reset(str_result);
    const size_t buffer_size = 32;
    uint8_t buffer[buffer_size];

    do {
        size_t was_read = stream_read(stream, buffer, buffer_size);
        if(was_read == 0) break;

        bool result = false;
        bool error = false;

        for(size_t i = 0; i < was_read; i++) {
            uint8_t data = buffer[i];
            if(data == flipper_format_eoln) {
                if(!stream_seek(stream, i - was_read, StreamOffsetFromCurrent)) {
                    error = true;
                    break;
                }

                result = true;
                break;
            } else if(data == flipper_format_eolr) {
                // Ignore
            } else {
                furi_string_push_back(str_result, data);
            }
        }

        if(result || error) {
            break;
        }
    } while(true);

    return furi_string_size(str_result) != 0;
}

static bool flipper_format_stream_seek_to_next_line(Stream* stream) {
    const size_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    bool result = false;
    bool error = false;

    do {
        size_t was_read = stream_read(stream, buffer, buffer_size);
        if(was_read == 0) {
            if(stream_eof(stream)) {
                result = true;
                break;
            }
        }

        for(size_t i = 0; i < was_read; i++) {
            if(buffer[i] == flipper_format_eoln) {
                if(!stream_seek(stream, i - was_read, StreamOffsetFromCurrent)) {
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

bool flipper_format_stream_write_value_line(Stream* stream, FlipperStreamWriteData* write_data) {
    bool result = false;

    if(write_data->type == FlipperStreamValueIgnore) {
        result = true;
    } else {
        FuriString* value;
        value = furi_string_alloc();

        do {
            if(!flipper_format_stream_write_key(stream, write_data->key)) break;

            if(write_data->type == FlipperStreamValueStr) write_data->data_size = 1;

            bool cycle_error = false;
            for(uint16_t i = 0; i < write_data->data_size; i++) {
                switch(write_data->type) {
                case FlipperStreamValueStr: {
                    const char* data = write_data->data;
                    furi_string_printf(value, "%s", data);
                }; break;
                case FlipperStreamValueHex: {
                    const uint8_t* data = write_data->data;
                    furi_string_printf(value, "%02X", data[i]);
                }; break;
#ifndef FLIPPER_STREAM_LITE
                case FlipperStreamValueFloat: {
                    const float* data = write_data->data;
                    furi_string_printf(value, "%f", (double)data[i]);
                }; break;
#endif
                case FlipperStreamValueInt32: {
                    const int32_t* data = write_data->data;
                    furi_string_printf(value, "%" PRIi32, data[i]);
                }; break;
                case FlipperStreamValueUint32: {
                    const uint32_t* data = write_data->data;
                    furi_string_printf(value, "%" PRIu32, data[i]);
                }; break;
                case FlipperStreamValueHexUint64: {
                    const uint64_t* data = write_data->data;
                    furi_string_printf(
                        value, "%08lX%08lX", (uint32_t)(data[i] >> 32), (uint32_t)data[i]);
                }; break;
                case FlipperStreamValueBool: {
                    const bool* data = write_data->data;
                    furi_string_printf(value, data[i] ? "true" : "false");
                }; break;
                default:
                    furi_crash("Unknown FF type");
                }

                if(((size_t)i + 1) < write_data->data_size) {
                    furi_string_cat(value, " ");
                }

                if(!flipper_format_stream_write(
                       stream, furi_string_get_cstr(value), furi_string_size(value))) {
                    cycle_error = true;
                    break;
                }
            }
            if(cycle_error) break;

            if(!flipper_format_stream_write_eol(stream)) break;
            result = true;
        } while(false);

        furi_string_free(value);
    }

    return result;
}

bool flipper_format_stream_read_value_line(
    Stream* stream,
    const char* key,
    FlipperStreamValue type,
    void* _data,
    size_t data_size,
    bool strict_mode) {
    bool result = false;

    do {
        if(!flipper_format_stream_seek_to_key(stream, key, strict_mode)) break;

        if(type == FlipperStreamValueStr) {
            FuriString* data = (FuriString*)_data;
            if(flipper_format_stream_read_line(stream, data)) {
                result = true;
                break;
            }
        } else {
            result = true;
            FuriString* value;
            value = furi_string_alloc();

            for(size_t i = 0; i < data_size; i++) {
                bool last = false;
                result = flipper_format_stream_read_value(stream, value, &last);
                if(result) {
                    int scan_values = 0;

                    switch(type) {
                    case FlipperStreamValueHex: {
                        uint8_t* data = _data;
                        if(furi_string_size(value) >= 2) {
                            // sscanf "%02X" does not work here
                            if(hex_char_to_uint8(
                                   furi_string_get_char(value, 0),
                                   furi_string_get_char(value, 1),
                                   &data[i])) {
                                scan_values = 1;
                            }
                        }
                    }; break;
#ifndef FLIPPER_STREAM_LITE
                    case FlipperStreamValueFloat: {
                        float* data = _data;
                        // newlib-nano does not have sscanf for floats
                        // scan_values = sscanf(furi_string_get_cstr(value), "%f", &data[i]);
                        char* end_char;
                        data[i] = strtof(furi_string_get_cstr(value), &end_char);
                        if(*end_char == 0) {
                            // most likely ok
                            scan_values = 1;
                        }
                    }; break;
#endif
                    case FlipperStreamValueInt32: {
                        int32_t* data = _data;
                        if(strint_to_int32(furi_string_get_cstr(value), NULL, &data[i], 10) ==
                           StrintParseNoError) {
                            scan_values = 1;
                        }
                    }; break;
                    case FlipperStreamValueUint32: {
                        uint32_t* data = _data;
                        if(strint_to_uint32(furi_string_get_cstr(value), NULL, &data[i], 10) ==
                           StrintParseNoError) {
                            scan_values = 1;
                        }
                    }; break;
                    case FlipperStreamValueHexUint64: {
                        uint64_t* data = _data;
                        if(furi_string_size(value) >= 16) {
                            if(hex_chars_to_uint64(furi_string_get_cstr(value), &data[i])) {
                                scan_values = 1;
                            }
                        }
                    }; break;
                    case FlipperStreamValueBool: {
                        bool* data = _data;
                        data[i] = !furi_string_cmpi(value, "true");
                        scan_values = 1;
                    }; break;
                    default:
                        furi_crash("Unknown FF type");
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

            furi_string_free(value);
        }
    } while(false);

    return result;
}

bool flipper_format_stream_get_value_count(
    Stream* stream,
    const char* key,
    uint32_t* count,
    bool strict_mode) {
    bool result = false;
    bool last = false;

    FuriString* value;
    value = furi_string_alloc();

    uint32_t position = stream_tell(stream);
    do {
        if(!flipper_format_stream_seek_to_key(stream, key, strict_mode)) break;
        *count = 0;

        result = true;
        while(true) {
            if(!flipper_format_stream_read_value(stream, value, &last)) {
                result = false;
                break;
            }

            *count = *count + 1;
            if(last) break;
        }

    } while(false);

    if(!stream_seek(stream, position, StreamOffsetFromStart)) {
        result = false;
    }

    furi_string_free(value);
    return result;
}

bool flipper_format_stream_delete_key_and_write(
    Stream* stream,
    FlipperStreamWriteData* write_data,
    bool strict_mode) {
    bool result = false;

    do {
        size_t size = stream_size(stream);
        if(size == 0) break;

        if(!stream_rewind(stream)) break;

        // find key
        if(!flipper_format_stream_seek_to_key(stream, write_data->key, strict_mode)) break;

        // get key start position
        size_t start_position = stream_tell(stream) - strlen(write_data->key);
        if(start_position >= 2) {
            start_position -= 2;
        } else {
            // something wrong
            break;
        }

        // get value end position
        if(!flipper_format_stream_seek_to_next_line(stream)) break;
        size_t end_position = stream_tell(stream);
        // newline symbol
        if(end_position < size) {
            end_position += 1;
        }

        if(!stream_seek(stream, start_position, StreamOffsetFromStart)) break;
        if(!stream_delete_and_insert(
               stream,
               end_position - start_position,
               (StreamWriteCB)flipper_format_stream_write_value_line,
               write_data))
            break;

        result = true;
    } while(false);

    return result;
}

bool flipper_format_stream_write_comment_cstr(Stream* stream, const char* data) {
    bool result = false;
    do {
        const char comment_buffer[2] = {flipper_format_comment, ' '};
        result = flipper_format_stream_write(stream, comment_buffer, sizeof(comment_buffer));
        if(!result) break;

        result = flipper_format_stream_write(stream, data, strlen(data));
        if(!result) break;

        result = flipper_format_stream_write_eol(stream);
    } while(false);

    return result;
}
