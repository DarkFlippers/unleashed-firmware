#pragma once

#include <storage/storage.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ResourceManifestEntryTypeUnknown = 0,
    ResourceManifestEntryTypeVersion,
    ResourceManifestEntryTypeTimestamp,
    ResourceManifestEntryTypeDirectory,
    ResourceManifestEntryTypeFile,
} ResourceManifestEntryType;

typedef struct {
    ResourceManifestEntryType type;
    FuriString* name;
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
 * @brief Rewind manifest to the beginning
 * @param resource_manifest allocated object
 * @return true if successful
 */
bool resource_manifest_rewind(ResourceManifestReader* resource_manifest);

/**
 * @brief Read next file/dir entry from manifest
 * @param resource_manifest allocated object
 * @return entry or NULL if end of file
 */
ResourceManifestEntry* resource_manifest_reader_next(ResourceManifestReader* resource_manifest);

/** Read previous file/dir entry from manifest
 *
 * You must be at the end of the manifest to use this function.
 * Intended to be used after reaching end with resource_manifest_reader_next
 *
 * @param      resource_manifest  Pointer to the ResourceManifestReader instance
 *
 * @return     entry or NULL if end of file
 */
ResourceManifestEntry*
    resource_manifest_reader_previous(ResourceManifestReader* resource_manifest);

#ifdef __cplusplus
} // extern "C"
#endif
