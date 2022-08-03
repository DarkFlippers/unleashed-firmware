#include "../bt_settings_app.h"
#include "furi_hal_bt.h"

void bt_settings_scene_forget_dev_confirm_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    BtSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void bt_settings_scene_forget_dev_confirm_on_enter(void* context) {
    BtSettingsApp* app = context;
    DialogEx* dialog = app->dialog;
    dialog_ex_set_header(dialog, "Unpair All Devices?", 64, 3, AlignCenter, AlignTop);
    dialog_ex_set_text(
        dialog, "All previous pairings\nwill be lost!", 64, 22, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog, "Back");
    dialog_ex_set_right_button_text(dialog, "Unpair");
    dialog_ex_set_context(dialog, app);
    dialog_ex_set_result_callback(dialog, bt_settings_scene_forget_dev_confirm_dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewDialog);
}

bool bt_settings_scene_forget_dev_confirm_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        } else if(event.event == DialogExResultRight) {
            bt_forget_bonded_devices(app->bt);
            scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneForgetDevSuccess);
            consumed = true;
        }
    }

    return consumed;
}

void bt_settings_scene_forget_dev_confirm_on_exit(void* context) {
    BtSettingsApp* app = context;
    dialog_ex_reset(app->dialog);
}
