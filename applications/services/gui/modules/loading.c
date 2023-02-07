#include "loading.h"

#include <gui/icon_animation.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <input/input.h>

#include <furi.h>
#include <assets_icons.h>
#include <stdint.h>

struct Loading {
    View* view;
};

typedef struct {
    IconAnimation* icon;
} LoadingModel;

static void loading_draw_callback(Canvas* canvas, void* _model) {
    LoadingModel* model = (LoadingModel*)_model;

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), canvas_height(canvas));
    canvas_set_color(canvas, ColorBlack);

    uint8_t x = canvas_width(canvas) / 2 - 24 / 2;
    uint8_t y = canvas_height(canvas) / 2 - 24 / 2;

    canvas_draw_icon(canvas, x, y, &A_Loading_24);

    canvas_draw_icon_animation(canvas, x, y, model->icon);
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
