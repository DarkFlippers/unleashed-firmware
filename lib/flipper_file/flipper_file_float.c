#include <furi.h>

#include "flipper_file.h"
#include "flipper_file_i.h"
#include "flipper_file_helper.h"

static bool flipper_file_write_float_internal(
    File* file,
    const char* key,
    const void* _data,
    const uint16_t data_size) {
    return flipper_file_write_internal(file, key, _data, data_size, FlipperFileValueFloat);
};

bool flipper_file_read_float(
    FlipperFile* flipper_file,
    const char* key,
    float* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_read_internal(
        flipper_file->file, key, data, data_size, FlipperFileValueFloat);
}

bool flipper_file_write_float(
    FlipperFile* flipper_file,
    const char* key,
    const float* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_write_float_internal(flipper_file->file, key, data, data_size);
}

bool flipper_file_update_float(
    FlipperFile* flipper_file,
    const char* key,
    const float* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_delete_key_and_call(
        flipper_file, key, flipper_file_write_float_internal, key, data, data_size);
}