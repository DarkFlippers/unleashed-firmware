#include "dialogs_message.h"
#include <toolbox/api_lock.h>
#include <assets_icons.h>
#include <storage/storage.h>

/****************** File browser ******************/

bool dialog_file_browser_show(
    DialogsApp* context,
    FuriString* result_path,
    FuriString* path,
    const DialogsFileBrowserOptions* options) {
    FuriApiLock lock = api_lock_alloc_locked();
    furi_check(lock != NULL);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* base_path = furi_string_alloc();

    if(options && options->base_path) {
        furi_string_set(base_path, options->base_path);
        storage_common_resolve_path_and_ensure_app_directory(storage, base_path);
    }

    if(result_path) {
        storage_common_resolve_path_and_ensure_app_directory(storage, result_path);
    }

    if(path) {
        storage_common_resolve_path_and_ensure_app_directory(storage, path);
    }

    DialogsAppData data = {
        .file_browser = {
            .extension = options ? options->extension : "",
            .result_path = result_path,
            .file_icon = options ? options->icon : NULL,
            .hide_ext = options ? options->hide_ext : true,
            .skip_assets = options ? options->skip_assets : true,
            .hide_dot_files = options ? options->hide_dot_files : true,
            .preselected_filename = path,
            .item_callback = options ? options->item_loader_callback : NULL,
            .item_callback_context = options ? options->item_loader_context : NULL,
            .base_path = furi_string_get_cstr(base_path),
        }};

    DialogsAppReturn return_data;
    DialogsAppMessage message = {
        .lock = lock,
        .command = DialogsAppCommandFileBrowser,
        .data = &data,
        .return_data = &return_data,
    };

    furi_check(
        furi_message_queue_put(context->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(lock);

    furi_record_close(RECORD_STORAGE);
    furi_string_free(base_path);

    return return_data.bool_value;
}

/****************** Message ******************/

DialogMessageButton dialog_message_show(DialogsApp* context, const DialogMessage* dialog_message) {
    furi_check(context);

    FuriApiLock lock = api_lock_alloc_locked();
    furi_check(lock != NULL);

    DialogsAppData data = {
        .dialog = {
            .message = dialog_message,
        }};

    DialogsAppReturn return_data;
    DialogsAppMessage message = {
        .lock = lock,
        .command = DialogsAppCommandDialog,
        .data = &data,
        .return_data = &return_data,
    };

    furi_check(
        furi_message_queue_put(context->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(lock);

    return return_data.dialog_value;
}

/****************** Storage error ******************/

void dialog_message_show_storage_error(DialogsApp* context, const char* error_text) {
    furi_check(context);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_text(message, error_text, 88, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(message, &I_SDQuestion_35x43, 5, 6);
    dialog_message_set_buttons(message, "Back", NULL, NULL);
    dialog_message_show(context, message);
    dialog_message_free(message);
}
