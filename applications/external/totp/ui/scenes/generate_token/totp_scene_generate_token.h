#pragma once

#include <gui/gui.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

typedef struct {
    uint16_t current_token_index;
} GenerateTokenSceneContext;

void totp_scene_generate_token_activate(
    PluginState* plugin_state,
    const GenerateTokenSceneContext* context);
void totp_scene_generate_token_render(Canvas* const canvas, PluginState* plugin_state);
bool totp_scene_generate_token_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state);
void totp_scene_generate_token_deactivate(PluginState* plugin_state);
