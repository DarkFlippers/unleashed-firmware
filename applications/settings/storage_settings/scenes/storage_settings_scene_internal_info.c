#include "../storage_settings.h"
#include <furi_hal_version.h>

static void
    storage_settings_scene_internal_info_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_internal_info_on_enter(void* context) {
    StorageSettings* app = context;
    uint64_t total_space;
    uint64_t free_space;
    FS_Error error =
        storage_common_fs_info(app->fs_api, STORAGE_INT_PATH_PREFIX, &total_space, &free_space);
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, storage_settings_scene_internal_info_dialog_callback);

    if(error != FSE_OK) {
        dialog_ex_set_header(
            dialog_ex, "Internal Storage Error", 64, 10, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex, storage_error_get_desc(error), 64, 32, AlignCenter, AlignCenter);
    } else {
        furi_string_printf(
            app->text_string,
            "Name: %s\nType: Virtual\nTotal: %lu KiB\nFree: %lu KiB",
            furi_hal_version_get_name_ptr() ? furi_hal_version_get_name_ptr() : "Unknown",
            (uint32_t)(total_space / 1024),
            (uint32_t)(free_space / 1024));
        dialog_ex_set_text(
            dialog_ex, furi_string_get_cstr(app->text_string), 4, 4, AlignLeft, AlignTop);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);
}

bool storage_settings_scene_internal_info_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            consumed = scene_manager_previous_scene(app->scene_manager);
            break;
        }
    }
    return consumed;
}

void storage_settings_scene_internal_info_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(dialog_ex);

    furi_string_reset(app->text_string);
}
