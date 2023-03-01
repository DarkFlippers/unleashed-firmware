#pragma once

#include <furi.h>
#include <gui/icon.h>

typedef enum {
    FlipperApplicationFlagDefault = 0,
    FlipperApplicationFlagInsomniaSafe = (1 << 0),
} FlipperApplicationFlag;

typedef struct {
    const FuriThreadCallback app;
    const char* name;
    const char* appid;
    const size_t stack_size;
    const Icon* icon;
    const FlipperApplicationFlag flags;
} FlipperApplication;

typedef void (*FlipperOnStartHook)(void);

extern const char* FLIPPER_AUTORUN_APP_NAME;

/* Services list
 * Spawned on startup
 */
extern const FlipperApplication FLIPPER_SERVICES[];
extern const size_t FLIPPER_SERVICES_COUNT;

/* Apps list
 * Spawned by loader
 */
extern const FlipperApplication FLIPPER_APPS[];
extern const size_t FLIPPER_APPS_COUNT;

/* On system start hooks
 * Called by loader, after OS initialization complete
 */
extern const FlipperOnStartHook FLIPPER_ON_SYSTEM_START[];
extern const size_t FLIPPER_ON_SYSTEM_START_COUNT;

/* Plugins list
 * Spawned by loader
 */
extern const FlipperApplication FLIPPER_PLUGINS[];
extern const size_t FLIPPER_PLUGINS_COUNT;

/* Debug menu apps
 * Spawned by loader
 */
extern const FlipperApplication FLIPPER_DEBUG_APPS[];
extern const size_t FLIPPER_DEBUG_APPS_COUNT;

/* System apps
 * Can only be spawned by loader by name
 */
extern const FlipperApplication FLIPPER_SYSTEM_APPS[];
extern const size_t FLIPPER_SYSTEM_APPS_COUNT;

/* Separate scene app holder
 * Spawned by loader
 */
extern const FlipperApplication FLIPPER_SCENE;
extern const FlipperApplication FLIPPER_SCENE_APPS[];
extern const size_t FLIPPER_SCENE_APPS_COUNT;

extern const FlipperApplication FLIPPER_ARCHIVE;

/* Settings list
 * Spawned by loader
 */
extern const FlipperApplication FLIPPER_SETTINGS_APPS[];
extern const size_t FLIPPER_SETTINGS_APPS_COUNT;
