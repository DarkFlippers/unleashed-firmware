#include "../desktop_settings_app.h"
#include "applications.h"
#include "desktop_settings_scene.h"

enum DesktopSettingsStartSubmenuIndex {
    DesktopSettingsStartSubmenuIndexFavorite,
    DesktopSettingsStartSubmenuIndexPinSetup,
};

static void desktop_settings_scene_start_submenu_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void desktop_settings_scene_start_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Favorite App",
        DesktopSettingsStartSubmenuIndexFavorite,
        desktop_settings_scene_start_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "PIN Setup",
        DesktopSettingsStartSubmenuIndexPinSetup,
        desktop_settings_scene_start_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsStartSubmenuIndexFavorite:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case DesktopSettingsStartSubmenuIndexPinSetup:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinCodeMenu);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_start_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    submenu_reset(app->submenu);
}
