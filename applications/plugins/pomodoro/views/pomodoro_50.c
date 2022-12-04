#include "../pomodoro_timer.h"
#include "pomodoro_50.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>

static void pomodoro_50_draw_callback(Canvas* canvas, void* context) {
    int max_seconds = 60 * 50;
    int max_seconds_rest = 60 * 10;
    pomodoro_draw_callback(canvas, context, max_seconds, max_seconds_rest);
}

static bool pomodoro_50_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    PomodoroTimer* pomodoro_50 = context;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        return false;
    } else {
        pomodoro_timer_process(pomodoro_50, event);
        return true;
    }
}

PomodoroTimer* pomodoro_50_alloc() {
    PomodoroTimer* pomodoro_50 = malloc(sizeof(PomodoroTimer));
    pomodoro_50->view = view_alloc();
    view_set_context(pomodoro_50->view, pomodoro_50);
    view_allocate_model(pomodoro_50->view, ViewModelTypeLocking, sizeof(PomodoroTimerModel));
    view_set_draw_callback(pomodoro_50->view, pomodoro_50_draw_callback);
    view_set_input_callback(pomodoro_50->view, pomodoro_50_input_callback);

    return pomodoro_50;
}

void pomodoro_50_free(PomodoroTimer* pomodoro_50) {
    furi_assert(pomodoro_50);
    view_free(pomodoro_50->view);
    free(pomodoro_50);
}

View* pomodoro_50_get_view(PomodoroTimer* pomodoro_50) {
    furi_assert(pomodoro_50);
    return pomodoro_50->view;
}
