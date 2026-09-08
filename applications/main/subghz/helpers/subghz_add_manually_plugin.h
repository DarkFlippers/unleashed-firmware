/**
 * @file subghz_add_manually_plugin.h
 * @brief Loads Add Manually, which ships as a .fal rather than being built into the firmware
 *        image. Mapped from the first of its scenes until the start scene is reached again.
 */
#pragma once

#include <gui/scene_manager.h>

#include "../subghz.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Unique string identifier for the Add Manually plugin. */
#define SUBGHZ_ADD_MANUALLY_PLUGIN_APP_ID "SubGhzAddManuallyPlugin"

/** Indexes into the plugin's handler table: shared ABI with the .fal, do not reorder or insert. */
typedef enum {
    SubGhzAddManuallySceneSetType,
    SubGhzAddManuallySceneSetKey,
    SubGhzAddManuallySceneSetSerial,
    SubGhzAddManuallySceneSetButton,
    SubGhzAddManuallySceneSetCounter,
    SubGhzAddManuallySceneSetSeed,
    SubGhzAddManuallySceneCount,
} SubGhzAddManuallyScene;

/** Add Manually plugin interface: the scene handlers the app's shim scenes dispatch to. Typed as
 * the scene manager's own callbacks, which is why the moved scene bodies compile unchanged. */
typedef struct {
    AppSceneOnEnterCallback on_enter;
    AppSceneOnEventCallback on_event;
    AppSceneOnExitCallback on_exit;
} SubGhzAddManuallySceneHandlers;

typedef struct {
    SubGhzAddManuallySceneHandlers scene[SubGhzAddManuallySceneCount];
} SubGhzAddManuallyPlugin;

/** Map the plugin if it is not up yet, then enter @p scene. A .fal that will not load is reported
 * by the loader; @p scene is passed second because the shims compile smaller that way. */
void subghz_add_manually_scene_on_enter(SubGhz* subghz, SubGhzAddManuallyScene scene);

/** Hand an event to @p scene, or turn a failed load into the error screen. */
bool subghz_add_manually_scene_on_event(
    SubGhz* subghz,
    SubGhzAddManuallyScene scene,
    SceneManagerEvent event);

/** Leave @p scene. Never unmaps - see subghz_add_manually_plugin_unload(). */
void subghz_add_manually_scene_on_exit(SubGhz* subghz, SubGhzAddManuallyScene scene);

/**
 * @brief Unmap the plugin and take back what it left in the app's views. A no-op if nothing is
 *        mapped.
 *
 * Belongs to the start scene and to app teardown, never to a scene the plugin implements: a
 * forward transition runs the current scene's on_exit with plugin frames still on the stack, and
 * unmapping there would carry on executing freed memory.
 */
void subghz_add_manually_plugin_unload(SubGhz* subghz);

#ifdef __cplusplus
}
#endif
