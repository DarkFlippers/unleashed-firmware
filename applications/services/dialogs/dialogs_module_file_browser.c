#include "dialogs_i.h"
#include "dialogs_api_lock.h"
#include "gui/modules/file_browser.h"

typedef struct {
    FuriApiLock lock;
    bool result;
} DialogsAppFileBrowserContext;

static void dialogs_app_file_browser_back_callback(void* context) {
    furi_assert(context);
    DialogsAppFileBrowserContext* file_browser_context = context;
    file_browser_context->result = false;
    API_LOCK_UNLOCK(file_browser_context->lock);
}

static void dialogs_app_file_browser_callback(void* context) {
    furi_assert(context);
    DialogsAppFileBrowserContext* file_browser_context = context;
    file_browser_context->result = true;
    API_LOCK_UNLOCK(file_browser_context->lock);
}

bool dialogs_app_process_module_file_browser(const DialogsAppMessageDataFileBrowser* data) {
    bool ret = false;
    Gui* gui = furi_record_open(RECORD_GUI);

    DialogsAppFileBrowserContext* file_browser_context =
        malloc(sizeof(DialogsAppFileBrowserContext));
    file_browser_context->lock = API_LOCK_INIT_LOCKED();

    ViewHolder* view_holder = view_holder_alloc();
    view_holder_attach_to_gui(view_holder, gui);
    view_holder_set_back_callback(
        view_holder, dialogs_app_file_browser_back_callback, file_browser_context);

    FileBrowser* file_browser = file_browser_alloc(data->result_path);
    file_browser_set_callback(
        file_browser, dialogs_app_file_browser_callback, file_browser_context);
    file_browser_configure(
        file_browser, data->extension, data->skip_assets, data->file_icon, data->hide_ext);
    file_browser_set_item_callback(file_browser, data->item_callback, data->item_callback_context);
    file_browser_start(file_browser, data->preselected_filename);

    view_holder_set_view(view_holder, file_browser_get_view(file_browser));
    view_holder_start(view_holder);
    API_LOCK_WAIT_UNTIL_UNLOCK(file_browser_context->lock);

    ret = file_browser_context->result;

    view_holder_stop(view_holder);
    view_holder_free(view_holder);
    file_browser_stop(file_browser);
    file_browser_free(file_browser);
    API_LOCK_FREE(file_browser_context->lock);
    free(file_browser_context);
    furi_record_close(RECORD_GUI);

    return ret;
}
