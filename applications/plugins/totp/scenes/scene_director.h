#pragma once

#include <gui/gui.h>
#include "../types/plugin_state.h"
#include "../types/plugin_event.h"
#include "totp_scenes_enum.h"

void totp_scene_director_activate_scene(
    PluginState* const plugin_state,
    Scene scene,
    const void* context);
void totp_scene_director_deactivate_active_scene(PluginState* const plugin_state);
void totp_scene_director_init_scenes(PluginState* const plugin_state);
void totp_scene_director_render(Canvas* const canvas, PluginState* const plugin_state);
void totp_scene_director_dispose(PluginState* const plugin_state);
bool totp_scene_director_handle_event(PluginEvent* const event, PluginState* const plugin_state);
