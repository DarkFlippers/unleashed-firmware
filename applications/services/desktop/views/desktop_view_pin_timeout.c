#include <furi.h>

#include <gui/canvas.h>
#include <gui/view.h>

#include "desktop_view_pin_timeout.h"

struct DesktopViewPinTimeout {
    View* view;
    FuriTimer* timer;
    DesktopViewPinTimeoutDoneCallback callback;
    void* context;
};

typedef struct {
    uint32_t time_left;
} DesktopViewPinTimeoutModel;

void desktop_view_pin_timeout_set_callback(
    DesktopViewPinTimeout* instance,
    DesktopViewPinTimeoutDoneCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static void desktop_view_pin_timeout_timer_callback(void* context) {
    DesktopViewPinTimeout* instance = context;
    bool stop = false;

    DesktopViewPinTimeoutModel* model = view_get_model(instance->view);
    if(model->time_left > 0) {
        --model->time_left;
    } else {
        stop = true;
    }
    view_commit_model(instance->view, true);

    if(stop) {
        furi_timer_stop(instance->timer);
        instance->callback(instance->context);
    }
}

static bool desktop_view_pin_timeout_input(InputEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);
    return true;
}

static void desktop_view_pin_timeout_draw(Canvas* canvas, void* _model) {
    furi_assert(canvas);
    furi_assert(_model);

    DesktopViewPinTimeoutModel* model = _model;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 36, 31, "Wrong PIN!");

    canvas_set_font(canvas, FontSecondary);
    char str[30] = {0};
    snprintf(str, sizeof(str), "Timeout: %lus", model->time_left);
    canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignCenter, str);
}

void desktop_view_pin_timeout_free(DesktopViewPinTimeout* instance) {
    view_free(instance->view);
    furi_timer_free(instance->timer);

    free(instance);
}

DesktopViewPinTimeout* desktop_view_pin_timeout_alloc(void) {
    DesktopViewPinTimeout* instance = malloc(sizeof(DesktopViewPinTimeout));
    instance->timer =
        furi_timer_alloc(desktop_view_pin_timeout_timer_callback, FuriTimerTypePeriodic, instance);

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLockFree, sizeof(DesktopViewPinTimeoutModel));

    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, desktop_view_pin_timeout_draw);
    view_set_input_callback(instance->view, desktop_view_pin_timeout_input);

    return instance;
}

void desktop_view_pin_timeout_start(DesktopViewPinTimeout* instance, uint32_t time_left) {
    furi_assert(instance);

    DesktopViewPinTimeoutModel* model = view_get_model(instance->view);
    // no race - always called when timer is stopped
    model->time_left = time_left;
    view_commit_model(instance->view, true);

    furi_timer_start(instance->timer, 1000);
}

View* desktop_view_pin_timeout_get_view(DesktopViewPinTimeout* instance) {
    furi_assert(instance);

    return instance->view;
}
