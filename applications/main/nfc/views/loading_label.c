#include "loading_label.h"

#include <gui/icon_animation.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <input/input.h>

#include <furi.h>
#include <assets_icons.h>

struct LoadingLabel {
    View* view;
};

typedef struct {
    IconAnimation* icon;
    const char* text;
    bool progress_shown;
    float progress;
} LoadingLabelModel;

static void loading_label_draw_callback(Canvas* canvas, void* _model) {
    LoadingLabelModel* model = _model;

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), canvas_height(canvas));
    canvas_set_color(canvas, ColorBlack);

    // Spinner on the left, vertically centered
    const uint8_t icon_x = 12;
    const uint8_t icon_y = canvas_height(canvas) / 2 - 24 / 2;
    canvas_draw_icon(canvas, icon_x, icon_y, &A_Loading_24);
    canvas_draw_icon_animation(canvas, icon_x, icon_y, model->icon);

    // Label right of the spinner. With a bar, the 2-line label (20px) + 4px gap + 9px bar
    // are centered as one block.
    const uint8_t text_y = canvas_height(canvas) / 2 - (model->progress_shown ? 6 : 0);
    if(model->text) {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(canvas, 82, text_y, AlignCenter, AlignCenter, model->text);
    }

    if(model->progress_shown) {
        elements_progress_bar(canvas, 82 - 64 / 2, text_y + 14, 64, model->progress);
    }
}

static bool loading_label_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    furi_assert(context);
    // Consume all input while loading
    return true;
}

static void loading_label_enter_callback(void* context) {
    furi_assert(context);
    LoadingLabel* instance = context;
    LoadingLabelModel* model = view_get_model(instance->view);
    view_tie_icon_animation(instance->view, model->icon);
    icon_animation_start(model->icon);
    view_commit_model(instance->view, false);
}

static void loading_label_exit_callback(void* context) {
    furi_assert(context);
    LoadingLabel* instance = context;
    LoadingLabelModel* model = view_get_model(instance->view);
    icon_animation_stop(model->icon);
    view_commit_model(instance->view, false);
}

LoadingLabel* loading_label_alloc(void) {
    LoadingLabel* instance = malloc(sizeof(LoadingLabel));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(LoadingLabelModel));
    LoadingLabelModel* model = view_get_model(instance->view);
    model->icon = icon_animation_alloc(&A_Loading_24);
    model->text = NULL;
    model->progress_shown = false;
    model->progress = 0.0f;
    view_tie_icon_animation(instance->view, model->icon);
    view_commit_model(instance->view, false);

    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, loading_label_draw_callback);
    view_set_input_callback(instance->view, loading_label_input_callback);
    view_set_enter_callback(instance->view, loading_label_enter_callback);
    view_set_exit_callback(instance->view, loading_label_exit_callback);

    return instance;
}

void loading_label_free(LoadingLabel* instance) {
    furi_check(instance);

    LoadingLabelModel* model = view_get_model(instance->view);
    icon_animation_free(model->icon);
    view_commit_model(instance->view, false);

    view_free(instance->view);
    free(instance);
}

View* loading_label_get_view(LoadingLabel* instance) {
    furi_check(instance);
    return instance->view;
}

void loading_label_set_text(LoadingLabel* instance, const char* text) {
    furi_check(instance);
    LoadingLabelModel* model = view_get_model(instance->view);
    model->text = text;
    model->progress_shown = false;
    view_commit_model(instance->view, true);
}

void loading_label_set_progress(LoadingLabel* instance, float progress) {
    furi_check(instance);
    const float clamped = CLAMP(progress, 1.0f, 0.0f);
    bool changed = false;
    with_view_model(
        instance->view,
        LoadingLabelModel * model,
        {
            changed = !model->progress_shown || (model->progress != clamped);
            model->progress_shown = true;
            model->progress = clamped;
        },
        changed);
}
