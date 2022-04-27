#pragma once

#include <stdint.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc32_calc_buffer(uint32_t crc, const void* buffer, size_t size);

typedef void (*FileCrcProgressCb)(const uint8_t progress, void* context);

uint32_t crc32_calc_file(File* file, const FileCrcProgressCb progress_cb, void* context);

#ifdef __cplusplus
}
#endif
