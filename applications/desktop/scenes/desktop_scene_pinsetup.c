#include "../desktop_i.h"

#define SCENE_EXIT_EVENT (0U)

void desktop_scene_ok_callback(void* context) {
    Desktop* app = context;
    desktop_settings_save(&app->settings);
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EXIT_EVENT);
}

void desktop_scene_pinsetup_on_enter(void* context) {
    Desktop* app = context;
    CodeInput* code_input = app->code_input;

    code_input_set_result_callback(
        code_input,
        desktop_scene_ok_callback,
        NULL,
        app,
        app->settings.pincode.data,
        &app->settings.pincode.length,
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopViewPinSetup);
}

bool desktop_scene_pinsetup_on_event(void* context, SceneManagerEvent event) {
    Desktop* app = context;
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

void desktop_scene_pinsetup_on_exit(void* context) {
    Desktop* app = context;
    code_input_set_result_callback(app->code_input, NULL, NULL, NULL, NULL, NULL, 0);
    code_input_set_header_text(app->code_input, "");
}
