#pragma once

#include <gui/view.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>

typedef struct PomodoroTimer PomodoroTimer;

struct PomodoroTimer {
    View* view;
};

typedef struct PomodoroTimerModel PomodoroTimerModel;

struct PomodoroTimerModel {
    bool ok_pressed;
    bool reset_pressed;
    bool back_pressed;
    bool connected;
    bool timer_running;
    bool rest_running;
    bool sound_playing;
    uint32_t timer_start_timestamp;
    uint32_t timer_stopped_seconds;
    uint32_t time_passed;
    uint32_t rest_start_timestamp;
    int counter;
};

void pomodoro_timer_process(PomodoroTimer* pomodoro_timer, InputEvent* event);

void pomodoro_draw_callback(Canvas* canvas, void* context, int max_seconds, int max_seconds_rest);
