/**
 * @file example_plugins.c
 * @brief Plugin host application example.
 *
 * Loads a single plugin and calls its methods.
 */

#include "plugin_interface.h"

#include <furi.h>

#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "ExamplePlugins"

int32_t example_plugins_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);

    do {
        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(app, APP_DATA_PATH("plugins/example_plugin1.fal"));

        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to preload plugin");
            break;
        }

        if(!flipper_application_is_plugin(app)) {
            FURI_LOG_E(TAG, "Plugin file is not a library");
            break;
        }

        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(app);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            FURI_LOG_E(TAG, "Failed to load plugin file");
            break;
        }

        const FlipperAppPluginDescriptor* app_descriptor =
            flipper_application_plugin_get_descriptor(app);

        FURI_LOG_I(
            TAG,
            "Loaded plugin for appid '%s', API %lu",
            app_descriptor->appid,
            app_descriptor->ep_api_version);

        furi_check(app_descriptor->ep_api_version == PLUGIN_API_VERSION);
        furi_check(strcmp(app_descriptor->appid, PLUGIN_APP_ID) == 0);

        const ExamplePlugin* plugin = app_descriptor->entry_point;

        FURI_LOG_I(TAG, "Plugin name: %s", plugin->name);
        FURI_LOG_I(TAG, "Plugin method1: %d", plugin->method1());
        FURI_LOG_I(TAG, "Plugin method2(7,8): %d", plugin->method2(7, 8));
        FURI_LOG_I(TAG, "Plugin method2(1337,228): %d", plugin->method2(1337, 228));
    } while(false);
    flipper_application_free(app);

    furi_record_close(RECORD_STORAGE);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}
