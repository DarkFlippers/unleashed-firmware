#include "t5577_work_view.h"

#include <furi.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <t5577_icons.h>

#define T5577_WORK_VIEW_TITLE_MAX  12
#define T5577_WORK_VIEW_STATUS_MAX 24

// The tag picture on the left, everything else in the strip beside it.
#define T5577_WORK_VIEW_COL 66

typedef struct {
    char title[T5577_WORK_VIEW_TITLE_MAX];
    char status[T5577_WORK_VIEW_STATUS_MAX];
    uint16_t done;
    uint16_t total;
} T5577WorkViewModel;

struct T5577WorkView {
    View* view;
};

static void t5577_work_view_draw_callback(Canvas* canvas, void* _model) {
    const T5577WorkViewModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_icon(canvas, 0, 8, &I_NFC_manual_60x50);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, T5577_WORK_VIEW_COL, 18, model->title);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, T5577_WORK_VIEW_COL, 31, model->status);

    char count[12];
    snprintf(count, sizeof(count), "%u/%u", model->done, model->total);
    const float progress = model->total ? (float)model->done / (float)model->total : 0.0f;
    // The bar's frame sits 2 px outside its text, so it starts left of the column to line up.
    elements_progress_bar_with_text(canvas, T5577_WORK_VIEW_COL - 2, 36, 62, progress, count);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, T5577_WORK_VIEW_COL, 58, "Hold still");
}

T5577WorkView* t5577_work_view_alloc(void) {
    T5577WorkView* instance = malloc(sizeof(T5577WorkView));

    instance->view = view_alloc();
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(T5577WorkViewModel));
    view_set_draw_callback(instance->view, t5577_work_view_draw_callback);

    return instance;
}

void t5577_work_view_free(T5577WorkView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* t5577_work_view_get_view(T5577WorkView* instance) {
    furi_assert(instance);

    return instance->view;
}

void t5577_work_view_start(T5577WorkView* instance, const char* title, uint16_t total) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577WorkViewModel * model,
        {
            strlcpy(model->title, title, sizeof(model->title));
            model->status[0] = '\0';
            model->done = 0;
            model->total = total;
        },
        true);
}

void t5577_work_view_set_status(T5577WorkView* instance, const char* status, uint16_t done) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        T5577WorkViewModel * model,
        {
            strlcpy(model->status, status, sizeof(model->status));
            // The bar cannot draw past its end, and the count beside it has to agree with it.
            model->done = done > model->total ? model->total : done;
        },
        true);
}
