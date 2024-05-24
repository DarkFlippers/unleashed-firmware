#include "../power_settings_app.h"

enum PowerSettingsRebootSubmenuIndex {
    PowerSettingsRebootSubmenuIndexDfu,
    PowerSettingsRebootSubmenuIndexOs,
};

void power_settings_scene_reboot_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    PowerSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void power_settings_scene_reboot_on_enter(void* context) {
    PowerSettingsApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_set_header(submenu, "Reboot Type");
    submenu_add_item(
        submenu,
        "Firmware Upgrade",
        PowerSettingsRebootSubmenuIndexDfu,
        power_settings_scene_reboot_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Reboot Flipper",
        PowerSettingsRebootSubmenuIndexOs,
        power_settings_scene_reboot_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewSubmenu);
}

bool power_settings_scene_reboot_on_event(void* context, SceneManagerEvent event) {
    PowerSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PowerSettingsRebootSubmenuIndexDfu) {
            scene_manager_set_scene_state(
                app->scene_manager, PowerSettingsAppSceneRebootConfirm, RebootTypeDFU);
            scene_manager_next_scene(app->scene_manager, PowerSettingsAppSceneRebootConfirm);
        } else if(event.event == PowerSettingsRebootSubmenuIndexOs) {
            scene_manager_set_scene_state(
                app->scene_manager, PowerSettingsAppSceneRebootConfirm, RebootTypeNormal);
            scene_manager_next_scene(app->scene_manager, PowerSettingsAppSceneRebootConfirm);
        }
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_reboot_on_exit(void* context) {
    PowerSettingsApp* app = context;
    submenu_reset(app->submenu);
}
