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

/** Frequency Analyzer plugin interface. Mirrors the scene handlers the app dispatches to. */
typedef struct {
    /** Allocate the view, register it and switch to it, which starts the worker. */
    void (*on_enter)(SubGhz* subghz);
    /** Handle an analyzer event. Returns true if it was consumed. */
    bool (*on_event)(SubGhz* subghz, SceneManagerEvent event);
    /** Persist the analyzer's settings, unregister the view and free everything. */
    void (*on_exit)(SubGhz* subghz);
} SubGhzFrequencyAnalyzerPlugin;

/** Map the plugin and enter the analyzer. A .fal that will not load is reported by the loader,
 * leaving nothing allocated. */
void subghz_frequency_analyzer_plugin_load(SubGhz* subghz);

/** Tear the analyzer down and unmap the plugin. A no-op if nothing is loaded. */
void subghz_frequency_analyzer_plugin_unload(SubGhz* subghz);

#ifdef __cplusplus
}
#endif
