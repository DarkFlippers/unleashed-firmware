#pragma once

#include <gui/gui.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

void totp_scene_authenticate_activate(PluginState* plugin_state);
void totp_scene_authenticate_render(Canvas* const canvas, PluginState* plugin_state);
bool totp_scene_authenticate_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state);
void totp_scene_authenticate_deactivate(PluginState* plugin_state);
