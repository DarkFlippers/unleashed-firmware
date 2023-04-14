#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/modules/text_input.h>
#include "../../../types/plugin_state.h"
#include "../../../types/plugin_event.h"

#define INPUT_BUFFER_SIZE (255)

typedef struct {
    char* user_input;
    size_t user_input_length;
    void* callback_data;
} InputTextSceneCallbackResult;

typedef void (*InputTextSceneCallback)(InputTextSceneCallbackResult* result);

typedef struct {
    InputTextSceneCallback callback;
    char* header_text;
    void* callback_data;
} InputTextSceneContext;

typedef struct {
    TextInput* text_input;
    View* text_input_view;
    char text_input_buffer[INPUT_BUFFER_SIZE];
    InputTextSceneCallback callback;
    void* callback_data;
} InputTextSceneState;

InputTextSceneState* totp_input_text_activate(InputTextSceneContext* context);
void totp_input_text_render(Canvas* const canvas, InputTextSceneState* text_input_state);
bool totp_input_text_handle_event(PluginEvent* const event, InputTextSceneState* text_input_state);
void totp_input_text_free(InputTextSceneState* state);
