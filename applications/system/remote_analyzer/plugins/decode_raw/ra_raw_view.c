#include "ra_raw_view.h"

#include <furi.h>
#include <gui/elements.h>

struct RaRawView {
    View* view;
};

typedef struct {
    FuriString* headline;
    FuriString* detail;
    uint8_t percent;
    bool working; // the bar is only drawn while there is progress to report
} RaRawViewModel;

static void ra_raw_view_draw(Canvas* canvas, RaRawViewModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 16, AlignCenter, AlignBottom, furi_string_get_cstr(model->headline));

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 28, AlignCenter, AlignBottom, furi_string_get_cstr(model->detail));

    if(model->working) {
        char text[8];
        snprintf(text, sizeof(text), "%u%%", (unsigned int)model->percent);
        elements_progress_bar_with_text(canvas, 8, 38, 112, model->percent / 100.0f, text);
    } else {
        // Nothing left to do on this screen, so say the one key that still does something
        canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignBottom, "Press Back");
    }
}

RaRawView* ra_raw_view_alloc(void) {
    RaRawView* instance = malloc(sizeof(RaRawView));

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(RaRawViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)ra_raw_view_draw);

    with_view_model(
        instance->view,
        RaRawViewModel * model,
        {
            model->headline = furi_string_alloc();
            model->detail = furi_string_alloc();
            model->percent = 0;
            model->working = false;
        },
        false);

    return instance;
}

void ra_raw_view_free(RaRawView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        RaRawViewModel * model,
        {
            furi_string_free(model->headline);
            furi_string_free(model->detail);
        },
        false);
    view_free(instance->view);
    free(instance);
}

View* ra_raw_view_get_view(RaRawView* instance) {
    furi_assert(instance);
    return instance->view;
}

void ra_raw_view_set_progress(RaRawView* instance, const char* name, uint8_t percent) {
    furi_assert(instance);
    bool changed = false;
    with_view_model(
        instance->view,
        RaRawViewModel * model,
        {
            // A redraw per tick for a bar that has not moved is not worth it
            changed = !model->working || (model->percent != percent);
            furi_string_set_str(model->headline, "Decoding");
            furi_string_set_str(model->detail, name);
            model->percent = percent;
            model->working = true;
        },
        changed);
}

void ra_raw_view_set_done(RaRawView* instance, const char* headline, const char* detail) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RaRawViewModel * model,
        {
            furi_string_set_str(model->headline, headline);
            furi_string_set_str(model->detail, detail);
            model->working = false;
        },
        true);
}
