#include <gui/scene_manager.h>
#include <applications.h>

#include "../desktop_settings_app.h"
#include "../desktop_settings_custom_event.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"

static void
    desktop_settings_scene_quick_apps_menu_submenu_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_quick_apps_menu_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_reset(submenu);

    submenu_add_item(
        submenu,
        "Default Mode",
        DesktopSettingsCustomEventSetDefault,
        desktop_settings_scene_quick_apps_menu_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "Dummy Mode",
        DesktopSettingsCustomEventSetDummy,
        desktop_settings_scene_quick_apps_menu_submenu_callback,
        app);

    submenu_set_header(app->submenu, "Set Quick Access Apps");
    submenu_set_selected_item(app->submenu, app->quick_apps_menu_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_quick_apps_menu_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsCustomEventSetDefault:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneQuickAppsDirectionMenu,
                SCENE_STATE_SET_FAVORITE_APP);
            scene_manager_next_scene(
                app->scene_manager, DesktopSettingsAppSceneQuickAppsDirectionMenu);
            consumed = true;
            break;
        case DesktopSettingsCustomEventSetDummy:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneQuickAppsDirectionMenu,
                SCENE_STATE_SET_DUMMY_APP);
            scene_manager_next_scene(
                app->scene_manager, DesktopSettingsAppSceneQuickAppsDirectionMenu);
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

void desktop_settings_scene_quick_apps_menu_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    app->quick_apps_menu_idx = submenu_get_selected_item(app->submenu);
    submenu_reset(app->submenu);
}
