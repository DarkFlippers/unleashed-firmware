#include "ra_plugin_loader.h"

#include <storage/storage.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#define TAG "RaPluginLoader"

struct RaPluginLoader {
    PluginManager* manager;
};

RaPluginLoader* ra_plugin_loader_load(
    const char* app_id,
    uint32_t api_version,
    const char* path,
    const void** entry_point) {
    furi_assert(entry_point);
    *entry_point = NULL;

    RaPluginLoader* loader = malloc(sizeof(RaPluginLoader));
    loader->manager = plugin_manager_alloc(app_id, api_version, firmware_api_interface);

    PluginManagerError error = plugin_manager_load_single(loader->manager, path);
    if(error != PluginManagerErrorNone) {
        FURI_LOG_E(TAG, "Failed to load %s: %d", path, error);
        plugin_manager_free(loader->manager);
        free(loader);
        return NULL;
    }

    *entry_point = plugin_manager_get_ep(loader->manager, 0);
    return loader;
}

void ra_plugin_loader_unload(RaPluginLoader* loader) {
    if(!loader) return;
    plugin_manager_free(loader->manager);
    free(loader);
}
