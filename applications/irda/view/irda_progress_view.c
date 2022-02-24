#include "furi/check.h"
#include "furi_hal_resources.h"
#include "assets_icons.h"
#include "gui/canvas.h"
#include "gui/view.h"
#include "input/input.h"
#include "m-string.h"
#include <gui/elements.h>
#include <furi.h>
#include "irda_progress_view.h"
#include "gui/modules/button_panel.h"
#include <stdint.h>

struct IrdaProgressView {
    View* view;
    IrdaProgressViewBackCallback back_callback;
    void* context;
};

typedef struct {
    size_t progress;
    size_t progress_total;
} IrdaProgressViewModel;

bool irda_progress_view_increase_progress(IrdaProgressView* progress) {
    furi_assert(progress);
    bool result = false;

    IrdaProgressViewModel* model = view_get_model(progress->view);
    if(model->progress < model->progress_total) {
        ++model->progress;
        result = model->progress < model->progress_total;
    }
    view_commit_model(progress->view, true);

    return result;
}

static void irda_progress_view_draw_callback(Canvas* canvas, void* _model) {
    IrdaProgressViewModel* model = (IrdaProgressViewModel*)_model;

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

    canvas_draw_icon(canvas, x + 11, y + height - 15, &I_Back_15x10);
    canvas_draw_str(canvas, x + 30, y + height - 6, "= stop");
}

void irda_progress_view_set_progress_total(IrdaProgressView* progress, uint16_t progress_total) {
    furi_assert(progress);
    IrdaProgressViewModel* model = view_get_model(progress->view);
    model->progress = 0;
    model->progress_total = progress_total;
    view_commit_model(progress->view, false);
}

bool irda_progress_view_input_callback(InputEvent* event, void* context) {
    IrdaProgressView* instance = context;

    if((event->type == InputTypeShort) && (event->key == InputKeyBack)) {
        if(instance->back_callback) {
            instance->back_callback(instance->context);
        }
    }

    return true;
}

IrdaProgressView* irda_progress_view_alloc(void) {
    IrdaProgressView* instance = malloc(sizeof(IrdaProgressView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(IrdaProgressViewModel));
    IrdaProgressViewModel* model = view_get_model(instance->view);
    model->progress = 0;
    model->progress_total = 0;
    view_commit_model(instance->view, false);
    view_set_draw_callback(instance->view, irda_progress_view_draw_callback);
    view_set_input_callback(instance->view, irda_progress_view_input_callback);
    view_set_context(instance->view, instance);

    return instance;
}

void irda_progress_view_free(IrdaProgressView* progress) {
    view_free(progress->view);
    free(progress);
}

void irda_progress_view_set_back_callback(
    IrdaProgressView* instance,
    IrdaProgressViewBackCallback callback,
    void* context) {
    furi_assert(instance);
    instance->back_callback = callback;
    instance->context = context;
}

View* irda_progress_view_get_view(IrdaProgressView* instance) {
    furi_assert(instance);
    furi_assert(instance->view);
    return instance->view;
}
