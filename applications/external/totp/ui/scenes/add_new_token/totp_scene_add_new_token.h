#pragma once

#include "../../../config/app/config.h"
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
#include <gui/gui.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

void totp_scene_add_new_token_activate(PluginState* plugin_state);
void totp_scene_add_new_token_render(Canvas* const canvas, const PluginState* plugin_state);
bool totp_scene_add_new_token_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state);
void totp_scene_add_new_token_deactivate(PluginState* plugin_state);
#endif
