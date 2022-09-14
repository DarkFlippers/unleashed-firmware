#include <stdint.h>
#include <core/check.h>
#include <gui/scene_manager.h>

#include "../desktop_settings_app.h"
#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"
#include <desktop/helpers/pin_lock.h>

#define SCENE_EVENT_EXIT (0U)
#define SCENE_EVENT_1ST_PIN_ENTERED (1U)
#define SCENE_EVENT_PINS_EQUAL (2U)
#define SCENE_EVENT_PINS_DIFFERENT (3U)

static void pin_setup_done_callback(const PinCode* pin_code, void* context) {
    furi_assert(pin_code);
    furi_assert(context);
    DesktopSettingsApp* app = context;

    if(!app->pincode_buffer_filled) {
        app->pincode_buffer = *pin_code;
        app->pincode_buffer_filled = true;
        view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_1ST_PIN_ENTERED);
    } else {
        app->pincode_buffer_filled = false;
        if(desktop_pins_are_equal(&app->pincode_buffer, pin_code)) {
            view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_PINS_EQUAL);
        } else {
            view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_PINS_DIFFERENT);
        }
    }
}

static void pin_setup_back_callback(void* context) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EVENT_EXIT);
}

void desktop_settings_scene_pin_setup_on_enter(void* context) {
    DesktopSettingsApp* app = context;

    app->pincode_buffer_filled = false;
    desktop_view_pin_input_set_context(app->pin_input_view, app);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, pin_setup_back_callback);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, pin_setup_done_callback);
    desktop_view_pin_input_set_label_button(app->pin_input_view, "OK");
    desktop_view_pin_input_set_label_primary(app->pin_input_view, 0, 0, NULL);
    desktop_view_pin_input_set_label_secondary(
        app->pin_input_view, 0, 8, "Enter from 4 to 10 arrows:");
    desktop_view_pin_input_reset_pin(app->pin_input_view);
    desktop_view_pin_input_unlock_input(app->pin_input_view);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinInput);
}

bool desktop_settings_scene_pin_setup_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_1ST_PIN_ENTERED:
            desktop_view_pin_input_set_label_button(app->pin_input_view, "OK");
            desktop_view_pin_input_set_label_primary(app->pin_input_view, 0, 0, NULL);
            desktop_view_pin_input_set_label_secondary(
                app->pin_input_view, 0, 8, "Confirm your PIN:");
            desktop_view_pin_input_reset_pin(app->pin_input_view);
            desktop_view_pin_input_unlock_input(app->pin_input_view);
            consumed = true;
            break;
        case SCENE_EVENT_PINS_DIFFERENT:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppScenePinError,
                SCENE_STATE_PIN_ERROR_MISMATCH);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinError);
            consumed = true;
            break;
        case SCENE_EVENT_PINS_EQUAL:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetupHowto2);
            consumed = true;
            break;
        case SCENE_EVENT_EXIT: {
            uint32_t scene_found;
            scene_found = scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, DesktopSettingsAppScenePinMenu);
            if(!scene_found) {
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
            break;
        }

        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_setup_on_exit(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    desktop_view_pin_input_set_back_callback(app->pin_input_view, NULL);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, NULL);
}
