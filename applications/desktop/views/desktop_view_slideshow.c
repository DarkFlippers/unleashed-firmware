#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>

#include "../desktop_i.h"
#include "desktop_view_slideshow.h"
#include "../helpers/slideshow.h"

struct DesktopSlideshowView {
    View* view;
    DesktopSlideshowViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t page;
    Slideshow* slideshow;
} DesktopSlideshowViewModel;

static void desktop_view_slideshow_draw(Canvas* canvas, void* model) {
    DesktopSlideshowViewModel* m = model;

    canvas_clear(canvas);
    if(slideshow_is_loaded(m->slideshow)) {
        slideshow_draw(m->slideshow, canvas, 0, 0);
    }
}

static bool desktop_view_slideshow_input(InputEvent* event, void* context) {
    furi_assert(event);
    DesktopSlideshowView* instance = context;

    if(event->type == InputTypeShort) {
        DesktopSlideshowViewModel* model = view_get_model(instance->view);
        bool end_slideshow = false;
        switch(event->key) {
        case InputKeyLeft:
            slideshow_goback(model->slideshow);
            break;
        case InputKeyRight:
        case InputKeyOk:
            end_slideshow = !slideshow_advance(model->slideshow);
            break;
        case InputKeyBack:
            end_slideshow = true;
        default:
            break;
        }
        if(end_slideshow) {
            instance->callback(DesktopSlideshowCompleted, instance->context);
        }
        view_commit_model(instance->view, true);
    }

    return true;
}

static void desktop_view_slideshow_enter(void* context) {
    DesktopSlideshowView* instance = context;

    DesktopSlideshowViewModel* model = view_get_model(instance->view);
    model->slideshow = slideshow_alloc();
    if(!slideshow_load(model->slideshow, "/int/slideshow")) {
        instance->callback(DesktopSlideshowCompleted, instance->context);
    }
    view_commit_model(instance->view, false);
}

static void desktop_view_slideshow_exit(void* context) {
    DesktopSlideshowView* instance = context;

    DesktopSlideshowViewModel* model = view_get_model(instance->view);
    slideshow_free(model->slideshow);
    view_commit_model(instance->view, false);
}

DesktopSlideshowView* desktop_view_slideshow_alloc() {
    DesktopSlideshowView* instance = malloc(sizeof(DesktopSlideshowView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(DesktopSlideshowViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)desktop_view_slideshow_draw);
    view_set_input_callback(instance->view, desktop_view_slideshow_input);
    view_set_enter_callback(instance->view, desktop_view_slideshow_enter);
    view_set_exit_callback(instance->view, desktop_view_slideshow_exit);

    return instance;
}

void desktop_view_slideshow_free(DesktopSlideshowView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* desktop_view_slideshow_get_view(DesktopSlideshowView* instance) {
    furi_assert(instance);
    return instance->view;
}

void desktop_view_slideshow_set_callback(
    DesktopSlideshowView* instance,
    DesktopSlideshowViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}