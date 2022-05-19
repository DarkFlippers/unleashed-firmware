#include "../storage_settings.h"

static void storage_settings_scene_sd_info_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_sd_info_on_enter(void* context) {
    StorageSettings* app = context;
    SDInfo sd_info;
    FS_Error sd_status = storage_sd_info(app->fs_api, &sd_info);
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, storage_settings_scene_sd_info_dialog_callback);

    if(sd_status != FSE_OK) {
        dialog_ex_set_header(dialog_ex, "SD card not mounted", 64, 10, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex,
            "If an SD card is inserted,\r\npull it out and reinsert it",
            64,
            32,
            AlignCenter,
            AlignCenter);
    } else {
        string_printf(
            app->text_string,
            "Label: %s\nType: %s\n%lu KB total\n%lu KB free",
            sd_info.label,
            sd_api_get_fs_type_text(sd_info.fs_type),
            sd_info.kb_total,
            sd_info.kb_free);
        dialog_ex_set_text(
            dialog_ex, string_get_cstr(app->text_string), 4, 4, AlignLeft, AlignTop);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);
}

bool storage_settings_scene_sd_info_on_event(void* context, SceneManagerEvent event) {
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

void storage_settings_scene_sd_info_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(dialog_ex);

    string_reset(app->text_string);
}
