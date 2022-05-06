#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include "desktop_settings_scene.h"
#include "../desktop_settings_app.h"
#include "../views/desktop_settings_view_pin_setup_howto.h"

#define SCENE_EXIT_EVENT (0U)

static void desktop_settings_scene_pin_lock_done_callback(void* context) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SCENE_EXIT_EVENT);
}

void desktop_settings_scene_pin_setup_howto_on_enter(void* context) {
    DesktopSettingsApp* app = context;

    desktop_settings_view_pin_setup_howto_set_callback(
        app->pin_setup_howto_view, desktop_settings_scene_pin_lock_done_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewIdPinSetupHowto);
}

bool desktop_settings_scene_pin_setup_howto_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EXIT_EVENT:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetup);
            consumed = true;
            break;
        default:
            furi_assert(0);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_setup_howto_on_exit(void* context) {
    UNUSED(context);
}
