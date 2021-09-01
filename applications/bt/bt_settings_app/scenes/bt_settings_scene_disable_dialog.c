#include "../bt_settings_app.h"
#include <furi-hal-boot.h>
#include <furi-hal-power.h>

static void bt_setting_disable_dialog_callback(DialogExResult result, void* context) {
    BtSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void bt_settings_scene_disable_dialog_on_enter(void* context) {
    BtSettingsApp* app = context;
    DialogEx* dialog_ex = app->dialog_ex;
    dialog_ex_set_text(
        dialog_ex, "Reboot device\nto disable Bluetooth", 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_right_button_text(dialog_ex, "Reboot");
    dialog_ex_set_result_callback(dialog_ex, bt_setting_disable_dialog_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewDialogEx);
}

bool bt_settings_scene_disable_dialog_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        } else if(event.event == DialogExResultRight) {
            app->settings.enabled = false;
            bt_settings_save(&app->settings);
            furi_hal_boot_set_mode(FuriHalBootModeNormal);
            furi_hal_power_reset();
        }
    }
    return consumed;
}

void bt_settings_scene_disable_dialog_on_exit(void* context) {
    BtSettingsApp* app = context;
    dialog_ex_clean(app->dialog_ex);
}
