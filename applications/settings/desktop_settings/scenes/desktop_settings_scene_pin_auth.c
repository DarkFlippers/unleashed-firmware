#include <stdint.h>
#include <core/check.h>
#include <gui/scene_manager.h>
#include <desktop/helpers/pin_lock.h>
#include "../desktop_settings_app.h"
#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"

#define SCENE_EVENT_EXIT (0U)
#define SCENE_EVENT_PINS_EQUAL (1U)
#define SCENE_EVENT_PINS_DIFFERENT (2U)

static void pin_auth_done_callback(const PinCode* pin_code, void* context) {
    furi_assert(pin_code);
    furi_assert(context);
    DesktopSettingsApp* app = context;

    app->pincode_buffer = *pin_code;
    if(desktop_pins_are_equal(&app->settings.pin_code, pin_code)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_PINS_EQUAL);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_PINS_DIFFERENT);
    }
}

static void pin_auth_back_callback(void* context) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_EXIT);
}

void desktop_settings_scene_pin_auth_on_enter(void* context) {
    DesktopSettingsApp* app = context;

    DESKTOP_SETTINGS_LOAD(&app->settings);
    furi_assert(app->settings.pin_code.length > 0);

    desktop_view_pin_input_set_context(app->pin_input_view, app);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, pin_auth_back_callback);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, pin_auth_done_callback);
    desktop_view_pin_input_set_label_button(app->pin_input_view, "OK");
    desktop_view_pin_input_set_label_primary(app->pin_input_view, 0, 0, NULL);
    desktop_view_pin_input_set_label_secondary(
        app->pin_input_view, 0, 8, "Enter your current PIN:");
    desktop_view_pin_input_reset_pin(app->pin_input_view);
    desktop_view_pin_input_unlock_input(app->pin_input_view);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinInput);
}

bool desktop_settings_scene_pin_auth_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_PINS_DIFFERENT:
            scene_manager_set_scene_state(
                app->scene_manager, DesktopSettingsAppScenePinError, SCENE_STATE_PIN_ERROR_WRONG);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinError);
            consumed = true;
            break;
        case SCENE_EVENT_PINS_EQUAL: {
            uint32_t state =
                scene_manager_get_scene_state(app->scene_manager, DesktopSettingsAppScenePinAuth);
            if(state == SCENE_STATE_PIN_AUTH_CHANGE_PIN) {
                scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetupHowto);
            } else if(state == SCENE_STATE_PIN_AUTH_DISABLE) {
                scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinDisable);
            } else {
                furi_assert(0);
            }
            consumed = true;
            break;
        }
        case SCENE_EVENT_EXIT:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, DesktopSettingsAppScenePinMenu);
            consumed = true;
            break;

        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_auth_on_exit(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    desktop_view_pin_input_set_back_callback(app->pin_input_view, NULL);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, NULL);
}
