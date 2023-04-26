#pragma once

#include <gui/gui.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

void totp_scene_token_menu_activate(PluginState* plugin_state);
void totp_scene_token_menu_render(Canvas* const canvas, PluginState* plugin_state);
bool totp_scene_token_menu_handle_event(const PluginEvent* const event, PluginState* plugin_state);
void totp_scene_token_menu_deactivate(PluginState* plugin_state);
