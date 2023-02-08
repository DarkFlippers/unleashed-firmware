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
    InfraredProgressViewBackCallback back_callback;
    void* context;
};

typedef struct {
    size_t progress;
    size_t progress_total;
} InfraredProgressViewModel;

bool infrared_progress_view_increase_progress(InfraredProgressView* progress) {
    furi_assert(progress);
    bool result = false;

    InfraredProgressViewModel* model = view_get_model(progress->view);
    if(model->progress < model->progress_total) {
        ++model->progress;
        result = model->progress < model->progress_total;
    }
    view_commit_model(progress->view, true);

    return result;
}

static void infrared_progress_view_draw_callback(Canvas* canvas, void* _model) {
    InfraredProgressViewModel* model = (InfraredProgressViewModel*)_model;

    uint8_t x = 0;
    uint8_t y = 36;
    uint8_t width = 63;
    uint8_t height = 59;

    elements_bold_rounded_frame(canvas, x, y, width, height);

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, x + 34, y + 9, AlignCenter, AlignCenter, "Sending ...");

    float progress_value = (float)model->progress / model->progress_total;
    elements_progress_bar(canvas, x + 4, y + 19, width - 7, progress_value);

    uint8_t percent_value = 100 * model->progress / model->progress_total;
    char percents_string[10] = {0};
    snprintf(percents_string, sizeof(percents_string), "%d%%", percent_value);
    elements_multiline_text_aligned(
        canvas, x + 33, y + 37, AlignCenter, AlignCenter, percents_string);

    canvas_draw_icon(canvas, x + 14, y + height - 14, &I_Pin_back_arrow_10x8);
    canvas_draw_str(canvas, x + 30, y + height - 6, "= stop");
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

bool infrared_progress_view_input_callback(InputEvent* event, void* context) {
    InfraredProgressView* instance = context;

    if((event->type == InputTypeShort) && (event->key == InputKeyBack)) {
        if(instance->back_callback) {
            instance->back_callback(instance->context);
        }
    }

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

void infrared_progress_view_set_back_callback(
    InfraredProgressView* instance,
    InfraredProgressViewBackCallback callback,
    void* context) {
    furi_assert(instance);
    instance->back_callback = callback;
    instance->context = context;
}

View* infrared_progress_view_get_view(InfraredProgressView* instance) {
    furi_assert(instance);
    furi_assert(instance->view);
    return instance->view;
}
