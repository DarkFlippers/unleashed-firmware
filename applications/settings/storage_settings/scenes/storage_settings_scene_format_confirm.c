#include "../storage_settings.h"

static void
    storage_settings_scene_format_confirm_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_format_confirm_on_enter(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    FS_Error sd_status = storage_sd_status(app->fs_api);

    if(sd_status == FSE_NOT_READY) {
        dialog_ex_set_icon(dialog_ex, 83, 22, &I_WarningDolphinFlip_45x42);
        dialog_ex_set_header(dialog_ex, "SD Card Not Mounted", 64, 3, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog_ex, "Try to reinsert\nor format SD\ncard.", 3, 19, AlignLeft, AlignTop);
        dialog_ex_set_center_button_text(dialog_ex, "Ok");
    } else {
        dialog_ex_set_header(dialog_ex, "Format SD Card?", 64, 0, AlignCenter, AlignTop);
        dialog_ex_set_text(dialog_ex, "All data will be lost!", 64, 12, AlignCenter, AlignTop);
        dialog_ex_set_left_button_text(dialog_ex, "Cancel");
        dialog_ex_set_right_button_text(dialog_ex, "Format");
    }

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(
        dialog_ex, storage_settings_scene_format_confirm_dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);
}

bool storage_settings_scene_format_confirm_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
        case DialogExResultCenter:
            consumed = scene_manager_previous_scene(app->scene_manager);
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, StorageSettingsFormatting);
            consumed = true;
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void storage_settings_scene_format_confirm_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(dialog_ex);
}
