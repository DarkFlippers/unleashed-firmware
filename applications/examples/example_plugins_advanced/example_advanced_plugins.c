#include "app_api.h"
#include "plugin_interface.h"
#include "app_api_interface.h"

#include <flipper_application/flipper_application.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <flipper_application/plugins/composite_resolver.h>

#include <loader/firmware_api/firmware_api.h>

#define TAG "ExampleAdvancedPlugins"

int32_t example_advanced_plugins_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting");

    CompositeApiResolver* resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(resolver, firmware_api_interface);
    composite_api_resolver_add(resolver, application_api_interface);

    PluginManager* manager = plugin_manager_alloc(
        PLUGIN_APP_ID, PLUGIN_API_VERSION, composite_api_resolver_get(resolver));

    do {
        // For built-in .fals (fal_embedded==True), use APP_ASSETS_PATH
        // Otherwise, use APP_DATA_PATH
        if(plugin_manager_load_all(manager, APP_ASSETS_PATH("plugins")) !=
           PluginManagerErrorNone) {
            FURI_LOG_E(TAG, "Failed to load all libs");
            break;
        }

        uint32_t plugin_count = plugin_manager_get_count(manager);
        FURI_LOG_I(TAG, "Loaded libs: %lu", plugin_count);

        for(uint32_t i = 0; i < plugin_count; i++) {
            const AdvancedPlugin* plugin = plugin_manager_get_ep(manager, i);
            FURI_LOG_I(TAG, "plugin name: %s. Calling methods", plugin->name);
            plugin->method1(228);
            plugin->method2();
            FURI_LOG_I(TAG, "Accumulator: %lu", app_api_accumulator_get());
        }
    } while(0);

    plugin_manager_free(manager);
    composite_api_resolver_free(resolver);
    FURI_LOG_I(TAG, "Goodbye!");

    return 0;
}
