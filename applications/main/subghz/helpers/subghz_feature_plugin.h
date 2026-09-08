/**
 * @file subghz_feature_plugin.h
 * @brief Shared loader for the Sub-GHz feature plugins - the parts of the app that ship as .fal
 *        files rather than being built into the firmware image.
 */
#pragma once

#include <flipper_application/plugins/plugin_manager.h>
#include <gui/scene_manager.h>
#include <storage/storage.h>

#include "../subghz.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Currently supported plugin API version, shared by every Sub-GHz feature plugin.
 *
 * Bump on any change to what a .fal compiles in from the app's headers and the loader cannot
 * check: the layout of a struct both sides touch, the value of an enum the plugin compiles in, or
 * the signature of an exported symbol - the API hash covers the name only, so a changed signature
 * still resolves and then miscalls. Dropping an export needs no bump; that one fails to resolve.
 */
#define SUBGHZ_FEATURE_PLUGIN_API_VERSION 2

/** Where the feature plugins live - fbt's default folder for this app's plugins, shared with the
 * radio device drivers, whose registry only loads radio_device_*.fal and skips the rest. */
#define SUBGHZ_FEATURE_PLUGIN_DIR EXT_PATH("apps_data/subghz/plugins/")

/**
 * @brief Map a feature plugin, with the loading animation up, and hand back its entry point.
 *
 * On failure it puts @p missing_message on screen through subghz_feature_plugin_handle_missing(),
 * which the caller's scene must call from its on_event - the report is a deferred event, and
 * dropping it leaves the user on a screen with no error and no explanation.
 *
 * @return the plugin's exported interface, or NULL if the .fal could not be loaded - in which case
 *         nothing is left allocated.
 */
const void* subghz_feature_plugin_load(
    SubGhz* subghz,
    PluginManager** manager,
    const char* app_id,
    const char* path,
    const char* missing_message);

/** Unmap a feature plugin. A no-op if nothing is mapped. */
void subghz_feature_plugin_unload(SubGhz* subghz, PluginManager** manager);

/** Turn a failed load into the error screen. True if it consumed the event. */
bool subghz_feature_plugin_handle_missing(SubGhz* subghz, SceneManagerEvent event);

#ifdef __cplusplus
}
#endif
