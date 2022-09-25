/**
 * @file application_manifest.h
 * Flipper application manifest
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "elf/elf_api_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FAP_MANIFEST_MAGIC 0x52474448
#define FAP_MANIFEST_SUPPORTED_VERSION 1

#define FAP_MANIFEST_MAX_APP_NAME_LENGTH 32
#define FAP_MANIFEST_MAX_ICON_SIZE 32 // TODO: reduce size?

#pragma pack(push, 1)

typedef struct {
    uint32_t manifest_magic;
    uint32_t manifest_version;
    union {
        struct {
            uint16_t minor;
            uint16_t major;
        };
        uint32_t version;
    } api_version;
    uint16_t hardware_target_id;
} FlipperApplicationManifestBase;

typedef struct {
    FlipperApplicationManifestBase base;
    uint16_t stack_size;
    uint32_t app_version;
    char name[FAP_MANIFEST_MAX_APP_NAME_LENGTH];
    char has_icon;
    char icon[FAP_MANIFEST_MAX_ICON_SIZE];
} FlipperApplicationManifestV1;

typedef FlipperApplicationManifestV1 FlipperApplicationManifest;

#pragma pack(pop)

/**
 * @brief Check if manifest is valid
 * 
 * @param manifest 
 * @return bool 
 */
bool flipper_application_manifest_is_valid(const FlipperApplicationManifest* manifest);

/**
 * @brief Check if manifest is compatible with current ELF API interface
 * 
 * @param manifest 
 * @param api_interface 
 * @return bool 
 */
bool flipper_application_manifest_is_compatible(
    const FlipperApplicationManifest* manifest,
    const ElfApiInterface* api_interface);

#ifdef __cplusplus
}
#endif
