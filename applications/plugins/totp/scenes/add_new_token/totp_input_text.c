#include "totp_input_text.h"
#include <gui/view_i.h>
#include "../../types/common.h"

void view_draw(View* view, Canvas* canvas) {
    furi_assert(view);
    if(view->draw_callback) {
        void* data = view_get_model(view);
        view->draw_callback(canvas, data);
        view_unlock_model(view);
    }
}

bool view_input(View* view, InputEvent* event) {
    furi_assert(view);
    if(view->input_callback) {
        return view->input_callback(event, view->context);
    } else {
        return false;
    }
}

void view_unlock_model(View* view) {
    furi_assert(view);
    if(view->model_type == ViewModelTypeLocking) {
        ViewModelLocking* model = (ViewModelLocking*)(view->model);
        furi_check(furi_mutex_release(model->mutex) == FuriStatusOk);
    }
}

static void commit_text_input_callback(void* context) {
    InputTextSceneState* text_input_state = (InputTextSceneState *)context;
    if (text_input_state->callback != 0) {
        InputTextSceneCallbackResult* result = malloc(sizeof(InputTextSceneCallbackResult));
        result->user_input_length = strlen(text_input_state->text_input_buffer);
        result->user_input = malloc(result->user_input_length + 1);
        result->callback_data = text_input_state->callback_data;
        strcpy(result->user_input, text_input_state->text_input_buffer);
        text_input_state->callback(result);
    }
}

InputTextSceneState* totp_input_text_activate(InputTextSceneContext* context) {
    InputTextSceneState* text_input_state = malloc(sizeof(InputTextSceneState));
    text_input_state->text_input = text_input_alloc();
    text_input_state->text_input_view = text_input_get_view(text_input_state->text_input);
    text_input_state->callback = context->callback;
    text_input_state->callback_data = context->callback_data;
    text_input_set_header_text(text_input_state->text_input, context->header_text);
    text_input_set_result_callback(
        text_input_state->text_input,
        commit_text_input_callback,
        text_input_state,
        &text_input_state->text_input_buffer[0],
        INPUT_BUFFER_SIZE,
        true);
    return text_input_state;
}

void totp_input_text_render(Canvas* const canvas, InputTextSceneState* text_input_state) {
    view_draw(text_input_state->text_input_view, canvas);
}

bool totp_input_text_handle_event(PluginEvent* const event, InputTextSceneState* text_input_state) {
    if(event->type == EventTypeKey) {
        view_input(text_input_state->text_input_view, &event->input);
    }

    return true;
}

void totp_input_text_free(InputTextSceneState* state) {
    text_input_free(state->text_input);
    free(state);
}
