#include "update_manifest.h"

#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>

#define MANIFEST_KEY_INFO "Info"
#define MANIFEST_KEY_TARGET "Target"
#define MANIFEST_KEY_LOADER_FILE "Loader"
#define MANIFEST_KEY_LOADER_CRC "Loader CRC"
#define MANIFEST_KEY_DFU_FILE "Firmware"
#define MANIFEST_KEY_RADIO_FILE "Radio"
#define MANIFEST_KEY_RADIO_ADDRESS "Radio address"
#define MANIFEST_KEY_RADIO_VERSION "Radio version"
#define MANIFEST_KEY_RADIO_CRC "Radio CRC"
#define MANIFEST_KEY_ASSETS_FILE "Resources"
#define MANIFEST_KEY_OB_REFERENCE "OB reference"
#define MANIFEST_KEY_OB_MASK "OB mask"
#define MANIFEST_KEY_OB_WRITE_MASK "OB write mask"
#define MANIFEST_KEY_SPLASH_FILE "Splashscreen"

UpdateManifest* update_manifest_alloc() {
    UpdateManifest* update_manifest = malloc(sizeof(UpdateManifest));
    update_manifest->version = furi_string_alloc();
    update_manifest->firmware_dfu_image = furi_string_alloc();
    update_manifest->radio_image = furi_string_alloc();
    update_manifest->staged_loader_file = furi_string_alloc();
    update_manifest->resource_bundle = furi_string_alloc();
    update_manifest->splash_file = furi_string_alloc();
    update_manifest->target = 0;
    update_manifest->manifest_version = 0;
    memset(update_manifest->ob_reference.bytes, 0, FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);
    memset(update_manifest->ob_compare_mask.bytes, 0, FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);
    memset(update_manifest->ob_write_mask.bytes, 0, FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);
    update_manifest->valid = false;
    return update_manifest;
}

void update_manifest_free(UpdateManifest* update_manifest) {
    furi_assert(update_manifest);
    furi_string_free(update_manifest->version);
    furi_string_free(update_manifest->firmware_dfu_image);
    furi_string_free(update_manifest->radio_image);
    furi_string_free(update_manifest->staged_loader_file);
    furi_string_free(update_manifest->resource_bundle);
    furi_string_free(update_manifest->splash_file);
    free(update_manifest);
}

static bool
    update_manifest_init_from_ff(UpdateManifest* update_manifest, FlipperFormat* flipper_file) {
    furi_assert(update_manifest);
    furi_assert(flipper_file);

    FuriString* filetype;

    // TODO FL-3543: compare filetype?
    filetype = furi_string_alloc();
    update_manifest->valid =
        flipper_format_read_header(flipper_file, filetype, &update_manifest->manifest_version) &&
        flipper_format_read_string(flipper_file, MANIFEST_KEY_INFO, update_manifest->version) &&
        flipper_format_read_uint32(
            flipper_file, MANIFEST_KEY_TARGET, &update_manifest->target, 1) &&
        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_LOADER_FILE, update_manifest->staged_loader_file) &&
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_LOADER_CRC,
            (uint8_t*)&update_manifest->staged_loader_crc,
            sizeof(uint32_t));
    furi_string_free(filetype);

    if(update_manifest->valid) {
        /* Optional fields - we can have dfu, radio, resources, or any combination */
        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_DFU_FILE, update_manifest->firmware_dfu_image);
        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_RADIO_FILE, update_manifest->radio_image);
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_RADIO_ADDRESS,
            (uint8_t*)&update_manifest->radio_address,
            sizeof(uint32_t));
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_RADIO_VERSION,
            update_manifest->radio_version.raw,
            sizeof(UpdateManifestRadioVersion));
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_RADIO_CRC,
            (uint8_t*)&update_manifest->radio_crc,
            sizeof(uint32_t));
        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_ASSETS_FILE, update_manifest->resource_bundle);

        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_OB_REFERENCE,
            update_manifest->ob_reference.bytes,
            FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_OB_MASK,
            update_manifest->ob_compare_mask.bytes,
            FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_OB_WRITE_MASK,
            update_manifest->ob_write_mask.bytes,
            FURI_HAL_FLASH_OB_RAW_SIZE_BYTES);

        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_SPLASH_FILE, update_manifest->splash_file);

        update_manifest->valid =
            (!furi_string_empty(update_manifest->firmware_dfu_image) ||
             !furi_string_empty(update_manifest->radio_image) ||
             !furi_string_empty(update_manifest->resource_bundle));
    }

    return update_manifest->valid;
}

// Verifies that mask values are same for adjacent words (value & inverted)
static bool ob_data_check_mask_valid(const FuriHalFlashRawOptionByteData* mask) {
    bool mask_valid = true;
    for(size_t idx = 0; mask_valid && (idx < FURI_HAL_FLASH_OB_TOTAL_VALUES); ++idx) {
        mask_valid &= mask->obs[idx].values.base == mask->obs[idx].values.complementary_value;
    }
    return mask_valid;
}

// Verifies that all reference values have no unmasked bits
static bool ob_data_check_masked_values_valid(
    const FuriHalFlashRawOptionByteData* data,
    const FuriHalFlashRawOptionByteData* mask) {
    bool valid = true;
    for(size_t idx = 0; valid && (idx < FURI_HAL_FLASH_OB_TOTAL_VALUES); ++idx) {
        valid &= (data->obs[idx].dword & mask->obs[idx].dword) == data->obs[idx].dword;
    }
    return valid;
}

bool update_manifest_has_obdata(UpdateManifest* update_manifest) {
    bool ob_data_valid = false;
    // do we have at least 1 value?
    for(size_t idx = 0; !ob_data_valid && (idx < FURI_HAL_FLASH_OB_RAW_SIZE_BYTES); ++idx) {
        ob_data_valid |= update_manifest->ob_reference.bytes[idx] != 0;
    }
    // sanity checks
    ob_data_valid &= ob_data_check_mask_valid(&update_manifest->ob_write_mask);
    ob_data_valid &= ob_data_check_mask_valid(&update_manifest->ob_compare_mask);
    ob_data_valid &= ob_data_check_masked_values_valid(
        &update_manifest->ob_reference, &update_manifest->ob_compare_mask);
    return ob_data_valid;
}

bool update_manifest_init(UpdateManifest* update_manifest, const char* manifest_filename) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* flipper_file = flipper_format_file_alloc(storage);
    if(flipper_format_file_open_existing(flipper_file, manifest_filename)) {
        update_manifest_init_from_ff(update_manifest, flipper_file);
    }

    flipper_format_free(flipper_file);
    furi_record_close(RECORD_STORAGE);

    return update_manifest->valid;
}

bool update_manifest_init_mem(
    UpdateManifest* update_manifest,
    const uint8_t* manifest_data,
    const uint16_t length) {
    FlipperFormat* flipper_file = flipper_format_string_alloc();
    Stream* sstream = flipper_format_get_raw_stream(flipper_file);

    stream_write(sstream, manifest_data, length);
    stream_seek(sstream, 0, StreamOffsetFromStart);

    update_manifest_init_from_ff(update_manifest, flipper_file);

    flipper_format_free(flipper_file);

    return update_manifest->valid;
}
