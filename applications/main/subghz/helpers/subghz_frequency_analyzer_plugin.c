#include "subghz_frequency_analyzer_plugin.h"

#include "../subghz_i.h"
#include "subghz_feature_plugin.h"

#define SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_PATH \
    SUBGHZ_FEATURE_PLUGIN_DIR "subghz_frequency_analyzer.fal"

void subghz_frequency_analyzer_plugin_load(SubGhz* subghz) {
    subghz->freq_analyzer_plugin = subghz_feature_plugin_load(
        subghz,
        &subghz->freq_analyzer_plugin_manager,
        SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_APP_ID,
        SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_PATH,
        "Analyzer plugin\nis missing or\noutdated. Update\nresources.");
    if(subghz->freq_analyzer_plugin) {
        subghz->freq_analyzer_plugin->on_enter(subghz);
    }
}

void subghz_frequency_analyzer_plugin_unload(SubGhz* subghz) {
    if(subghz->freq_analyzer_plugin) {
        // Joins the analyzer's worker thread, which runs plugin code, before we unmap it.
        subghz->freq_analyzer_plugin->on_exit(subghz);
        subghz->freq_analyzer_plugin = NULL;
    }
    subghz_feature_plugin_unload(subghz, &subghz->freq_analyzer_plugin_manager);
}
