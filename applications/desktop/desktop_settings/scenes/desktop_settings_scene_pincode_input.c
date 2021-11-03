#include "../desktop_settings_app.h"
#include "desktop/desktop_settings/desktop_settings.h"

#define SCENE_EXIT_EVENT (0U)

void desktop_settings_scene_ok_callback(void* context) {
    DesktopSettingsApp* app = context;
    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppScenePinCodeInput);

    if(state == CodeEventsDisablePin) {
        memset(app->settings.pincode.data, 0, app->settings.pincode.length * sizeof(uint8_t));
        app->settings.pincode.length = 0;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EXIT_EVENT);
}

void desktop_settings_scene_pincode_input_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    CodeInput* code_input = app->code_input;

    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppScenePinCodeInput);
    bool update = state != CodeEventsDisablePin;

    code_input_set_header_text(code_input, "PIN Code Setup");
    code_input_set_result_callback(
        code_input,
        desktop_settings_scene_ok_callback,
        NULL,
        app,
        app->settings.pincode.data,
        &app->settings.pincode.length,
        update);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewPincodeInput);
}

bool desktop_settings_scene_pincode_input_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EXIT_EVENT:
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
            break;

        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pincode_input_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    SAVE_DESKTOP_SETTINGS(&app->settings);
    code_input_set_result_callback(app->code_input, NULL, NULL, NULL, NULL, NULL, 0);
    code_input_set_header_text(app->code_input, "");
}
