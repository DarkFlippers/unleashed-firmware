#include "dialogs/dialogs_message.h"
#include "dialogs_i.h"
#include "dialogs_api_lock.h"
#include "dialogs_module_file_browser.h"
#include "dialogs_module_message.h"

static DialogsApp* dialogs_app_alloc() {
    DialogsApp* app = malloc(sizeof(DialogsApp));
    app->message_queue = osMessageQueueNew(8, sizeof(DialogsAppMessage), NULL);

    return app;
}

static void dialogs_app_process_message(DialogsApp* app, DialogsAppMessage* message) {
    UNUSED(app);
    switch(message->command) {
    case DialogsAppCommandFileBrowser:
        message->return_data->bool_value =
            dialogs_app_process_module_file_browser(&message->data->file_browser);
        break;
    case DialogsAppCommandDialog:
        message->return_data->dialog_value =
            dialogs_app_process_module_message(&message->data->dialog);
        break;
    }
    API_LOCK_UNLOCK(message->lock);
}

int32_t dialogs_srv(void* p) {
    UNUSED(p);
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
