#pragma once

#include <gui/gui.h>
#include <furi.h>
#include <furi_hal.h>
#include "../../types/plugin_state.h"
#include "../../types/plugin_event.h"

typedef struct {
    uint8_t current_token_index;
} GenerateTokenSceneContext;

void totp_scene_generate_token_init(PluginState* plugin_state);
void totp_scene_generate_token_activate(PluginState* plugin_state, const GenerateTokenSceneContext* context);
void totp_scene_generate_token_render(Canvas* const canvas, PluginState* plugin_state);
bool totp_scene_generate_token_handle_event(PluginEvent* const event, PluginState* plugin_state);
void totp_scene_generate_token_deactivate(PluginState* plugin_state);
void totp_scene_generate_token_free(PluginState* plugin_state);
