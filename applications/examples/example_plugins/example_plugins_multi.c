/**
 * @file example_plugins_multi.c
 * @brief Advanced plugin host application example.
 *
 * It uses PluginManager to load all plugins from a directory
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#include <furi.h>

#define TAG "ExamplePlugins"

int32_t example_plugins_multi_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    PluginManager* manager =
        plugin_manager_alloc(PLUGIN_APP_ID, PLUGIN_API_VERSION, firmware_api_interface);

    if(plugin_manager_load_all(manager, APP_DATA_PATH("plugins")) != PluginManagerErrorNone) {
        FURI_LOG_E(TAG, "Failed to load all libs");
        return 0;
    }

    uint32_t plugin_count = plugin_manager_get_count(manager);
    FURI_LOG_I(TAG, "Loaded %lu plugin(s)", plugin_count);

    for(uint32_t i = 0; i < plugin_count; i++) {
        const ExamplePlugin* plugin = plugin_manager_get_ep(manager, i);
        FURI_LOG_I(TAG, "plugin name: %s", plugin->name);
        FURI_LOG_I(TAG, "plugin method1: %d", plugin->method1());
        FURI_LOG_I(TAG, "plugin method2(7,8): %d", plugin->method2(7, 8));
    }

    plugin_manager_free(manager);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}
