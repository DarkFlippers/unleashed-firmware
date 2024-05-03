#include "application_manifest.h"

#include <furi_hal_version.h>
#include <furi.h>

bool flipper_application_manifest_is_valid(const FlipperApplicationManifest* manifest) {
    furi_check(manifest);

    if((manifest->base.manifest_magic != FAP_MANIFEST_MAGIC) ||
       (manifest->base.manifest_version != FAP_MANIFEST_SUPPORTED_VERSION)) {
        return false;
    }

    return true;
}

bool flipper_application_manifest_is_too_old(
    const FlipperApplicationManifest* manifest,
    const ElfApiInterface* api_interface) {
    furi_check(manifest);
    furi_check(api_interface);

    if(manifest->base.api_version.major < api_interface->api_version_major /* ||
       manifest->base.api_version.minor > app->api_interface->api_version_minor */) {
        return false;
    }

    return true;
}

bool flipper_application_manifest_is_too_new(
    const FlipperApplicationManifest* manifest,
    const ElfApiInterface* api_interface) {
    furi_check(manifest);
    furi_check(api_interface);

    if(manifest->base.api_version.major > api_interface->api_version_major /* ||
       manifest->base.api_version.minor > app->api_interface->api_version_minor */) {
        return false;
    }

    return true;
}

bool flipper_application_manifest_is_target_compatible(const FlipperApplicationManifest* manifest) {
    furi_check(manifest);

    const Version* version = furi_hal_version_get_firmware_version();
    return version_get_target(version) == manifest->base.hardware_target_id;
}