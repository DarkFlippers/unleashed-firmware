#include "../power_settings_app.h"

void power_settings_scene_reboot_confirm_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    PowerSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void power_settings_scene_reboot_confirm_on_enter(void* context) {
    PowerSettingsApp* app = context;
    DialogEx* dialog = app->dialog;

    RebootType reboot_type =
        scene_manager_get_scene_state(app->scene_manager, PowerSettingsAppSceneRebootConfirm);

    if(reboot_type == RebootTypeDFU) {
        dialog_ex_set_header(dialog, "Reboot to DFU Mode?", 64, 0, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog,
            "Needed for device maintenance\nor firmware upgrades",
            64,
            14,
            AlignCenter,
            AlignTop);
    } else if(reboot_type == RebootTypeNormal) {
        dialog_ex_set_header(dialog, "Reboot Flipper?", 64, 0, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog, "May help with some firmware\n issues", 64, 14, AlignCenter, AlignTop);
    } else {
        furi_crash("Invalid reboot type");
    }

    dialog_ex_set_left_button_text(dialog, "Cancel");
    dialog_ex_set_right_button_text(dialog, "Reboot");

    dialog_ex_set_result_callback(dialog, power_settings_scene_reboot_confirm_dialog_callback);
    dialog_ex_set_context(dialog, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewDialog);
}

bool power_settings_scene_reboot_confirm_on_event(void* context, SceneManagerEvent event) {
    PowerSettingsApp* app = context;
    bool consumed = false;
    RebootType reboot_type =
        scene_manager_get_scene_state(app->scene_manager, PowerSettingsAppSceneRebootConfirm);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_previous_scene(app->scene_manager);
        } else if(event.event == DialogExResultRight) {
            Power* power = furi_record_open(RECORD_POWER);

            if(reboot_type == RebootTypeDFU) {
                power_reboot(power, PowerBootModeDfu);
            } else {
                power_reboot(power, PowerBootModeNormal);
            }
        }
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_reboot_confirm_on_exit(void* context) {
    PowerSettingsApp* app = context;
    dialog_ex_reset(app->dialog);
}
