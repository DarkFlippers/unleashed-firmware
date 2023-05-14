#include "../desktop_settings_app.h"
#include <desktop/desktop_settings.h>
#include "desktop_settings_scene.h"

enum TextInputIndex {
    TextInputResultOk,
};

static void desktop_settings_scene_change_name_text_input_callback(void* context) {
    DesktopSettingsApp* app = context;

    app->save_name = true;
    view_dispatcher_send_custom_event(app->view_dispatcher, TextInputResultOk);
}

static bool desktop_settings_scene_change_name_validator(
    const char* text,
    FuriString* error,
    void* context) {
    UNUSED(context);

    for(; *text; ++text) {
        const char c = *text;
        if((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
            furi_string_printf(error, "Please only\nenter letters\nand numbers!");
            return false;
        }
    }

    return true;
}

void desktop_settings_scene_change_name_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    TextInput* text_input = app->text_input;

    text_input_set_header_text(text_input, "Leave empty for default");

    text_input_set_validator(text_input, desktop_settings_scene_change_name_validator, NULL);

    text_input_set_minimum_length(text_input, 0);

    text_input_set_result_callback(
        text_input,
        desktop_settings_scene_change_name_text_input_callback,
        app,
        app->device_name,
        FURI_HAL_VERSION_ARRAY_NAME_LENGTH,
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewTextInput);
}

bool desktop_settings_scene_change_name_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case TextInputResultOk:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneNamePopup);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void desktop_settings_scene_change_name_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    text_input_reset(app->text_input);
}