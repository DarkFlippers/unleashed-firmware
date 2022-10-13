#pragma once

#include <gui/gui.h>
#include <furi.h>
#include <furi_hal.h>
#include "../../types/plugin_state.h"
#include "../../types/plugin_event.h"

typedef struct {
    uint8_t current_token_index;
} AppSettingsSceneContext;

void totp_scene_app_settings_init(PluginState* plugin_state);
void totp_scene_app_settings_activate(
    PluginState* plugin_state,
    const AppSettingsSceneContext* context);
void totp_scene_app_settings_render(Canvas* const canvas, PluginState* plugin_state);
bool totp_scene_app_settings_handle_event(PluginEvent* const event, PluginState* plugin_state);
void totp_scene_app_settings_deactivate(PluginState* plugin_state);
void totp_scene_app_settings_free(PluginState* plugin_state);