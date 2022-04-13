#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

/* Paths don't include /ext -- because at startup SD card is mounted as root */
#define UPDATE_DIR_DEFAULT_REL_PATH "/update"
#define UPDATE_MANIFEST_DEFAULT_NAME "update.fuf"
#define UPDATE_MAINFEST_DEFAULT_PATH UPDATE_DIR_DEFAULT_REL_PATH "/" UPDATE_MANIFEST_DEFAULT_NAME

typedef struct {
    string_t version;
    uint32_t target;
    string_t staged_loader_file;
    uint32_t staged_loader_crc;
    string_t firmware_dfu_image;
    string_t radio_image;
    uint32_t radio_address;
    bool valid;
} UpdateManifest;

UpdateManifest* update_manifest_alloc();

void update_manifest_free(UpdateManifest* update_manifest);

bool update_manifest_init(UpdateManifest* update_manifest, const char* manifest_filename);

bool update_manifest_init_mem(
    UpdateManifest* update_manifest,
    const uint8_t* manifest_data,
    const uint16_t length);

#ifdef __cplusplus
}
#endif