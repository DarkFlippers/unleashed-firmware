#pragma once

#include <gui/gui.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

typedef struct {
    uint16_t current_token_index;
} AppSettingsSceneContext;

void totp_scene_app_settings_activate(
    PluginState* plugin_state,
    const AppSettingsSceneContext* context);
void totp_scene_app_settings_render(Canvas* const canvas, const PluginState* plugin_state);
bool totp_scene_app_settings_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state);
void totp_scene_app_settings_deactivate(PluginState* plugin_state);