#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include <furi_hal_flash.h>

/* Paths don't include /ext -- because at startup SD card is mounted as FS root */
#define UPDATE_MANIFEST_DEFAULT_NAME      "update.fuf"
#define UPDATE_MANIFEST_POINTER_FILE_NAME ".fupdate"

typedef union {
    uint8_t raw[6];
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t sub;
        uint8_t branch;
        uint8_t release;
        uint8_t type;
    } version;
} UpdateManifestRadioVersion;
_Static_assert(sizeof(UpdateManifestRadioVersion) == 6, "UpdateManifestRadioVersion size error");

typedef struct {
    uint32_t manifest_version;
    FuriString* version;
    uint32_t target;
    FuriString* staged_loader_file;
    uint32_t staged_loader_crc;
    FuriString* firmware_dfu_image;
    FuriString* radio_image;
    uint32_t radio_address;
    UpdateManifestRadioVersion radio_version;
    uint32_t radio_crc;
    FuriString* resource_bundle;
    FuriHalFlashRawOptionByteData ob_reference;
    FuriHalFlashRawOptionByteData ob_compare_mask;
    FuriHalFlashRawOptionByteData ob_write_mask;
    FuriString* splash_file;
    bool valid;
} UpdateManifest;

UpdateManifest* update_manifest_alloc(void);

void update_manifest_free(UpdateManifest* update_manifest);

bool update_manifest_init(UpdateManifest* update_manifest, const char* manifest_filename);

bool update_manifest_init_mem(
    UpdateManifest* update_manifest,
    const uint8_t* manifest_data,
    const uint16_t length);

bool update_manifest_has_obdata(UpdateManifest* update_manifest);

#ifdef __cplusplus
}
#endif
