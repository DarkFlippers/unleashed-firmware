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

    submenu_set_header(submenu, "Reboot type");
    submenu_add_item(
        submenu,
        "Firmware upgrade",
        PowerSettingsRebootSubmenuIndexDfu,
        power_settings_scene_reboot_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Flipper OS",
        PowerSettingsRebootSubmenuIndexOs,
        power_settings_scene_reboot_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewSubmenu);
}

bool power_settings_scene_reboot_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PowerSettingsRebootSubmenuIndexDfu) {
            power_reboot(PowerBootModeDfu);
        } else if(event.event == PowerSettingsRebootSubmenuIndexOs) {
            power_reboot(PowerBootModeNormal);
        }
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_reboot_on_exit(void* context) {
    PowerSettingsApp* app = context;
    submenu_reset(app->submenu);
}
