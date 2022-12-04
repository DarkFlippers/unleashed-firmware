#include "../pomodoro_timer.h"
#include "pomodoro_25.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>

static void pomodoro_25_draw_callback(Canvas* canvas, void* context) {
    int max_seconds = 60 * 25;
    int max_seconds_rest = 60 * 5;
    pomodoro_draw_callback(canvas, context, max_seconds, max_seconds_rest);
}

static bool pomodoro_25_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    PomodoroTimer* pomodoro_25 = context;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        return false;
    } else {
        pomodoro_timer_process(pomodoro_25, event);
        return true;
    }
}

PomodoroTimer* pomodoro_25_alloc() {
    PomodoroTimer* pomodoro_25 = malloc(sizeof(PomodoroTimer));
    pomodoro_25->view = view_alloc();
    view_set_context(pomodoro_25->view, pomodoro_25);
    view_allocate_model(pomodoro_25->view, ViewModelTypeLocking, sizeof(PomodoroTimerModel));
    view_set_draw_callback(pomodoro_25->view, pomodoro_25_draw_callback);
    view_set_input_callback(pomodoro_25->view, pomodoro_25_input_callback);

    return pomodoro_25;
}

void pomodoro_25_free(PomodoroTimer* pomodoro_25) {
    furi_assert(pomodoro_25);
    view_free(pomodoro_25->view);
    free(pomodoro_25);
}

View* pomodoro_25_get_view(PomodoroTimer* pomodoro_25) {
    furi_assert(pomodoro_25);
    return pomodoro_25->view;
}
