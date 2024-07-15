#include "lfrfid_raw_file.h"
#include "tools/varint_pair.h"
#include <toolbox/stream/file_stream.h>
#include <toolbox/varint.h>

#define LFRFID_RAW_FILE_MAGIC   0x4C464952
#define LFRFID_RAW_FILE_VERSION 1

#define TAG "LfRfidRawFile"

typedef struct {
    uint32_t magic;
    uint32_t version;
    float frequency;
    float duty_cycle;
    uint32_t max_buffer_size;
} LFRFIDRawFileHeader;

struct LFRFIDRawFile {
    Stream* stream;
    uint32_t max_buffer_size;

    uint8_t* buffer;
    uint32_t buffer_size;
    size_t buffer_counter;
};

LFRFIDRawFile* lfrfid_raw_file_alloc(Storage* storage) {
    furi_check(storage);

    LFRFIDRawFile* file = malloc(sizeof(LFRFIDRawFile));
    file->stream = file_stream_alloc(storage);
    file->buffer = NULL;
    return file;
}

void lfrfid_raw_file_free(LFRFIDRawFile* file) {
    furi_check(file);

    if(file->buffer) free(file->buffer);
    stream_free(file->stream);
    free(file);
}

bool lfrfid_raw_file_open_write(LFRFIDRawFile* file, const char* file_path) {
    furi_check(file);
    furi_check(file_path);

    return file_stream_open(file->stream, file_path, FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
}

bool lfrfid_raw_file_open_read(LFRFIDRawFile* file, const char* file_path) {
    furi_check(file);
    furi_check(file_path);

    return file_stream_open(file->stream, file_path, FSAM_READ, FSOM_OPEN_EXISTING);
}

bool lfrfid_raw_file_write_header(
    LFRFIDRawFile* file,
    float frequency,
    float duty_cycle,
    uint32_t max_buffer_size) {
    furi_check(file);

    LFRFIDRawFileHeader header = {
        .magic = LFRFID_RAW_FILE_MAGIC,
        .version = LFRFID_RAW_FILE_VERSION,
        .frequency = frequency,
        .duty_cycle = duty_cycle,
        .max_buffer_size = max_buffer_size};

    size_t size = stream_write(file->stream, (uint8_t*)&header, sizeof(LFRFIDRawFileHeader));
    return size == sizeof(LFRFIDRawFileHeader);
}

bool lfrfid_raw_file_write_buffer(LFRFIDRawFile* file, uint8_t* buffer_data, size_t buffer_size) {
    furi_check(file);
    furi_check(buffer_data);
    furi_check(buffer_size);

    size_t size;
    size = stream_write(file->stream, (uint8_t*)&buffer_size, sizeof(size_t));
    if(size != sizeof(size_t)) return false;

    size = stream_write(file->stream, buffer_data, buffer_size);
    if(size != buffer_size) return false;

    return true;
}

bool lfrfid_raw_file_read_header(LFRFIDRawFile* file, float* frequency, float* duty_cycle) {
    furi_check(file);
    furi_check(frequency);
    furi_check(duty_cycle);

    LFRFIDRawFileHeader header;
    size_t size = stream_read(file->stream, (uint8_t*)&header, sizeof(LFRFIDRawFileHeader));
    if(size == sizeof(LFRFIDRawFileHeader)) {
        if(header.magic == LFRFID_RAW_FILE_MAGIC && header.version == LFRFID_RAW_FILE_VERSION) {
            *frequency = header.frequency;
            *duty_cycle = header.duty_cycle;
            file->max_buffer_size = header.max_buffer_size;
            file->buffer = malloc(file->max_buffer_size);
            file->buffer_size = 0;
            file->buffer_counter = 0;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool lfrfid_raw_file_read_pair(
    LFRFIDRawFile* file,
    uint32_t* duration,
    uint32_t* pulse,
    bool* pass_end) {
    furi_check(file);
    furi_check(duration);
    furi_check(pulse);

    size_t length = 0;
    if(file->buffer_counter >= file->buffer_size) {
        if(stream_eof(file->stream)) {
            // rewind stream and pass header
            stream_seek(file->stream, sizeof(LFRFIDRawFileHeader), StreamOffsetFromStart);
            if(pass_end) *pass_end = true;
        }

        length = stream_read(file->stream, (uint8_t*)&file->buffer_size, sizeof(size_t));
        if(length != sizeof(size_t)) {
            FURI_LOG_E(TAG, "read pair: failed to read size");
            return false;
        }

        if(file->buffer_size > file->max_buffer_size) {
            FURI_LOG_E(TAG, "read pair: buffer size is too big");
            return false;
        }

        length = stream_read(file->stream, file->buffer, file->buffer_size);
        if(length != file->buffer_size) {
            FURI_LOG_E(TAG, "read pair: failed to read data");
            return false;
        }

        file->buffer_counter = 0;
    }

    size_t size = 0;
    bool result = varint_pair_unpack(
        &file->buffer[file->buffer_counter],
        (size_t)(file->buffer_size - file->buffer_counter),
        pulse,
        duration,
        &size);

    if(result) {
        file->buffer_counter += size;
    } else {
        FURI_LOG_E(TAG, "read pair: buffer is too small");
        return false;
    }

    return true;
}
