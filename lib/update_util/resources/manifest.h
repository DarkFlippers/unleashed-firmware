#pragma once

#include <storage/storage.h>

#include <m-string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ResourceManifestEntryTypeUnknown = 0,
    ResourceManifestEntryTypeDirectory,
    ResourceManifestEntryTypeFile,
} ResourceManifestEntryType;

typedef struct {
    ResourceManifestEntryType type;
    string_t name;
    uint32_t size;
    uint8_t hash[16];
} ResourceManifestEntry;

typedef struct ResourceManifestReader ResourceManifestReader;

/**
 * @brief Initialize resource manifest reader
 * @param storage Storage API pointer
 * @return allocated object
 */
ResourceManifestReader* resource_manifest_reader_alloc(Storage* storage);

/**
 * @brief Release resource manifest reader
 * @param resource_manifest allocated object
 */
void resource_manifest_reader_free(ResourceManifestReader* resource_manifest);

/**
 * @brief Initialize resource manifest reader iteration
 * @param resource_manifest allocated object
 * @param filename manifest file name
 * @return true if file opened
 */
bool resource_manifest_reader_open(ResourceManifestReader* resource_manifest, const char* filename);

/**
 * @brief Read next file/dir entry from manifest
 * @param resource_manifest allocated object
 * @return entry or NULL if end of file
 */
ResourceManifestEntry* resource_manifest_reader_next(ResourceManifestReader* resource_manifest);

#ifdef __cplusplus
} // extern "C"
#endif