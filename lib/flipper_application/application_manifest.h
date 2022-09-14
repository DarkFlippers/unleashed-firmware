#pragma once

#include <stdint.h>

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

#ifdef __cplusplus
}
#endif
