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
    osSemaphoreId_t semaphore = API_LOCK_INIT_LOCKED();
    furi_check(semaphore != NULL);

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
        .semaphore = semaphore,
        .command = DialogsAppCommandFileOpen,
        .data = &data,
        .return_data = &return_data,
    };

    furi_check(osMessageQueuePut(context->message_queue, &message, 0, osWaitForever) == osOK);
    API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(semaphore);

    return return_data.bool_value;
}

/****************** Message ******************/

DialogMessageButton dialog_message_show(DialogsApp* context, const DialogMessage* dialog_message) {
    osSemaphoreId_t semaphore = API_LOCK_INIT_LOCKED();
    furi_check(semaphore != NULL);

    DialogsAppData data = {
        .dialog = {
            .message = dialog_message,
        }};

    DialogsAppReturn return_data;
    DialogsAppMessage message = {
        .semaphore = semaphore,
        .command = DialogsAppCommandDialog,
        .data = &data,
        .return_data = &return_data,
    };

    furi_check(osMessageQueuePut(context->message_queue, &message, 0, osWaitForever) == osOK);
    API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(semaphore);

    return return_data.dialog_value;
}
