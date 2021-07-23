#include "dialogs-i.h"
#include "dialogs-api-lock.h"
#include "dialogs-module-file-select.h"
#include "dialogs-module-message.h"

static DialogsApp* dialogs_app_alloc() {
    DialogsApp* app = malloc(sizeof(DialogsApp));
    app->message_queue = osMessageQueueNew(8, sizeof(DialogsAppMessage), NULL);

    return app;
}

static void dialogs_app_process_message(DialogsApp* app, DialogsAppMessage* message) {
    switch(message->command) {
    case DialogsAppCommandFileOpen:
        message->return_data->bool_value =
            dialogs_app_process_module_file_select(&message->data->file_select);
        break;
    case DialogsAppCommandDialog:
        message->return_data->dialog_value =
            dialogs_app_process_module_message(&message->data->dialog);
        break;
    }
    API_LOCK_UNLOCK(message->semaphore);
}

int32_t dialogs_app(void* p) {
    DialogsApp* app = dialogs_app_alloc();
    furi_record_create("dialogs", app);

    DialogsAppMessage message;
    while(1) {
        if(osMessageQueueGet(app->message_queue, &message, NULL, osWaitForever) == osOK) {
            dialogs_app_process_message(app, &message);
        }
    }

    return 0;
}