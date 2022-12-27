#include <furi.h>
#include <gui/scene_manager.h>
#include <stdint.h>

#include "desktop_settings_scene.h"
#include "../desktop_settings_app.h"
#include "../views/desktop_settings_view_pin_setup_howto2.h"

#define SCENE_EXIT_EVENT (0U)
#define SCENE_DONE_EVENT (1U)

static void desktop_settings_scene_pin_setup_howto2_done_callback(void* context) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_DONE_EVENT);
}

static void desktop_settings_scene_pin_setup_howto2_exit_callback(void* context) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EXIT_EVENT);
}

void desktop_settings_scene_pin_setup_howto2_on_enter(void* context) {
    DesktopSettingsApp* app = context;

    desktop_settings_view_pin_setup_howto2_set_context(app->pin_setup_howto2_view, app);
    desktop_settings_view_pin_setup_howto2_set_ok_callback(
        app->pin_setup_howto2_view, desktop_settings_scene_pin_setup_howto2_done_callback);
    desktop_settings_view_pin_setup_howto2_set_cancel_callback(
        app->pin_setup_howto2_view, desktop_settings_scene_pin_setup_howto2_exit_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinSetupHowto2);
}

bool desktop_settings_scene_pin_setup_howto2_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_DONE_EVENT: {
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetupDone);
            consumed = true;
            break;
        }
        case SCENE_EXIT_EVENT: {
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
            furi_assert(0);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_setup_howto2_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    desktop_settings_view_pin_setup_howto2_set_ok_callback(app->pin_setup_howto2_view, NULL);
    desktop_settings_view_pin_setup_howto2_set_cancel_callback(app->pin_setup_howto2_view, NULL);
}
