#include "dialogs_i.h"
#include "dialogs_api_lock.h"
#include <gui/modules/dialog_ex.h>

typedef struct {
    FuriApiLock lock;
    DialogMessageButton result;
} DialogsAppMessageContext;

struct DialogMessage {
    const char* header_text;
    uint8_t header_text_x;
    uint8_t header_text_y;
    Align header_horizontal;
    Align header_vertical;
    const char* dialog_text;
    uint8_t dialog_text_x;
    uint8_t dialog_text_y;
    Align dialog_text_horizontal;
    Align dialog_text_vertical;
    const Icon* icon;
    uint8_t icon_x;
    uint8_t icon_y;
    const char* left_button_text;
    const char* center_button_text;
    const char* right_button_text;
};

static void dialogs_app_message_back_callback(void* context) {
    furi_assert(context);
    DialogsAppMessageContext* message_context = context;
    message_context->result = DialogMessageButtonBack;
    API_LOCK_UNLOCK(message_context->lock);
}

static void dialogs_app_message_callback(DialogExResult result, void* context) {
    furi_assert(context);
    DialogsAppMessageContext* message_context = context;
    switch(result) {
    case DialogExResultLeft:
        message_context->result = DialogMessageButtonLeft;
        break;
    case DialogExResultRight:
        message_context->result = DialogMessageButtonRight;
        break;
    case DialogExResultCenter:
        message_context->result = DialogMessageButtonCenter;
        break;
    default:
        break;
    }
    API_LOCK_UNLOCK(message_context->lock);
}

DialogMessageButton dialogs_app_process_module_message(const DialogsAppMessageDataDialog* data) {
    DialogMessageButton ret = DialogMessageButtonBack;
    Gui* gui = furi_record_open(RECORD_GUI);
    const DialogMessage* message = data->message;
    DialogsAppMessageContext* message_context = malloc(sizeof(DialogsAppMessageContext));
    message_context->lock = API_LOCK_INIT_LOCKED();

    ViewHolder* view_holder = view_holder_alloc();
    view_holder_attach_to_gui(view_holder, gui);
    view_holder_set_back_callback(view_holder, dialogs_app_message_back_callback, message_context);

    DialogEx* dialog_ex = dialog_ex_alloc();
    dialog_ex_set_result_callback(dialog_ex, dialogs_app_message_callback);
    dialog_ex_set_context(dialog_ex, message_context);
    dialog_ex_set_header(
        dialog_ex,
        message->header_text,
        message->header_text_x,
        message->header_text_y,
        message->header_horizontal,
        message->header_vertical);
    dialog_ex_set_text(
        dialog_ex,
        message->dialog_text,
        message->dialog_text_x,
        message->dialog_text_y,
        message->dialog_text_horizontal,
        message->dialog_text_vertical);
    dialog_ex_set_icon(dialog_ex, message->icon_x, message->icon_y, message->icon);
    dialog_ex_set_left_button_text(dialog_ex, message->left_button_text);
    dialog_ex_set_center_button_text(dialog_ex, message->center_button_text);
    dialog_ex_set_right_button_text(dialog_ex, message->right_button_text);

    view_holder_set_view(view_holder, dialog_ex_get_view(dialog_ex));
    view_holder_start(view_holder);
    API_LOCK_WAIT_UNTIL_UNLOCK(message_context->lock);

    ret = message_context->result;

    view_holder_stop(view_holder);
    view_holder_free(view_holder);
    dialog_ex_free(dialog_ex);
    API_LOCK_FREE(message_context->lock);
    free(message_context);
    furi_record_close(RECORD_GUI);

    return ret;
}

DialogMessage* dialog_message_alloc() {
    DialogMessage* message = malloc(sizeof(DialogMessage));
    return message;
}

void dialog_message_free(DialogMessage* message) {
    free(message);
}

void dialog_message_set_text(
    DialogMessage* message,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    message->dialog_text = text;
    message->dialog_text_x = x;
    message->dialog_text_y = y;
    message->dialog_text_horizontal = horizontal;
    message->dialog_text_vertical = vertical;
}

void dialog_message_set_header(
    DialogMessage* message,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    message->header_text = text;
    message->header_text_x = x;
    message->header_text_y = y;
    message->header_horizontal = horizontal;
    message->header_vertical = vertical;
}

void dialog_message_set_icon(DialogMessage* message, const Icon* icon, uint8_t x, uint8_t y) {
    message->icon = icon;
    message->icon_x = x;
    message->icon_y = y;
}

void dialog_message_set_buttons(
    DialogMessage* message,
    const char* left,
    const char* center,
    const char* right) {
    message->left_button_text = left;
    message->center_button_text = center;
    message->right_button_text = right;
}
