
#include "one_shot_animation_view.h"
#include <furi.h>
#include <gui/canvas.h>
#include <gui/view.h>
#include <gui/icon_i.h>
#include <stdint.h>

typedef void (*OneShotInteractCallback)(void*);

struct OneShotView {
    View* view;
    FuriTimer* update_timer;
    OneShotInteractCallback interact_callback;
    void* interact_callback_context;
};

typedef struct {
    const Icon* icon;
    uint32_t index;
    bool block_input;
} OneShotViewModel;

static void one_shot_view_update_timer_callback(void* context) {
    OneShotView* view = context;

    OneShotViewModel* model = view_get_model(view->view);
    if((model->index + 1) < model->icon->frame_count) {
        ++model->index;
    } else {
        model->block_input = false;
        model->index = model->icon->frame_count - 2;
    }
    view_commit_model(view->view, true);
}

static void one_shot_view_draw(Canvas* canvas, void* model_) {
    furi_assert(canvas);
    furi_assert(model_);

    OneShotViewModel* model = model_;
    furi_check(model->index < model->icon->frame_count);
    uint8_t y_offset = canvas_height(canvas) - model->icon->height;
    canvas_draw_bitmap(
        canvas,
        0,
        y_offset,
        model->icon->width,
        model->icon->height,
        model->icon->frames[model->index]);
}

static bool one_shot_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    OneShotView* view = context;
    bool consumed = false;

    OneShotViewModel* model = view_get_model(view->view);
    consumed = model->block_input;
    view_commit_model(view->view, false);

    if(!consumed) {
        if(event->key == InputKeyRight) {
            /* Right button reserved for animation activation, so consume */
            if(event->type == InputTypeShort) {
                consumed = true;
                if(view->interact_callback) {
                    view->interact_callback(view->interact_callback_context);
                }
            }
        }
    }

    return consumed;
}

OneShotView* one_shot_view_alloc(void) {
    OneShotView* view = malloc(sizeof(OneShotView));
    view->view = view_alloc();
    view->update_timer =
        furi_timer_alloc(one_shot_view_update_timer_callback, FuriTimerTypePeriodic, view);

    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(OneShotViewModel));
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, one_shot_view_draw);
    view_set_input_callback(view->view, one_shot_view_input);

    return view;
}

void one_shot_view_free(OneShotView* view) {
    furi_assert(view);

    furi_timer_free(view->update_timer);
    view_free(view->view);
    view->view = NULL;
    free(view);
}

void one_shot_view_set_interact_callback(
    OneShotView* view,
    OneShotInteractCallback callback,
    void* context) {
    furi_assert(view);

    view->interact_callback_context = context;
    view->interact_callback = callback;
}

void one_shot_view_start_animation(OneShotView* view, const Icon* icon) {
    furi_assert(view);
    furi_assert(icon);
    furi_check(icon->frame_count >= 2);

    OneShotViewModel* model = view_get_model(view->view);
    model->index = 0;
    model->icon = icon;
    model->block_input = true;
    view_commit_model(view->view, true);
    furi_timer_start(view->update_timer, 1000 / model->icon->frame_rate);
}

View* one_shot_view_get_view(OneShotView* view) {
    furi_assert(view);

    return view->view;
}
