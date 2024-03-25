/**
 * @file saved_struct.h
 * @brief SavedStruct - data serialization/de-serialization
 * 
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Load data from the file in saved structure format
 *
 * @param[in]  path     The path to the file
 * @param[out] data     Pointer to the memory where to load data
 * @param[in]  size     The size of the data
 * @param[in]  magic    The magic to embed into metadata
 * @param[in]  version  The version to embed into metadata
 *
 * @return     true on success, false otherwise
 */
bool saved_struct_load(const char* path, void* data, size_t size, uint8_t magic, uint8_t version);

/** Save data in saved structure format
 *
 * @param[in]  path     The path to the file
 * @param[in]  data     Pointer to the memory where data
 * @param[in]  size     The size of the data
 * @param[in]  magic    The magic to embed into metadata
 * @param[in]  version  The version to embed into metadata
 *
 * @return     true on success, false otherwise
 */
bool saved_struct_save(
    const char* path,
    const void* data,
    size_t size,
    uint8_t magic,
    uint8_t version);

/** Get SavedStructure file metadata
 *
 * @param[in]  path          The path to the file
 * @param[out] magic         Pointer to store magic or NULL if you don't need it
 * @param[out] version       Pointer to store version or NULL if you don't need
 *                           it
 * @param[out] payload_size  Pointer to store payload size or NULL if you don't
 *                           need it
 *
 * @return     true on success, false otherwise
 */
bool saved_struct_get_metadata(
    const char* path,
    uint8_t* magic,
    uint8_t* version,
    size_t* payload_size);

#ifdef __cplusplus
}
#endif
