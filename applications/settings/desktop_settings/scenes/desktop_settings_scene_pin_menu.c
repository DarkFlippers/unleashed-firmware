#include <gui/scene_manager.h>
#include <applications.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"
#include "../desktop_settings_custom_event.h"

static void desktop_settings_scene_pin_menu_submenu_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_pin_menu_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_reset(submenu);

    if(!desktop_pin_code_is_set()) {
        submenu_add_item(
            submenu,
            "Set PIN",
            DesktopSettingsCustomEventSetPin,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);

    } else {
        submenu_add_item(
            submenu,
            "Change PIN",
            DesktopSettingsCustomEventChangePin,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Remove PIN",
            DesktopSettingsCustomEventDisablePin,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);
    }

    submenu_set_header(app->submenu, "PIN Code Settings");
    submenu_set_selected_item(app->submenu, app->pin_menu_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_pin_menu_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventSetPin:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetupHowto);
            consumed = true;
            break;
        case DesktopSettingsCustomEventChangePin:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppScenePinAuth,
                SCENE_STATE_PIN_AUTH_CHANGE_PIN);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinAuth);
            consumed = true;
            break;
        case DesktopSettingsCustomEventDisablePin:
            scene_manager_set_scene_state(
                app->scene_manager, DesktopSettingsAppScenePinAuth, SCENE_STATE_PIN_AUTH_DISABLE);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinAuth);
            consumed = true;
            break;
        default:
            consumed = true;
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        submenu_set_selected_item(app->submenu, 0);
    }

    return consumed;
}

void desktop_settings_scene_pin_menu_on_exit(void* context) {
    DesktopSettingsApp* app = context;

    app->pin_menu_idx = submenu_get_selected_item(app->submenu);
    submenu_reset(app->submenu);
}
