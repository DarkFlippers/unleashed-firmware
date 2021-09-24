#include "../power_settings_app.h"

void power_settings_scene_usb_disconnect_on_enter(void* context) {
    PowerSettingsApp* app = context;

    dialog_ex_set_header(
        app->dialog, "Disconnect USB for safe\nshutdown", 64, 26, AlignCenter, AlignTop);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewDialog);
}

bool power_settings_scene_usb_disconnect_on_event(void* context, SceneManagerEvent event) {
    return true;
}

void power_settings_scene_usb_disconnect_on_exit(void* context) {
}
