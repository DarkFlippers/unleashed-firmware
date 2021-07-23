#include "../storage-settings.h"

static void
    storage_settings_scene_unmount_confirm_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_unmount_confirm_on_enter(void* context) {
    StorageSettings* app = context;
    FS_Error sd_status = storage_sd_status(app->fs_api);
    DialogEx* dialog_ex = app->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Back");

    if(sd_status == FSE_NOT_READY) {
        dialog_ex_set_header(dialog_ex, "SD card not mounted", 64, 10, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex,
            "If an SD card is inserted,\r\npull it out and reinsert it",
            64,
            32,
            AlignCenter,
            AlignCenter);
    } else {
        dialog_ex_set_right_button_text(dialog_ex, "Unmount");
        dialog_ex_set_header(dialog_ex, "Unmount SD card?", 64, 10, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex, "SD card will be\nunavailable", 64, 32, AlignCenter, AlignCenter);
    }

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(
        dialog_ex, storage_settings_scene_unmount_confirm_dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);
}

bool storage_settings_scene_unmount_confirm_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            consumed = scene_manager_previous_scene(app->scene_manager);
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, StorageSettingsUnmounted);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void storage_settings_scene_unmount_confirm_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
}
