#include <applications.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"

#define SCENE_EVENT_SELECT_FAVORITE 0
#define SCENE_EVENT_SELECT_PIN_SETUP 1

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
        SCENE_EVENT_SELECT_FAVORITE,
        desktop_settings_scene_start_submenu_callback,
        app);

    submenu_add_item(
        submenu,
        "PIN Setup",
        SCENE_EVENT_SELECT_PIN_SETUP,
        desktop_settings_scene_start_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_SELECT_FAVORITE:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_PIN_SETUP:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinMenu);
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
