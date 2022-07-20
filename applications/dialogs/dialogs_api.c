#include "dialogs/dialogs_message.h"
#include "dialogs_i.h"
#include "dialogs_api_lock.h"
#include "m-string.h"

/****************** File browser ******************/

bool dialog_file_browser_show(
    DialogsApp* context,
    string_ptr result_path,
    string_ptr path,
    const char* extension,
    bool skip_assets,
    const Icon* icon,
    bool hide_ext) {
    FuriApiLock lock = API_LOCK_INIT_LOCKED();
    furi_check(lock != NULL);

    DialogsAppData data = {
        .file_browser = {
            .extension = extension,
            .result_path = result_path,
            .file_icon = icon,
            .hide_ext = hide_ext,
            .skip_assets = skip_assets,
            .preselected_filename = path,

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
    API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(lock);

    return return_data.bool_value;
}

/****************** Message ******************/

DialogMessageButton dialog_message_show(DialogsApp* context, const DialogMessage* dialog_message) {
    FuriApiLock lock = API_LOCK_INIT_LOCKED();
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
    API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(lock);

    return return_data.dialog_value;
}

/****************** Storage error ******************/

void dialog_message_show_storage_error(DialogsApp* context, const char* error_text) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_text(message, error_text, 88, 32, AlignCenter, AlignCenter);
    dialog_message_set_icon(message, &I_SDQuestion_35x43, 5, 6);
    dialog_message_set_buttons(message, "Back", NULL, NULL);
    dialog_message_show(context, message);
    dialog_message_free(message);
}
