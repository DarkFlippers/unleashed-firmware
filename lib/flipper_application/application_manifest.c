#include "application_manifest.h"

bool flipper_application_manifest_is_valid(const FlipperApplicationManifest* manifest) {
    if((manifest->base.manifest_magic != FAP_MANIFEST_MAGIC) ||
       (manifest->base.manifest_version != FAP_MANIFEST_SUPPORTED_VERSION)) {
        return false;
    }

    return true;
}

bool flipper_application_manifest_is_compatible(
    const FlipperApplicationManifest* manifest,
    const ElfApiInterface* api_interface) {
    if(manifest->base.api_version.major != api_interface->api_version_major /* ||
       manifest->base.api_version.minor > app->api_interface->api_version_minor */) {
        return false;
    }

    return true;
}
