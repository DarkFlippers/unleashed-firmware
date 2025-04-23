#include "infrared_progress_view.h"

#include <assets_icons.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <gui/modules/button_panel.h>
#include <input/input.h>

#include <furi.h>
#include <furi_hal_resources.h>
#include <core/check.h>
#include <stdint.h>

struct InfraredProgressView {
    View* view;
    InfraredProgressViewInputCallback input_callback;
    void* context;
};

typedef struct {
    size_t progress;
    size_t progress_total;
    bool is_paused;
} InfraredProgressViewModel;

static void infrared_progress_view_draw_callback(Canvas* canvas, void* _model) {
    InfraredProgressViewModel* model = (InfraredProgressViewModel*)_model;

    uint8_t x = 0;
    uint8_t y = 25;
    uint8_t width = 63;
    uint8_t height = 81;

    elements_bold_rounded_frame(canvas, x, y, width, height);

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas,
        x + 32,
        y + 9,
        AlignCenter,
        AlignCenter,
        model->is_paused ? "Paused" : "Sending...");

    float progress_value = (float)model->progress / model->progress_total;
    elements_progress_bar(canvas, x + 4, y + 19, width - 7, progress_value);

    char progress_string[16] = {0};
    if(model->is_paused) {
        snprintf(
            progress_string,
            sizeof(progress_string),
            "%zu/%zu",
            model->progress,
            model->progress_total);
    } else {
        uint8_t percent_value = 100 * model->progress / model->progress_total;
        snprintf(progress_string, sizeof(progress_string), "%d%%", percent_value);
    }
    elements_multiline_text_aligned(
        canvas, x + 33, y + 37, AlignCenter, AlignCenter, progress_string);

    uint8_t buttons_x = x + (model->is_paused ? 10 : 14);
    uint8_t buttons_y = y + (model->is_paused ? 46 : 50);

    canvas_draw_icon(canvas, buttons_x + 0, buttons_y + 0, &I_Pin_back_arrow_10x8);
    canvas_draw_str(canvas, buttons_x + 14, buttons_y + 8, model->is_paused ? "resume" : "stop");

    canvas_draw_icon(canvas, buttons_x + 1, buttons_y + 10, &I_Ok_btn_9x9);
    canvas_draw_str(canvas, buttons_x + 14, buttons_y + 17, model->is_paused ? "send 1" : "pause");

    if(model->is_paused) {
        canvas_draw_icon(canvas, buttons_x + 2, buttons_y + 21, &I_ButtonLeftSmall_3x5);
        canvas_draw_icon(canvas, buttons_x + 7, buttons_y + 21, &I_ButtonRightSmall_3x5);
        canvas_draw_str(canvas, buttons_x + 14, buttons_y + 26, "select");
    }
}

bool infrared_progress_view_set_progress(InfraredProgressView* instance, uint16_t progress) {
    bool result;
    with_view_model(
        instance->view,
        InfraredProgressViewModel * model,
        {
            result = progress <= model->progress_total;
            if(result) model->progress = progress;
        },
        true);
    return result;
}

void infrared_progress_view_set_progress_total(
    InfraredProgressView* progress,
    uint16_t progress_total) {
    furi_assert(progress);
    InfraredProgressViewModel* model = view_get_model(progress->view);
    model->progress = 0;
    model->progress_total = progress_total;
    view_commit_model(progress->view, false);
}

void infrared_progress_view_set_paused(InfraredProgressView* instance, bool is_paused) {
    with_view_model(
        instance->view, InfraredProgressViewModel * model, { model->is_paused = is_paused; }, true);
}

bool infrared_progress_view_input_callback(InputEvent* event, void* context) {
    InfraredProgressView* instance = context;

    if(event->type == InputTypePress || event->type == InputTypeRelease) {
        return false;
    }

    if(!instance->input_callback) return false;

    with_view_model(
        instance->view,
        InfraredProgressViewModel * model,
        {
            if(model->is_paused) {
                if(event->key == InputKeyLeft)
                    instance->input_callback(
                        instance->context, InfraredProgressViewInputPreviousSignal);
                else if(event->key == InputKeyRight)
                    instance->input_callback(
                        instance->context, InfraredProgressViewInputNextSignal);
                else if(event->key == InputKeyOk)
                    instance->input_callback(
                        instance->context, InfraredProgressViewInputSendSingle);
                else if(event->key == InputKeyBack)
                    instance->input_callback(instance->context, InfraredProgressViewInputResume);
            } else {
                if(event->key == InputKeyOk)
                    instance->input_callback(instance->context, InfraredProgressViewInputPause);
                else if(event->key == InputKeyBack)
                    instance->input_callback(instance->context, InfraredProgressViewInputStop);
            }
        },
        false);

    return true;
}

InfraredProgressView* infrared_progress_view_alloc(void) {
    InfraredProgressView* instance = malloc(sizeof(InfraredProgressView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(InfraredProgressViewModel));
    InfraredProgressViewModel* model = view_get_model(instance->view);
    model->progress = 0;
    model->progress_total = 0;
    view_commit_model(instance->view, false);
    view_set_draw_callback(instance->view, infrared_progress_view_draw_callback);
    view_set_input_callback(instance->view, infrared_progress_view_input_callback);
    view_set_context(instance->view, instance);

    return instance;
}

void infrared_progress_view_free(InfraredProgressView* progress) {
    view_free(progress->view);
    free(progress);
}

void infrared_progress_view_set_input_callback(
    InfraredProgressView* instance,
    InfraredProgressViewInputCallback callback,
    void* context) {
    furi_assert(instance);
    instance->input_callback = callback;
    instance->context = context;
}

View* infrared_progress_view_get_view(InfraredProgressView* instance) {
    furi_assert(instance);
    furi_assert(instance->view);
    return instance->view;
}
