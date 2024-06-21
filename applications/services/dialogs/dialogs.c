#include "dialogs/dialogs_message.h"
#include <toolbox/api_lock.h>
#include "dialogs_module_file_browser.h"
#include "dialogs_module_message.h"

void dialog_file_browser_set_basic_options(
    DialogsFileBrowserOptions* options,
    const char* extension,
    const Icon* icon) {
    furi_check(options);
    options->extension = extension;
    options->base_path = NULL;
    options->skip_assets = true;
    options->hide_dot_files = true;
    options->icon = icon;
    options->hide_ext = true;
    options->item_loader_callback = NULL;
    options->item_loader_context = NULL;
}

static DialogsApp* dialogs_app_alloc(void) {
    DialogsApp* app = malloc(sizeof(DialogsApp));
    app->message_queue = furi_message_queue_alloc(8, sizeof(DialogsAppMessage));

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
    api_lock_unlock(message->lock);
}

int32_t dialogs_srv(void* p) {
    UNUSED(p);
    DialogsApp* app = dialogs_app_alloc();
    furi_record_create(RECORD_DIALOGS, app);

    DialogsAppMessage message;
    while(1) {
        if(furi_message_queue_get(app->message_queue, &message, FuriWaitForever) == FuriStatusOk) {
            dialogs_app_process_message(app, &message);
        }
    }

    return 0;
}
