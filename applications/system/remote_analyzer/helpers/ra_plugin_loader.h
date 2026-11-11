/**
 * @file ra_plugin_loader.h
 * @brief Maps and unmaps one of the app's .fal screens.
 *
 * Each screen that is not the Read screen ships as a .fal embedded in the .fap: it is mapped
 * when its scene is entered and unmapped when the scene is left, so its code and its strings
 * are only resident while it is on screen. One is mapped at a time.
 */
#pragma once

#include <furi.h>

/** Where fal_embedded unpacks a plugin, next to the app's assets. */
#define RA_PLUGIN_PATH(name) APP_ASSETS_PATH("plugins/" name ".fal")

typedef struct RaPluginLoader RaPluginLoader;

/**
 * Map the .fal at @p path and hand back its entry point.
 *
 * @param[out] entry_point the plugin's exported struct, NULL on failure
 * @return NULL, having logged, when the file is missing or built against another API version
 */
RaPluginLoader* ra_plugin_loader_load(
    const char* app_id,
    uint32_t api_version,
    const char* path,
    const void** entry_point);

/** Unmap. Safe with NULL. The plugin's own on_exit() must have run first. */
void ra_plugin_loader_unload(RaPluginLoader* loader);
