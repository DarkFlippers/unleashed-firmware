/**
 * @file subghz_scene_plugin.h
 * @brief Settings screens that ship as one .fal per scene, mapped on enter and unmapped on exit.
 *
 * They never nest, so one slot serves all of them.
 */
#pragma once

#include <gui/scene_manager.h>
#include <flipper_application/flipper_application.h>

#include "../subghz.h"
#include "subghz_feature_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUBGHZ_SCENE_PLUGIN_APP_ID "SubGhzScenePlugin"

typedef struct {
    AppSceneOnEnterCallback on_enter;
    AppSceneOnEventCallback on_event;
    AppSceneOnExitCallback on_exit;
} SubGhzScenePlugin;

/** Plugin side: the entry point subghz_<name>_ep exporting scene subghz_scene_<name>. The app side
 * (subghz_scene_plugin.c) forwards that scene to subghz_<name>.fal. */
#define SUBGHZ_SCENE_PLUGIN_DEFINE(name)                                          \
    static const SubGhzScenePlugin subghz_##name##_plugin = {                     \
        subghz_scene_##name##_on_enter,                                           \
        subghz_scene_##name##_on_event,                                           \
        subghz_scene_##name##_on_exit,                                            \
    };                                                                            \
    static const FlipperAppPluginDescriptor subghz_##name##_plugin_descriptor = { \
        .appid = SUBGHZ_SCENE_PLUGIN_APP_ID,                                      \
        .ep_api_version = SUBGHZ_FEATURE_PLUGIN_API_VERSION,                      \
        .entry_point = &subghz_##name##_plugin,                                   \
    };                                                                            \
    const FlipperAppPluginDescriptor* subghz_##name##_ep(void) {                  \
        return &subghz_##name##_plugin_descriptor;                                \
    }

#ifdef __cplusplus
}
#endif
