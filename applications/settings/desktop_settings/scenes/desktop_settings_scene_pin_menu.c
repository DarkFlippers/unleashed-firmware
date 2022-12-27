#include <gui/scene_manager.h>
#include <applications.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"

#define SCENE_EVENT_SET_PIN 0
#define SCENE_EVENT_CHANGE_PIN 1
#define SCENE_EVENT_DISABLE_PIN 2

static void desktop_settings_scene_pin_menu_submenu_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_pin_menu_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_reset(submenu);

    if(!app->settings.pin_code.length) {
        submenu_add_item(
            submenu,
            "Set Pin",
            SCENE_EVENT_SET_PIN,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);

    } else {
        submenu_add_item(
            submenu,
            "Change Pin",
            SCENE_EVENT_CHANGE_PIN,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Disable",
            SCENE_EVENT_DISABLE_PIN,
            desktop_settings_scene_pin_menu_submenu_callback,
            app);
    }

    submenu_set_header(app->submenu, "Pin code settings:");
    submenu_set_selected_item(app->submenu, app->menu_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_pin_menu_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_SET_PIN:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinSetupHowto);
            consumed = true;
            break;
        case SCENE_EVENT_CHANGE_PIN:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppScenePinAuth,
                SCENE_STATE_PIN_AUTH_CHANGE_PIN);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinAuth);
            consumed = true;
            break;
        case SCENE_EVENT_DISABLE_PIN:
            scene_manager_set_scene_state(
                app->scene_manager, DesktopSettingsAppScenePinAuth, SCENE_STATE_PIN_AUTH_DISABLE);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinAuth);
            consumed = true;
            break;
        default:
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_pin_menu_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    submenu_reset(app->submenu);
}
