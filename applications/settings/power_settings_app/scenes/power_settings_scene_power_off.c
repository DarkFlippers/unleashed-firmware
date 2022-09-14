#include "../power_settings_app.h"

void power_settings_scene_power_off_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    PowerSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void power_settings_scene_power_off_on_enter(void* context) {
    PowerSettingsApp* app = context;
    DialogEx* dialog = app->dialog;

    dialog_ex_set_header(dialog, "Turn Off Device?", 64, 2, AlignCenter, AlignTop);
    dialog_ex_set_text(
        dialog, "   I will be\nwaiting for\n you here...", 78, 16, AlignLeft, AlignTop);
    dialog_ex_set_icon(dialog, 21, 13, &I_Cry_dolph_55x52);
    dialog_ex_set_left_button_text(dialog, "Back");
    dialog_ex_set_right_button_text(dialog, "OFF");
    dialog_ex_set_result_callback(dialog, power_settings_scene_power_off_dialog_callback);
    dialog_ex_set_context(dialog, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewDialog);
}

bool power_settings_scene_power_off_on_event(void* context, SceneManagerEvent event) {
    PowerSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
        } else if(event.event == DialogExResultRight) {
            power_off(app->power);
        }
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_power_off_on_exit(void* context) {
    PowerSettingsApp* app = context;
    dialog_ex_reset(app->dialog);
}
