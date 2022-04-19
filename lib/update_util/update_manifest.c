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
#define MANIFEST_KEY_ASSETS_FILE "Assets"

UpdateManifest* update_manifest_alloc() {
    UpdateManifest* update_manifest = malloc(sizeof(UpdateManifest));
    string_init(update_manifest->version);
    string_init(update_manifest->firmware_dfu_image);
    string_init(update_manifest->radio_image);
    string_init(update_manifest->staged_loader_file);
    string_init(update_manifest->resource_bundle);
    update_manifest->target = 0;
    update_manifest->valid = false;
    return update_manifest;
}

void update_manifest_free(UpdateManifest* update_manifest) {
    furi_assert(update_manifest);
    string_clear(update_manifest->version);
    string_clear(update_manifest->firmware_dfu_image);
    string_clear(update_manifest->radio_image);
    string_clear(update_manifest->staged_loader_file);
    string_clear(update_manifest->resource_bundle);
    free(update_manifest);
}

static bool
    update_manifest_init_from_ff(UpdateManifest* update_manifest, FlipperFormat* flipper_file) {
    furi_assert(update_manifest);
    furi_assert(flipper_file);

    string_t filetype;
    uint32_t version = 0;

    // TODO: compare filetype?
    string_init(filetype);
    update_manifest->valid =
        flipper_format_read_header(flipper_file, filetype, &version) &&
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
    string_clear(filetype);

    if(update_manifest->valid) {
        /* Optional fields - we can have dfu, radio, or both */
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
            (uint8_t*)&update_manifest->radio_version,
            sizeof(uint32_t));
        flipper_format_read_hex(
            flipper_file,
            MANIFEST_KEY_RADIO_CRC,
            (uint8_t*)&update_manifest->radio_crc,
            sizeof(uint32_t));
        flipper_format_read_string(
            flipper_file, MANIFEST_KEY_ASSETS_FILE, update_manifest->resource_bundle);

        update_manifest->valid =
            (!string_empty_p(update_manifest->firmware_dfu_image) ||
             !string_empty_p(update_manifest->radio_image) ||
             !string_empty_p(update_manifest->resource_bundle));
    }

    return update_manifest->valid;
}

bool update_manifest_init(UpdateManifest* update_manifest, const char* manifest_filename) {
    Storage* storage = furi_record_open("storage");
    FlipperFormat* flipper_file = flipper_format_file_alloc(storage);
    if(flipper_format_file_open_existing(flipper_file, manifest_filename)) {
        update_manifest_init_from_ff(update_manifest, flipper_file);
    }

    flipper_format_free(flipper_file);
    furi_record_close("storage");

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