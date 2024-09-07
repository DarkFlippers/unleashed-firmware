#include <stdint.h>
#include <core/check.h>
#include <gui/scene_manager.h>

#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"
#include <desktop/helpers/pin_code.h>
#include "../desktop_settings_app.h"
#include "../desktop_settings_custom_event.h"

static void pin_error_back_callback(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, DesktopSettingsCustomEventExit);
}

static void pin_error_done_callback(const DesktopPinCode* pin_code, void* context) {
    UNUSED(pin_code);
    furi_assert(context);
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, DesktopSettingsCustomEventExit);
}

void desktop_settings_scene_pin_error_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    desktop_pin_lock_error_notify();

    desktop_view_pin_input_set_context(app->pin_input_view, app);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, pin_error_back_callback);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, pin_error_done_callback);

    uint32_t state =
        scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppScenePinError);
    if(state == SCENE_STATE_PIN_ERROR_MISMATCH) {
        desktop_view_pin_input_set_label_primary(app->pin_input_view, 29, 8, "PIN mismatch!");
    } else if(state == SCENE_STATE_PIN_ERROR_WRONG) {
        desktop_view_pin_input_set_label_primary(app->pin_input_view, 35, 8, "Wrong PIN!");
    } else {
        furi_crash();
    }
    desktop_view_pin_input_set_label_secondary(app->pin_input_view, 0, 8, NULL);
    desktop_view_pin_input_set_label_button(app->pin_input_view, "Retry");
    desktop_view_pin_input_lock_input(app->pin_input_view);
    desktop_view_pin_input_set_pin(app->pin_input_view, &app->pincode_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinInput);
}

bool desktop_settings_scene_pin_error_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventExit:
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

void desktop_settings_scene_pin_error_on_exit(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    desktop_view_pin_input_unlock_input(app->pin_input_view);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, NULL);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, NULL);
}
