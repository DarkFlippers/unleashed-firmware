#include "../bt_settings_app.h"
#include "furi-hal-bt.h"

enum BtSettingsAppStartSubmenuIndex {
    BtSettingsAppStartSubmenuIndexEnable,
};

static void bt_settings_scene_start_submenu_callback(void* context, uint32_t index) {
    BtSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void bt_settings_scene_start_on_enter(void* context) {
    BtSettingsApp* app = context;
    Submenu* submenu = app->submenu;

    const char* submenu_label = app->settings.enabled ? "Disable" : "Enable";
    submenu_add_item(
        submenu,
        submenu_label,
        BtSettingsAppStartSubmenuIndexEnable,
        bt_settings_scene_start_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewSubmenu);
}

bool bt_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BtSettingsAppStartSubmenuIndexEnable) {
            if(!app->settings.enabled) {
                app->settings.enabled = true;
                furi_hal_bt_start_app();
                submenu_clean(app->submenu);
                submenu_add_item(
                    app->submenu,
                    "Disable",
                    BtSettingsAppStartSubmenuIndexEnable,
                    bt_settings_scene_start_submenu_callback,
                    app);
            } else {
                scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneDisableDialog);
            }
            consumed = true;
        }
    }
    return consumed;
}

void bt_settings_scene_start_on_exit(void* context) {
    BtSettingsApp* app = context;
    submenu_clean(app->submenu);
}
