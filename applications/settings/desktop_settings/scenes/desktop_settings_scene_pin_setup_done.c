#include <furi.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include "../desktop_settings_app.h"
#include "../desktop_settings_custom_event.h"
#include <desktop/desktop_settings.h>
#include <desktop/views/desktop_view_pin_input.h>
#include "desktop_settings_scene.h"

static void pin_setup_done_callback(const DesktopPinCode* pin_code, void* context) {
    furi_assert(pin_code);
    furi_assert(context);
    DesktopSettingsApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, DesktopSettingsCustomEventDone);
}

void desktop_settings_scene_pin_setup_done_on_enter(void* context) {
    DesktopSettingsApp* app = context;

    desktop_pin_code_set(&app->pincode_buffer);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_single_vibro);
    notification_message(notification, &sequence_blink_green_10);
    furi_record_close(RECORD_NOTIFICATION);

    desktop_view_pin_input_set_context(app->pin_input_view, app);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, NULL);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, pin_setup_done_callback);
    desktop_view_pin_input_set_pin(app->pin_input_view, &app->pincode_buffer);
    desktop_view_pin_input_set_label_button(app->pin_input_view, "Done");
    desktop_view_pin_input_set_label_primary(app->pin_input_view, 29, 8, "PIN Activated!");
    desktop_view_pin_input_set_label_secondary(
        app->pin_input_view, 7, 45, "Remember or write it down");
    desktop_view_pin_input_lock_input(app->pin_input_view);
    desktop_view_pin_input_set_pin_position(app->pin_input_view, 64, 24);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinInput);
}

bool desktop_settings_scene_pin_setup_done_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventDone: {
            bool scene_found = false;
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
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }
    return consumed;
}

void desktop_settings_scene_pin_setup_done_on_exit(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    desktop_view_pin_input_set_pin_position(app->pin_input_view, 64, 32);
    desktop_view_pin_input_set_back_callback(app->pin_input_view, NULL);
    desktop_view_pin_input_set_done_callback(app->pin_input_view, NULL);
}
