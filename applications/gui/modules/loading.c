#include <stdint.h>
#include <furi.h>
#include <assets_icons.h>
#include <gui/icon_animation.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <input/input.h>

#include "loading.h"

struct Loading {
    View* view;
};

typedef struct {
    IconAnimation* icon;
} LoadingModel;

static void loading_draw_callback(Canvas* canvas, void* _model) {
    LoadingModel* model = (LoadingModel*)_model;

    uint8_t width = 49;
    uint8_t height = 47;
    uint8_t x = (canvas_width(canvas) - width) / 2;
    uint8_t y = (canvas_height(canvas) - height) / 2;

    elements_bold_rounded_frame(canvas, x, y, width, height);

    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text(canvas, x + 7, y + 13, "Loading...");

    canvas_draw_icon_animation(canvas, x + 13, y + 19, model->icon);
}

static bool loading_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    furi_assert(context);
    return true;
}

static void loading_enter_callback(void* context) {
    furi_assert(context);
    Loading* instance = context;
    LoadingModel* model = view_get_model(instance->view);
    /* using Loading View in conjunction with several
     * Stack View obligates to reassign
     * Update callback, as it can be rewritten
     */
    view_tie_icon_animation(instance->view, model->icon);
    icon_animation_start(model->icon);
    view_commit_model(instance->view, false);
}

static void loading_exit_callback(void* context) {
    furi_assert(context);
    Loading* instance = context;
    LoadingModel* model = view_get_model(instance->view);
    icon_animation_stop(model->icon);
    view_commit_model(instance->view, false);
}

Loading* loading_alloc(void) {
    Loading* instance = malloc(sizeof(Loading));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(LoadingModel));
    LoadingModel* model = view_get_model(instance->view);
    model->icon = icon_animation_alloc(&A_Loading_24);
    view_tie_icon_animation(instance->view, model->icon);
    view_commit_model(instance->view, false);

    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, loading_draw_callback);
    view_set_input_callback(instance->view, loading_input_callback);
    view_set_enter_callback(instance->view, loading_enter_callback);
    view_set_exit_callback(instance->view, loading_exit_callback);

    return instance;
}

void loading_free(Loading* instance) {
    LoadingModel* model = view_get_model(instance->view);
    icon_animation_free(model->icon);
    view_commit_model(instance->view, false);

    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* loading_get_view(Loading* instance) {
    furi_assert(instance);
    furi_assert(instance->view);
    return instance->view;
}
