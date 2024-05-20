#include "../storage_settings.h"

static void storage_settings_scene_sd_info_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_sd_info_on_enter(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    SDInfo sd_info;
    FS_Error sd_status = storage_sd_info(app->fs_api, &sd_info);

    scene_manager_set_scene_state(app->scene_manager, StorageSettingsSDInfo, sd_status);

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, storage_settings_scene_sd_info_dialog_callback);

    if(sd_status != FSE_OK) {
        dialog_ex_set_icon(dialog_ex, 83, 22, &I_WarningDolphinFlip_45x42);
        dialog_ex_set_header(dialog_ex, "SD Card Not Mounted", 64, 3, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog_ex, "Try to reinsert\nor format SD\ncard.", 3, 19, AlignLeft, AlignTop);
        dialog_ex_set_center_button_text(dialog_ex, "Ok");
    } else {
        furi_string_printf(
            app->text_string,
            "Label: %s\nType: %s\n",
            sd_info.label,
            sd_api_get_fs_type_text(sd_info.fs_type));

        if(sd_info.kb_total < 1024) {
            furi_string_cat_printf(app->text_string, "Total: %lu KiB\n", sd_info.kb_total);
        } else if(sd_info.kb_total < 1024 * 1024) {
            furi_string_cat_printf(app->text_string, "Total: %lu MiB\n", sd_info.kb_total / 1024);
        } else {
            furi_string_cat_printf(
                app->text_string, "Total: %lu GiB\n", sd_info.kb_total / (1024 * 1024));
        }

        if(sd_info.kb_free < 1024) {
            furi_string_cat_printf(app->text_string, "Free: %lu KiB\n", sd_info.kb_free);
        } else if(sd_info.kb_free < 1024 * 1024) {
            furi_string_cat_printf(app->text_string, "Free: %lu MiB\n", sd_info.kb_free / 1024);
        } else {
            furi_string_cat_printf(
                app->text_string, "Free: %lu GiB\n", sd_info.kb_free / (1024 * 1024));
        }

        furi_string_cat_printf(
            app->text_string,
            "%02X%s %s v%i.%i\nSN:%04lX %02i/%i",
            sd_info.manufacturer_id,
            sd_info.oem_id,
            sd_info.product_name,
            sd_info.product_revision_major,
            sd_info.product_revision_minor,
            sd_info.product_serial_number,
            sd_info.manufacturing_month,
            sd_info.manufacturing_year);

        dialog_ex_set_text(
            dialog_ex, furi_string_get_cstr(app->text_string), 4, 1, AlignLeft, AlignTop);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);
}

bool storage_settings_scene_sd_info_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    FS_Error sd_status = scene_manager_get_scene_state(app->scene_manager, StorageSettingsSDInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
        case DialogExResultCenter:
            consumed = scene_manager_previous_scene(app->scene_manager);
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, StorageSettingsUnmounted);
            consumed = true;
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack && sd_status != FSE_OK) {
        consumed = true;
    }

    return consumed;
}

void storage_settings_scene_sd_info_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(dialog_ex);

    furi_string_reset(app->text_string);
}
