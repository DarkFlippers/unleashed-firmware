#pragma once

#include <furi.h>
#include <assets_icons.h>

typedef struct {
    const FuriThreadCallback app;
    const char* name;
    const size_t stack_size;
    const IconName icon;
} FlipperApplication;

/* Services list
 * Spawned on startup
 */
extern const FlipperApplication FLIPPER_SERVICES[];
extern const size_t FLIPPER_SERVICES_COUNT;

/* Apps list
 * Spawned by app-loader
 */
extern const FlipperApplication FLIPPER_APPS[];
extern const size_t FLIPPER_APPS_COUNT;

/* Plugins list
 * Spawned by app-loader
 */
extern const FlipperApplication FLIPPER_PLUGINS[];
extern const size_t FLIPPER_PLUGINS_COUNT;

/* Seperate scene app holder
 * Spawned by app-loader
 */
extern const FlipperApplication FLIPPER_SCENES;
extern const FlipperApplication FLIPPER_SCENE_APPS[];
extern const size_t FLIPPER_SCENE_APPS_COUNT;
