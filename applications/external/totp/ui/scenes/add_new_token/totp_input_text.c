#include "totp_input_text.h"

#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>

typedef struct {
    InputTextResult* result;
    ViewDispatcher* view_dispatcher;
} InputTextContext;

static void commit_text_input_callback(void* ctx) {
    InputTextContext* context = ctx;
    context->result->user_input_length = strnlen(context->result->user_input, INPUT_BUFFER_SIZE);
    context->result->success = true;
    view_dispatcher_stop(context->view_dispatcher);
}

static bool back_event_callback(void* ctx) {
    InputTextContext* context = ctx;
    context->result->success = false;
    view_dispatcher_stop(context->view_dispatcher);
    return false;
}

void totp_input_text(Gui* gui, const char* header_text, InputTextResult* result) {
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    TextInput* text_input = text_input_alloc();
    InputTextContext context = {.result = result, .view_dispatcher = view_dispatcher};
    text_input_set_header_text(text_input, header_text);
    text_input_set_result_callback(
        text_input,
        commit_text_input_callback,
        &context,
        result->user_input,
        INPUT_BUFFER_SIZE,
        true);

    view_dispatcher_enable_queue(view_dispatcher);
    view_dispatcher_add_view(view_dispatcher, 0, text_input_get_view(text_input));

    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    view_dispatcher_set_navigation_event_callback(view_dispatcher, &back_event_callback);
    view_dispatcher_set_event_callback_context(view_dispatcher, &context);
    view_dispatcher_switch_to_view(view_dispatcher, 0);

    view_dispatcher_run(view_dispatcher);

    view_dispatcher_remove_view(view_dispatcher, 0);
    view_dispatcher_free(view_dispatcher);
    text_input_free(text_input);
}
