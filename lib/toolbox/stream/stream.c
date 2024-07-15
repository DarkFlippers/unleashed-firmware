#include "stream.h"
#include "stream_i.h"
#include "file_stream.h"
#include <core/check.h>
#include <core/common_defines.h>

#define STREAM_BUFFER_SIZE (32U)

void stream_free(Stream* stream) {
    furi_check(stream);
    stream->vtable->free(stream);
}

void stream_clean(Stream* stream) {
    furi_check(stream);
    stream->vtable->clean(stream);
}

bool stream_eof(Stream* stream) {
    furi_check(stream);
    return stream->vtable->eof(stream);
}

bool stream_seek(Stream* stream, int32_t offset, StreamOffset offset_type) {
    furi_check(stream);
    return stream->vtable->seek(stream, offset, offset_type);
}

static bool stream_seek_to_char_forward(Stream* stream, char c) {
    // Search is starting from seconds character
    if(!stream_seek(stream, 1, StreamOffsetFromCurrent)) {
        return false;
    }

    // Search character in a stream
    bool result = false;
    while(!result) {
        uint8_t buffer[STREAM_BUFFER_SIZE] = {0};
        size_t ret = stream_read(stream, buffer, STREAM_BUFFER_SIZE);
        for(size_t i = 0; i < ret; i++) {
            if(buffer[i] == c) {
                stream_seek(stream, (int32_t)i - ret, StreamOffsetFromCurrent);
                result = true;
                break;
            }
        }
        if(ret != STREAM_BUFFER_SIZE) break;
    }
    return result;
}

static bool stream_seek_to_char_backward(Stream* stream, char c) {
    size_t anchor = stream_tell(stream);

    // Special case, no previous characters
    if(anchor == 0) {
        return false;
    }

    bool result = false;
    while(!result) {
        // Seek back
        uint8_t buffer[STREAM_BUFFER_SIZE] = {0};
        size_t to_read = STREAM_BUFFER_SIZE;
        if(to_read > anchor) {
            to_read = anchor;
        }

        anchor -= to_read;
        furi_check(stream_seek(stream, anchor, StreamOffsetFromStart));

        size_t ret = stream_read(stream, buffer, to_read);
        for(size_t i = 0; i < ret; i++) {
            size_t cursor = ret - i - 1;
            if(buffer[cursor] == c) {
                result = true;
                furi_check(stream_seek(stream, anchor + cursor, StreamOffsetFromStart));
                break;
            } else {
            }
        }
        if(ret != STREAM_BUFFER_SIZE) break;
    }
    return result;
}

bool stream_seek_to_char(Stream* stream, char c, StreamDirection direction) {
    furi_check(stream);

    const size_t old_position = stream_tell(stream);

    bool result = false;
    if(direction == StreamDirectionForward) {
        result = stream_seek_to_char_forward(stream, c);
    } else if(direction == StreamDirectionBackward) {
        result = stream_seek_to_char_backward(stream, c);
    }

    // Rollback
    if(!result) {
        stream_seek(stream, old_position, StreamOffsetFromStart);
    }

    return result;
}

size_t stream_tell(Stream* stream) {
    furi_check(stream);
    return stream->vtable->tell(stream);
}

size_t stream_size(Stream* stream) {
    furi_check(stream);
    return stream->vtable->size(stream);
}

size_t stream_write(Stream* stream, const uint8_t* data, size_t size) {
    furi_check(stream);
    return stream->vtable->write(stream, data, size);
}

size_t stream_read(Stream* stream, uint8_t* data, size_t size) {
    furi_check(stream);
    return stream->vtable->read(stream, data, size);
}

bool stream_delete_and_insert(
    Stream* stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* ctx) {
    furi_check(stream);
    return stream->vtable->delete_and_insert(stream, delete_size, write_callback, ctx);
}

/********************************** Some random helpers starts here **********************************/

typedef struct {
    const uint8_t* data;
    size_t size;
} StreamWriteData;

static bool stream_write_struct(Stream* stream, const void* context) {
    furi_check(stream);
    furi_check(context);
    const StreamWriteData* write_data = context;
    return stream_write(stream, write_data->data, write_data->size) == write_data->size;
}

bool stream_read_line(Stream* stream, FuriString* str_result) {
    furi_check(stream);
    furi_check(str_result);

    furi_string_reset(str_result);
    uint8_t buffer[STREAM_BUFFER_SIZE];

    do {
        uint16_t bytes_were_read = stream_read(stream, buffer, STREAM_BUFFER_SIZE);
        if(bytes_were_read == 0) break;

        bool result = false;
        bool error = false;
        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == '\n') {
                if(!stream_seek(stream, i - bytes_were_read + 1, StreamOffsetFromCurrent)) {
                    error = true;
                    break;
                }
                furi_string_push_back(str_result, buffer[i]);
                result = true;
                break;
            } else if(buffer[i] == '\r') {
                // Ignore
            } else {
                furi_string_push_back(str_result, buffer[i]);
            }
        }

        if(result || error) {
            break;
        }
    } while(true);

    return furi_string_size(str_result) != 0;
}

bool stream_rewind(Stream* stream) {
    furi_check(stream);
    return stream_seek(stream, 0, StreamOffsetFromStart);
}

size_t stream_write_char(Stream* stream, char c) {
    furi_check(stream);
    return stream_write(stream, (const uint8_t*)&c, 1);
}

size_t stream_write_string(Stream* stream, FuriString* string) {
    furi_check(stream);
    return stream_write(
        stream, (const uint8_t*)furi_string_get_cstr(string), furi_string_size(string));
}

size_t stream_write_cstring(Stream* stream, const char* string) {
    furi_check(stream);
    return stream_write(stream, (const uint8_t*)string, strlen(string));
}

size_t stream_write_format(Stream* stream, const char* format, ...) {
    furi_check(stream);
    size_t size;
    va_list args;
    va_start(args, format);
    size = stream_write_vaformat(stream, format, args);
    va_end(args);
    return size;
}

size_t stream_write_vaformat(Stream* stream, const char* format, va_list args) {
    furi_check(stream);
    FuriString* data;
    data = furi_string_alloc_vprintf(format, args);
    size_t size = stream_write_string(stream, data);
    furi_string_free(data);

    return size;
}

bool stream_insert(Stream* stream, const uint8_t* data, size_t size) {
    furi_check(stream);
    StreamWriteData write_data = {.data = data, .size = size};
    return stream_delete_and_insert(stream, 0, stream_write_struct, &write_data);
}

bool stream_insert_char(Stream* stream, char c) {
    furi_check(stream);
    return stream_delete_and_insert_char(stream, 0, c);
}

bool stream_insert_string(Stream* stream, FuriString* string) {
    furi_check(stream);
    return stream_delete_and_insert_string(stream, 0, string);
}

bool stream_insert_cstring(Stream* stream, const char* string) {
    furi_check(stream);
    return stream_delete_and_insert_cstring(stream, 0, string);
}

bool stream_insert_format(Stream* stream, const char* format, ...) {
    furi_check(stream);
    va_list args;
    va_start(args, format);
    bool result = stream_insert_vaformat(stream, format, args);
    va_end(args);

    return result;
}

bool stream_insert_vaformat(Stream* stream, const char* format, va_list args) {
    furi_check(stream);
    return stream_delete_and_insert_vaformat(stream, 0, format, args);
}

bool stream_delete_and_insert_char(Stream* stream, size_t delete_size, char c) {
    furi_check(stream);
    StreamWriteData write_data = {.data = (uint8_t*)&c, .size = 1};
    return stream_delete_and_insert(stream, delete_size, stream_write_struct, &write_data);
}

bool stream_delete_and_insert_string(Stream* stream, size_t delete_size, FuriString* string) {
    furi_check(stream);
    StreamWriteData write_data = {
        .data = (uint8_t*)furi_string_get_cstr(string), .size = furi_string_size(string)};
    return stream_delete_and_insert(stream, delete_size, stream_write_struct, &write_data);
}

bool stream_delete_and_insert_cstring(Stream* stream, size_t delete_size, const char* string) {
    furi_check(stream);
    StreamWriteData write_data = {.data = (uint8_t*)string, .size = strlen(string)};
    return stream_delete_and_insert(stream, delete_size, stream_write_struct, &write_data);
}

bool stream_delete_and_insert_format(Stream* stream, size_t delete_size, const char* format, ...) {
    furi_check(stream);
    va_list args;
    va_start(args, format);
    bool result = stream_delete_and_insert_vaformat(stream, delete_size, format, args);
    va_end(args);

    return result;
}

bool stream_delete_and_insert_vaformat(
    Stream* stream,
    size_t delete_size,
    const char* format,
    va_list args) {
    furi_check(stream);

    FuriString* data = furi_string_alloc_vprintf(format, args);
    StreamWriteData write_data = {
        .data = (uint8_t*)furi_string_get_cstr(data), .size = furi_string_size(data)};
    bool result = stream_delete_and_insert(stream, delete_size, stream_write_struct, &write_data);
    furi_string_free(data);

    return result;
}

bool stream_delete(Stream* stream, size_t size) {
    furi_check(stream);
    return stream_delete_and_insert(stream, size, NULL, NULL);
}

size_t stream_copy(Stream* stream_from, Stream* stream_to, size_t size) {
    furi_check(stream_from);
    furi_check(stream_to);

    uint8_t* buffer = malloc(STREAM_CACHE_SIZE);
    size_t copied = 0;

    do {
        size_t bytes_count = MIN(STREAM_CACHE_SIZE, size - copied);
        if(bytes_count <= 0) {
            break;
        }

        uint16_t bytes_were_read = stream_read(stream_from, buffer, bytes_count);
        if(bytes_were_read != bytes_count) break;

        uint16_t bytes_were_written = stream_write(stream_to, buffer, bytes_count);
        if(bytes_were_written != bytes_count) break;

        copied += bytes_count;
    } while(true);

    free(buffer);
    return copied;
}

size_t stream_copy_full(Stream* stream_from, Stream* stream_to) {
    furi_check(stream_from);
    furi_check(stream_to);

    size_t was_written = 0;

    do {
        if(!stream_seek(stream_from, 0, StreamOffsetFromStart)) break;
        if(!stream_seek(stream_to, 0, StreamOffsetFromStart)) break;
        was_written = stream_copy(stream_from, stream_to, stream_size(stream_from));
    } while(false);

    return was_written;
}

bool stream_split(Stream* stream, Stream* stream_left, Stream* stream_right) {
    furi_check(stream);

    bool result = false;
    size_t size = stream_size(stream);
    size_t tell = stream_tell(stream);

    do {
        // copy right
        if(stream_copy(stream, stream_right, size - tell) != (size - tell)) break;

        // copy left
        if(!stream_rewind(stream)) break;
        if(stream_copy(stream, stream_left, tell) != tell) break;

        // restore RW pointer
        if(!stream_seek(stream, tell, StreamOffsetFromStart)) break;
        result = true;
    } while(false);

    return result;
}

size_t stream_load_from_file(Stream* stream, Storage* storage, const char* path) {
    furi_check(stream);
    furi_check(storage);

    size_t was_written = 0;
    Stream* file = file_stream_alloc(storage);

    do {
        if(!file_stream_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) break;
        was_written = stream_copy(file, stream, stream_size(file));
    } while(false);

    stream_free(file);
    return was_written;
}

size_t stream_save_to_file(Stream* stream, Storage* storage, const char* path, FS_OpenMode mode) {
    furi_check(stream);
    furi_check(storage);

    size_t was_written = 0;
    Stream* file = file_stream_alloc(storage);

    do {
        if(!file_stream_open(file, path, FSAM_WRITE, mode)) break;
        was_written = stream_copy(stream, file, stream_size(stream));
    } while(false);

    stream_free(file);
    return was_written;
}

void stream_dump_data(Stream* stream) {
    furi_check(stream);

    size_t size = stream_size(stream);
    size_t tell = stream_tell(stream);
    printf("stream %p\r\n", stream);
    printf("size = %zu\r\n", size);
    printf("tell = %zu\r\n", tell);
    printf("DATA START\r\n");
    uint8_t* data = malloc(STREAM_CACHE_SIZE);
    stream_rewind(stream);

    while(true) {
        size_t was_read = stream_read(stream, data, STREAM_CACHE_SIZE);
        if(was_read == 0) break;

        for(size_t i = 0; i < was_read; i++) {
            printf("%c", data[i]);
        }
    }

    free(data);
    printf("\r\n");
    printf("DATA END\r\n");
    stream_seek(stream, tell, StreamOffsetFromStart);
}
