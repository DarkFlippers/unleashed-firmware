#include <furi.h>

#include "flipper_file.h"
#include "flipper_file_i.h"
#include "flipper_file_helper.h"

static bool flipper_file_write_int32_internal(
    File* file,
    const char* key,
    const void* _data,
    const uint16_t data_size) {
    return flipper_file_write_internal(file, key, _data, data_size, FlipperFileValueInt32);
};

bool flipper_file_read_int32(
    FlipperFile* flipper_file,
    const char* key,
    int32_t* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_read_internal(
        flipper_file->file, key, data, data_size, flipper_file->strict_mode, FlipperFileValueInt32);
}

bool flipper_file_write_int32(
    FlipperFile* flipper_file,
    const char* key,
    const int32_t* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_write_int32_internal(flipper_file->file, key, data, data_size);
}

bool flipper_file_update_int32(
    FlipperFile* flipper_file,
    const char* key,
    const int32_t* data,
    const uint16_t data_size) {
    furi_assert(flipper_file);
    return flipper_file_delete_key_and_call(
        flipper_file, key, flipper_file_write_int32_internal, key, data, data_size);
}
