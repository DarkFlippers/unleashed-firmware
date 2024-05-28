#include <gui/scene_manager.h>
#include <applications.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"

enum QuickAppsSubmenuIndex {
    QuickAppsSubmenuIndexFavoriteLeftClick,
    QuickAppsSubmenuIndexFavoriteRightClick,
    QuickAppsSubmenuIndexFavoriteLeftHold,
    QuickAppsSubmenuIndexFavoriteRightHold,
    QuickAppsSubmenuIndexDummyLeftClick,
    QuickAppsSubmenuIndexDummyRightClick,
    QuickAppsSubmenuIndexDummyDownClick,
    QuickAppsSubmenuIndexDummyMiddleClick,
};

static void desktop_settings_scene_quick_apps_direction_menu_submenu_callback(
    void* context,
    uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_quick_apps_direction_menu_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_reset(submenu);

    uint32_t favorite_id = scene_manager_get_scene_state(
        app->scene_manager, DesktopSettingsAppSceneQuickAppsDirectionMenu);

    if(favorite_id == SCENE_STATE_SET_FAVORITE_APP) {
        submenu_add_item(
            submenu,
            "Left - Press",
            QuickAppsSubmenuIndexFavoriteLeftClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Right - Press",
            QuickAppsSubmenuIndexFavoriteRightClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Left - Hold",
            QuickAppsSubmenuIndexFavoriteLeftHold,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Right - Hold",
            QuickAppsSubmenuIndexFavoriteRightHold,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_set_header(app->submenu, "Default Mode");
    } else {
        submenu_add_item(
            submenu,
            "Left - Press",
            QuickAppsSubmenuIndexDummyLeftClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Right - Press",
            QuickAppsSubmenuIndexDummyRightClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Down - Press",
            QuickAppsSubmenuIndexDummyDownClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_add_item(
            submenu,
            "Middle - Press",
            QuickAppsSubmenuIndexDummyMiddleClick,
            desktop_settings_scene_quick_apps_direction_menu_submenu_callback,
            app);

        submenu_set_header(app->submenu, "Dummy Mode");
    }

    submenu_set_selected_item(app->submenu, app->quick_apps_direction_menu_idx);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_quick_apps_direction_menu_on_event(
    void* context,
    SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case QuickAppsSubmenuIndexFavoriteLeftClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppLeftShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexFavoriteRightClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppRightShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexFavoriteLeftHold:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppLeftLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexFavoriteRightHold:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppRightLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexDummyLeftClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppLeft);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexDummyRightClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppRight);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexDummyDownClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppDown);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case QuickAppsSubmenuIndexDummyMiddleClick:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppOk);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
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

void desktop_settings_scene_quick_apps_direction_menu_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    app->quick_apps_direction_menu_idx = submenu_get_selected_item(app->submenu);
    submenu_reset(app->submenu);
}
