/**
 * @file subghz_frequency_analyzer_plugin.h
 * @brief Loads the Frequency Analyzer, which ships as a .fal rather than being built into the
 *        firmware image. Mapped when its scene is entered, dropped when it is left.
 */
#pragma once

#include <gui/scene_manager.h>

#include "../subghz.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Unique string identifier for the Frequency Analyzer plugin. */
#define SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_APP_ID "SubGhzFrequencyAnalyzerPlugin"

/**
 * @brief Currently supported plugin API version.
 *
 * Bump on any change to what the .fal compiles in: @c SubGhz or @c SubGhzLastSettings layout,
 * the signature of an exported symbol (the API hash covers the name only, so a changed signature
 * still resolves and then miscalls), or the values in @c SubGhzCustomEvent / @c SubGhzViewId.
 * Dropping an export needs no bump - the loader fails to resolve it on its own.
 */
#define SUBGHZ_FREQUENCY_ANALYZER_PLUGIN_API_VERSION 1

/** Frequency Analyzer plugin interface. Mirrors the scene handlers the app dispatches to. */
typedef struct {
    /** Allocate the view, register it and switch to it, which starts the worker. */
    void (*on_enter)(SubGhz* subghz);
    /** Handle an analyzer event. Returns true if it was consumed. */
    bool (*on_event)(SubGhz* subghz, SceneManagerEvent event);
    /** Persist the analyzer's settings, unregister the view and free everything. */
    void (*on_exit)(SubGhz* subghz);
} SubGhzFrequencyAnalyzerPlugin;

/** Map the plugin and enter the analyzer. Returns false and logs if the .fal is missing or
 * refuses the API version, leaving nothing allocated. */
bool subghz_frequency_analyzer_plugin_load(SubGhz* subghz);

/** Tear the analyzer down and unmap the plugin. A no-op if nothing is loaded. */
void subghz_frequency_analyzer_plugin_unload(SubGhz* subghz);

#ifdef __cplusplus
}
#endif
