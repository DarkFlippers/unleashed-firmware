#include "dialogs-i.h"
#include "dialogs-api-lock.h"

/****************** File select ******************/

bool dialog_file_select_show(
    DialogsApp* context,
    const char* path,
    const char* extension,
    char* result,
    uint8_t result_size,
    const char* preselected_filename) {
    FuriApiLock lock = API_LOCK_INIT_LOCKED();
    furi_check(lock != NULL);

    DialogsAppData data = {
        .file_select = {
            .path = path,
            .extension = extension,
            .result = result,
            .result_size = result_size,
            .preselected_filename = preselected_filename,
        }};

    DialogsAppReturn return_data;
    DialogsAppMessage message = {
        .lock = lock,
        .command = DialogsAppCommandFileOpen,
        .data = &data,
        .return_data = &return_data,
    };

    furi_check(osMessageQueuePut(context->message_queue, &message, 0, osWaitForever) == osOK);
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

    furi_check(osMessageQueuePut(context->message_queue, &message, 0, osWaitForever) == osOK);
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