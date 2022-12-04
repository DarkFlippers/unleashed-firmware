#include "../pomodoro_timer.h"
#include "pomodoro_10.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>

static void pomodoro_10_draw_callback(Canvas* canvas, void* context) {
    int max_seconds = 60 * 10;
    int max_seconds_rest = 60 * 2;
    pomodoro_draw_callback(canvas, context, max_seconds, max_seconds_rest);
}

static bool pomodoro_10_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    PomodoroTimer* pomodoro_10 = context;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        return false;
    } else {
        pomodoro_timer_process(pomodoro_10, event);
        return true;
    }
}

PomodoroTimer* pomodoro_10_alloc() {
    PomodoroTimer* pomodoro_10 = malloc(sizeof(PomodoroTimer));
    pomodoro_10->view = view_alloc();
    view_set_context(pomodoro_10->view, pomodoro_10);
    view_allocate_model(pomodoro_10->view, ViewModelTypeLocking, sizeof(PomodoroTimerModel));
    view_set_draw_callback(pomodoro_10->view, pomodoro_10_draw_callback);
    view_set_input_callback(pomodoro_10->view, pomodoro_10_input_callback);

    return pomodoro_10;
}

void pomodoro_10_free(PomodoroTimer* pomodoro_10) {
    furi_assert(pomodoro_10);
    view_free(pomodoro_10->view);
    free(pomodoro_10);
}

View* pomodoro_10_get_view(PomodoroTimer* pomodoro_10) {
    furi_assert(pomodoro_10);
    return pomodoro_10->view;
}
