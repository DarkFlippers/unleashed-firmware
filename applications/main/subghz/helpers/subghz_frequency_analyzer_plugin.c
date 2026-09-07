#include "subghz_frequency_analyzer_plugin.h"

#include "../subghz_i.h"
#include "../api/subghz_app_api_interface.h"

#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "SubGhzFrequencyAnalyzer"

/** Not apps_data/subghz/plugins - see the fal_path comment in the plugin's application.fam. */
#define SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_PATH \
    EXT_PATH("apps_data/subghz/plugins/features/subghz_frequency_analyzer.fal")

bool subghz_frequency_analyzer_plugin_load(SubGhz* subghz) {
    furi_assert(!subghz->freq_analyzer_plugin_manager);

    subghz->api_resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(subghz->api_resolver, firmware_api_interface);
    composite_api_resolver_add(subghz->api_resolver, subghz_application_api_interface);

    subghz->freq_analyzer_plugin_manager = plugin_manager_alloc(
        SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_APP_ID,
        SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_API_VERSION,
        composite_api_resolver_get(subghz->api_resolver));

    PluginManagerError error = plugin_manager_load_single(
        subghz->freq_analyzer_plugin_manager, SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_PATH);
    if(error != PluginManagerErrorNone) {
        FURI_LOG_E(
            TAG, "Failed to load %s (error %d)", SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_PATH, error);
        // Leave nothing half-built behind, so failure and success are the same contract.
        subghz_frequency_analyzer_plugin_unload(subghz);
        return false;
    }

    subghz->freq_analyzer_plugin = plugin_manager_get_ep(subghz->freq_analyzer_plugin_manager, 0);
    subghz->freq_analyzer_plugin->on_enter(subghz);
    return true;
}

void subghz_frequency_analyzer_plugin_unload(SubGhz* subghz) {
    if(subghz->freq_analyzer_plugin) {
        // Joins the analyzer's worker thread, which runs plugin code, before we unmap it.
        subghz->freq_analyzer_plugin->on_exit(subghz);
        subghz->freq_analyzer_plugin = NULL;
    }
    if(subghz->freq_analyzer_plugin_manager) {
        plugin_manager_free(subghz->freq_analyzer_plugin_manager);
        subghz->freq_analyzer_plugin_manager = NULL;
    }
    if(subghz->api_resolver) {
        composite_api_resolver_free(subghz->api_resolver);
        subghz->api_resolver = NULL;
    }
}
