#include "dialogs-i.h"
#include "dialogs-api-lock.h"
#include <gui/modules/file_select.h>

typedef struct {
    osSemaphoreId_t semaphore;
    bool result;
} DialogsAppFileSelectContext;

static void dialogs_app_file_select_back_callback(void* context) {
    furi_assert(context);
    DialogsAppFileSelectContext* file_select_context = context;
    file_select_context->result = false;
    API_LOCK_UNLOCK(file_select_context->semaphore);
}

static void dialogs_app_file_select_callback(bool result, void* context) {
    furi_assert(context);
    DialogsAppFileSelectContext* file_select_context = context;
    file_select_context->result = result;
    API_LOCK_UNLOCK(file_select_context->semaphore);
}

bool dialogs_app_process_module_file_select(const DialogsAppMessageDataFileSelect* data) {
    bool ret = false;
    Gui* gui = furi_record_open("gui");

    DialogsAppFileSelectContext* file_select_context =
        furi_alloc(sizeof(DialogsAppFileSelectContext));
    file_select_context->semaphore = API_LOCK_INIT_LOCKED();

    ViewHolder* view_holder = view_holder_alloc();
    view_holder_attach_to_gui(view_holder, gui);
    view_holder_set_back_callback(
        view_holder, dialogs_app_file_select_back_callback, file_select_context);

    FileSelect* file_select = file_select_alloc();
    file_select_set_callback(file_select, dialogs_app_file_select_callback, file_select_context);
    file_select_set_filter(file_select, data->path, data->extension);
    file_select_set_result_buffer(file_select, data->result, data->result_size);
    file_select_init(file_select);
    if(data->preselected_filename != NULL) {
        file_select_set_selected_file(file_select, data->preselected_filename);
    }

    view_holder_set_view(view_holder, file_select_get_view(file_select));
    view_holder_start(view_holder);
    API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(file_select_context->semaphore);

    ret = file_select_context->result;

    free(file_select_context);
    view_holder_stop(view_holder);
    view_holder_free(view_holder);
    file_select_free(file_select);
    furi_record_close("gui");

    return ret;
}